import argparse
import html
import re
import shutil
from collections import defaultdict
from pathlib import Path
from typing import Dict, List, Optional

DECLARED_RE = re.compile(r"Declared in `(.*)`")


def get_declared_in(adoc_path: Path) -> Optional[str]:
    """Extract the first "Declared in" header location from an .adoc file."""
    if not adoc_path.exists():
        return None
    with adoc_path.open("r", encoding="utf-8") as f:
        for line in f:
            match = DECLARED_RE.search(line)
            if not match:
                continue
            raw = match.group(1).replace("&period;h&gt;", "&gt;")
            value = re.sub(r'^[`"\s<]+|[`"\s>]+$', '', html.unescape(raw))
            return value[:-2] if value.endswith(".h") else value or None
    return None


def safe_filename(name: str) -> str:
    s = re.sub(r"[\\/]+", "_", name)
    s = re.sub(r"[^0-9A-Za-z._-]", "_", s)
    return s


def _replace_macros_in_text(text: str) -> str:
    """Replace $lift(expr) with ^^expr and decltype() with /* ... */."""
    out: List[str] = []
    pos = 0
    
    while pos < len(text):
        next_lift = text.find('$lift(', pos)
        if next_lift == -1:
            out.append(text[pos:])
            break
        
        out.append(text[pos:next_lift])
        i = next_lift + len('$lift(')
        depth = 1
        start = i
        
        while i < len(text) and depth > 0:
            if text[i] == '(':
                depth += 1
            elif text[i] == ')':
                depth -= 1
            i += 1
        
        if depth == 0:
            out.append('^^' + text[start:i-1])
            pos = i
        else:
            out.append(text[next_lift:])
            break
    
    result = ''.join(out)
    return result.replace('decltype()', '/* ... */')


def find_namespace_overviews(base_dir: Path) -> List[Path]:
    """Return .adoc files that appear to be namespace overviews.
    
    Heuristic: contains section headings and at least one xref link.
    """
    res: List[Path] = []
    for p in sorted(base_dir.rglob("*.adoc")):
        if p.name in ("nav.adoc", "index.adoc") or (base_dir / "headers") in p.parents:
            continue
        try:
            txt = p.read_text(encoding="utf-8")
            if "xref:" in txt and re.search(r"^==\s*(Functions|Concepts|Namespaces|Classes|Types|Enumerations)\b", txt, re.M):
                res.append(p)
        except Exception:
            continue
    return res


def parse_overview_file(path: Path, base_dir: Path) -> List[Dict[str, str]]:
    """Parse a namespace overview file and return symbol entries."""
    entries: List[Dict[str, str]] = []
    text = path.read_text(encoding="utf-8")
    current_category = "unsorted"
    
    rel = path.relative_to(base_dir)
    namespace = str(rel).replace(".adoc", "").replace("\\", "/").strip("/").replace("/", "::")

    for line in text.splitlines():
        cat_match = re.match(r"^==\s*(.+)", line)
        if cat_match:
            current_category = cat_match.group(1).strip()
            continue
        for xref_match in re.finditer(r"xref:([^\[]+)\[([^\]]+)\]", line):
            entries.append({
                "category": current_category,
                "target": xref_match.group(1).strip(),
                "label": xref_match.group(2).strip(),
                "namespace": namespace,
            })
    return entries


def collect_symbols_from_overviews(base_dir: Path, output_root: Path):
    """Collect symbols from namespace overview pages, grouped by header.
    
    Returns (headers_symbols, namespace_map, overviews_list) where:
    - headers_symbols: mapping header_key -> list of symbol dicts
    - namespace_map: mapping namespace -> list of {target, label}
    """
    headers_symbols: Dict[str, List[Dict[str, str]]] = defaultdict(list)
    namespace_map: Dict[str, List[Dict[str, str]]] = defaultdict(list)
    overviews = find_namespace_overviews(base_dir)
    copied_overviews: List[Path] = []
    
    for overview_file in overviews:
        try:
            copied_file = _copy_and_patch_to_output(overview_file, base_dir, output_root)
            copied_overviews.append(copied_file)
        except Exception:
            copied_overviews.append(overview_file)
        
        entries = parse_overview_file(overview_file, base_dir)
        for entry in entries:
            target = entry["target"]
            target_src = base_dir / target
            declared_in = None
            
            if target_src.exists():
                try:
                    copy_dest = _copy_and_patch_to_output(target_src, base_dir, output_root)
                    declared_in = get_declared_in(copy_dest)
                except Exception:
                    declared_in = get_declared_in(target_src)
            
            if not declared_in:
                continue
            
            category = entry["category"].strip().lstrip("= ").strip()
            label = entry["label"].strip().strip("`").replace("&lowbar;", "_")
            label = html.unescape(label)
            namespace = entry.get("namespace", "")
            
            seen = {s["url"] for s in headers_symbols.get(declared_in, [])}
            if target not in seen:
                headers_symbols[declared_in].append({
                    "name": label,
                    "kind": category,
                    "url": target,
                    "category": category,
                    "namespace": namespace,
                })
                namespace_map[namespace].append({"target": target, "label": label})
    
    return headers_symbols, namespace_map, overviews


