#include <rsl/experimental/print>
struct Foo {int a; char b;};
int main() {
  rsl::println("a {[red]1} asdf {[blue]0} b", 42, 'c');
  rsl::println("a {[cyan]} asdf {[green]} b", 42, 'c');
  rsl::println("a {[yellow]!}{} sfdg {}{[reset]!} b", 42, 'c');

  rsl::println("b={b}, a={a}", Foo(42, 'c'));
  rsl::println("b={0.b}, a={0.a}", Foo(42, 'c'));
  rsl::println("b={0.1}, a={0.0}", Foo(42, 'c'));
}