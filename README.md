# Reflection Support Library

This project contains reimplementations of various standard library facilities and other utilities to assist with reflective code.

## Overview

### <rsl/span>
`rsl::span` is a reimplementation of `std::span`. Notably `rsl::span` is a structural type, so it can be used as constant template parameter and be reflected.

Additionally `rsl::span` provides an implicit conversion operator to `std::meta::info`. This allows usage in splicers to splice in an array of all elements in the span.

### <rsl/string_view>
`rsl::string_view` is a reimplementation of `std::string_view`. Notably `rsl::string_view` is a structural type, so it can be used as constant template parameter and be reflected.

Additionally `rsl::string_view` provides an implicit conversion operator to `std::meta::info`. This allows splicing a `rsl::string_view` as array of characters.
### <rsl/variant> and <rsl/tagged_variant>
`rsl::variant` is a reflective reimplementation of `std::variant`. The underlying union is generated via `std::meta::define_aggregate`. Since this implies not having to recursively generate the underlying union, `rsl::variant` performs significantly better than all implementations of `std::variant` in terms of compile time.

`rsl::tagged_variant` allows generating variants from annotated enums. Consider the following example:
```cpp
enum class Foo {
  foo [[=^^int]],
  bar [[=^^bool]] = 10,
  baz [[=^^char]] = 20
};

int main() {
  auto obj = rsl::tagged_variant<Foo>{42};
  switch (obj) {
    case Foo::foo: return obj->foo;
    case Foo::bar: return obj->bar;
    case Foo::baz: return obj->baz;
  }
}
```
`rsl::tagged_variant` is implicitly convertible to the enum it was created from. This allows mapping the current alternative's index to the appropriate enumerator value. Additionally, the underlying union's members are named exactly like the respective enumerators. This allows unchecked access via operator `->`.


### <rsl/tuple>
`rsl::tuple` is a reflective reimplementation of `std::tuple`.