def _copy_and_patch_to_output(src_path: Path, base_dir: Path, output_root: Path) -> Path:
    """Copy file to output_root preserving relative path and apply text patches."""
    rel = src_path.relative_to(base_dir)
    dst = output_root / rel
    dst.parent.mkdir(parents=True, exist_ok=True)
    
    try:
        src_txt = src_path.read_text(encoding="utf-8")
    except Exception:
        src_txt = ""
    
    new_txt = src_txt.replace("&period;h&gt;", "&gt;")
    new_txt = _replace_macros_in_text(new_txt)
    new_txt = html.unescape(new_txt)

    dst.write_text(new_txt, encoding="utf-8")
    return dst


def write_outputs(headers_symbols: Dict[str, List[Dict[str, str]]], output_dir: Path, base_dir: Path) -> None:
    """Write header-grouped symbols as .adoc files."""
    output_dir.mkdir(parents=True, exist_ok=True)
    
    for declared_in in sorted(headers_symbols.keys()):
        symbols = headers_symbols[declared_in]
        header_rel = Path(declared_in.replace("::", "/"))
        out_file = output_dir / header_rel.with_suffix(".adoc")
        print(f"Writing {out_file}")
        out_file.parent.mkdir(parents=True, exist_ok=True)
        
        grouped: Dict[str, List[Dict[str, str]]] = defaultdict(list)
        for s in symbols:
            grouped[s.get("category", "unsorted")].append(s)
        
        content_parts: List[str] = [f"= <{declared_in}>\n\n"]
        
        for kind in sorted(grouped.keys()):
            kind_title = kind.replace("_", " ").title()
            content_parts.append(f"\n== {kind_title}\n\n")
            
            items = grouped[kind]
            for s in sorted(items, key=lambda x: (x.get("namespace", "").count("::"), x.get("namespace", ""), x.get("name", ""))):
                name = s.get("name") or ""
                namespace = s.get("namespace", "") or ""
                fq_name = (namespace + "::" + name) if namespace else name
                content_parts.append(f"* xref:{s['url']}[`{fq_name}`]\n")
        
        content = ''.join(content_parts)
        content = _replace_macros_in_text(content)
        out_file.write_text(content, encoding="utf-8")


