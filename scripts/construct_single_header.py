#!/usr/bin/env python3

import argparse
import os
import re
import sys


def remove_include_guards(content):
    # Remove #pragma once
    content = re.sub(r'#pragma\s+once[ \t]*\n?', '', content)
    return content


def find_include_root(file_path):
    """Walk up from file_path to find the directory that contains an 'rsl/' subdirectory."""
    abs_path = os.path.realpath(file_path)
    directory = os.path.dirname(abs_path)
    while directory != os.path.dirname(directory):  # not filesystem root
        if os.path.isdir(os.path.join(directory, 'rsl')):
            return directory
        directory = os.path.dirname(directory)
    return None


class SingleHeaderBuilder:
    def __init__(self, include_root):
        # include_root is the directory containing the 'rsl/' folder
        self.include_root = include_root
        self.processed = set()  # canonicalized paths of already-inlined files

    def process_file(self, file_path):
        """Process a file and return its content with all rsl includes inlined."""
        real_path = os.path.realpath(file_path)
        if real_path in self.processed:
            return ''  # already inlined, skip
        self.processed.add(real_path)

        print(f"Processing '{file_path}'...")

        with open(file_path, 'r') as f:
            content = f.read()

        content = remove_include_guards(content)
        content = self._inline_includes(content, file_path)
        return content

    def _inline_includes(self, content, current_file):
        """Replace rsl includes and relative quoted includes with their inlined content."""
        result = []
        current_dir = os.path.dirname(os.path.realpath(current_file))

        for line in content.splitlines(keepends=True):
            stripped = line.strip()

            # Match #include <rsl/...>
            m = re.match(r'#include\s+<rsl/([^>]+)>', stripped)
            if m:
                include_name = m.group(1)
                include_path = os.path.join(self.include_root, 'rsl', include_name)
                if os.path.isfile(include_path):
                    inlined = self.process_file(include_path)
                    if inlined:
                        result.append(f"\n// ---- begin: rsl/{include_name} ----\n")
                        result.append(inlined)
                        result.append(f"// ---- end: rsl/{include_name} ----\n\n")
                else:
                    print(f"  Warning: <rsl/{include_name}> not found at '{include_path}', keeping include")
                    result.append(line)
                continue

            # Match #include "..."
            m = re.match(r'#include\s+"([^"]+)"', stripped)
            if m:
                rel_path = m.group(1)
                include_path = os.path.join(current_dir, rel_path)
                if os.path.isfile(include_path):
                    inlined = self.process_file(include_path)
                    if inlined:
                        result.append(f"\n// ---- begin: {rel_path} ----\n")
                        result.append(inlined)
                        result.append(f"// ---- end: {rel_path} ----\n\n")
                else:
                    print(f"  Warning: \"{rel_path}\" not found at '{include_path}', keeping include")
                    result.append(line)
                continue

            result.append(line)

        return ''.join(result)


def main():
    parser = argparse.ArgumentParser(description='Construct a single header file for a given header file')
    parser.add_argument('input_header', help='The input header file to process')
    parser.add_argument('output_header', help='The output single header file to create')
    parser.add_argument('--include-root', help='Root directory for <rsl/...> includes (default: auto-detect)')

    args = parser.parse_args()

    if not os.path.isfile(args.input_header):
        print(f"Error: Input header file '{args.input_header}' does not exist.")
        sys.exit(1)

    if args.include_root:
        include_root = args.include_root
    else:
        include_root = find_include_root(args.input_header)
        if include_root is None:
            print("Error: Could not auto-detect include root (no parent directory contains 'rsl/'). "
                  "Use --include-root.")
            sys.exit(1)
        print(f"Auto-detected include root: '{include_root}'")

    print(f"Processing '{args.input_header}' -> '{args.output_header}'...")

    builder = SingleHeaderBuilder(include_root)
    content = builder.process_file(args.input_header)

    with open(args.output_header, 'w') as f:
        f.write(content)

    print(f"Single header file '{args.output_header}' created successfully.")


if __name__ == "__main__":
    main()