def generate_nav(base_dir: Path, headers_keys: List[str], namespace_files: List[Path], out_file: Path, namespace_map: Optional[Dict[str, List[Dict[str, str]]]] = None) -> None:
    """Generate navigation file listing headers and namespace structure."""
    lines: List[str] = ["* Headers"]
    
    for hk in sorted(headers_keys):
        href = f"headers/{hk.replace('::','/')}.adoc"
        lines.append(f"** xref:{href}[`<{hk}>`]")
    
    lines.append("* Namespaces")
    lines.append("** xref:index.adoc[]")
    
    relpaths = [p.relative_to(base_dir) for p in namespace_files]
    namespaces = sorted({str(p).replace('.adoc', '') for p in relpaths})
    
    tree: Dict[str, List[str]] = defaultdict(list)
    for ns in namespaces:
        parts = ns.split('/')
        for i in range(1, len(parts)):
            parent = '/'.join(parts[:i])
            child = '/'.join(parts[: i + 1])
            if child not in tree[parent]:
                tree[parent].append(child)
        if len(parts) == 1 and ns not in tree:
            tree.setdefault('', []).append(ns)
    
    def write_namespace(path_parts: List[str], depth: int) -> None:
        ns_path = '/'.join(path_parts)
        href = (ns_path + '.adoc') if ns_path else 'index.adoc'
        stars = '*' * (depth + 1)
        label = path_parts[-1] if path_parts else 'index'
        lines.append(f"{stars} xref:{href}[`{label}`]")
        
        for child in sorted(tree.get(ns_path, [])):
            write_namespace(child.split('/'), depth + 1)
        
        if not namespace_map:
            return
        
        ns_key = ns_path.replace('/', '::')
        syms = sorted(namespace_map.get(ns_key, []), key=lambda x: x['label'])
        groups: Dict[str, List[Dict[str, str]]] = defaultdict(list)
        for s in syms:
            groups[s['label']].append(s)
        
        for label, items in groups.items():
            parent = items[0]['target']
            parent_stars = '*' * (depth + 2)
            lines.append(f"{parent_stars} xref:{parent}[`{label}`]")
            
            child_stars = '*' * (depth + 3)
            if len(items) == 1:
                try:
                    rep = Path(parent)
                    base = rep.stem.split('-', 1)[0]
                    dirp = (base_dir / rep.parent).resolve()
                    overloads = [p for p in sorted(dirp.iterdir()) if p.suffix == '.adoc' and p.stem.startswith(base + '-')]
                    for p in overloads:
                        if str(p).endswith(parent):
                            continue
                        over_href = str((rep.parent / p.name)).replace('\\', '/')
                        lines.append(f"{child_stars} xref:{over_href}[`{label}`]")
                except Exception:
                    pass
            else:
                for item in items:
                    lines.append(f"{child_stars} xref:{item['target']}[`{label}`]")
    
    for t in sorted(tree.get('', [])):
        write_namespace(t.split('/'), 1)
    
    out_file.parent.mkdir(parents=True, exist_ok=True)
    text = "\n".join(lines) + "\n"
    text = _replace_macros_in_text(text)
    out_file.write_text(text, encoding="utf-8")

def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Generate header .adoc files from Doxygen XML")
    p.add_argument("--only-clean", action='store_true')
    p.add_argument("--input", default="build", 
                    help="Path to directory containing reference.tag.xml and .adoc files")
    p.add_argument("--out", default="modules/api/pages", 
                    help="Output directory for generated .adoc files")
    p.add_argument("--nav-out", default="modules/api/nav.adoc", 
                    help="Path to write generated nav.adoc (relative to input folder unless absolute)")
    return p.parse_args()


def main() -> None:
    args = parse_args()
    base_dir = Path(args.input).resolve()

    if not base_dir.exists():
        print(f"Input directory not found: {base_dir}")
        raise SystemExit(1)
    
    output_root = Path(args.out)
    if not output_root.is_absolute():
        output_root = Path.cwd() / output_root

    nav_path = Path(args.nav_out)
    if not nav_path.is_absolute():
        nav_path = Path.cwd() / nav_path

    if nav_path.exists():
        # remove previous result
        nav_path.unlink()

    if output_root.exists():
        # remove previous results
        shutil.rmtree(output_root, ignore_errors=True)

    if args.only_clean: 
        # done
        return


    output_root.mkdir(parents=True, exist_ok=True)    
    headers_symbols, namespace_map, namespace_files = collect_symbols_from_overviews(base_dir, output_root)
    
    for p in sorted(base_dir.rglob('*.adoc')):
        try:
            _copy_and_patch_to_output(p, base_dir, output_root)
        except Exception:
            pass
    
    try:
        xml_path = base_dir / 'reference.tag.xml'
        if not xml_path.exists():
            print(f"Warning: reference.tag.xml not found in {base_dir}")
        if xml_path.exists():
            dst_xml = output_root / xml_path.name
            if not dst_xml.exists() or dst_xml.read_bytes() != xml_path.read_bytes():
                dst_xml.write_bytes(xml_path.read_bytes())
                print(f"Copied {xml_path} -> {dst_xml}")
    except Exception:
        pass
    
    headers_dir = output_root / 'headers'
    write_outputs(headers_symbols, headers_dir, base_dir)
    print(f"Wrote {len(headers_symbols)} header files to {headers_dir}")

    generate_nav(base_dir, list(headers_symbols.keys()), namespace_files, nav_path, namespace_map)
    print(f"Wrote nav to {nav_path}")


if __name__ == "__main__":
    main()
