#pragma once
#include <cstddef>
#include <meta>
#include <source_location>
#include <string_view>

namespace rsl {

class source_location {
  struct RawSloc {
    const char* file_name     = std::define_static_string("");
    const char* function_name = std::define_static_string("");
    unsigned line             = 0;
    unsigned column           = 0;
  };

  struct SourceContext : RawSloc {
    consteval SourceContext(RawSloc raw) : RawSloc(raw) {}

    constexpr virtual ~SourceContext()                                 = default;
    consteval virtual std::meta::info scope() const                    = 0;
    consteval virtual std::meta::access_context access_context() const = 0;
  };

  template <std::meta::access_context R, RawSloc Data>
  consteval static SourceContext const* make_context() { return nullptr; }

  // TODO: remove
  //? this is a workaround for clang-p2996:
  //? static member functions aren't extracted  as free functions
  template <std::meta::access_context R, RawSloc Data>
  constexpr static SourceContext const* _impl_context = make_context<R, Data>();

  explicit consteval source_location(SourceContext const* data);

public:
  // public to avoid violating rules for structural types
  SourceContext const* _impl_sloc      = nullptr;
  consteval source_location() noexcept = default;
  consteval explicit(false)
      source_location(std::source_location const& sloc,
                      std::meta::access_context ctx = std::meta::access_context::current());

  consteval static source_location current(
      std::source_location sloc     = std::source_location::current(),
      std::meta::access_context ctx = std::meta::access_context::current());

  // TODO wrap file_name and function_name in rsl::cstring_view as soon as implemented
  [[nodiscard]] constexpr char const* file_name() const noexcept;
  [[nodiscard]] constexpr char const* function_name() const noexcept;
  [[nodiscard]] constexpr unsigned line() const noexcept;
  [[nodiscard]] constexpr unsigned column() const noexcept;

  // consteval-only extensions
  [[nodiscard]] consteval std::meta::info scope() const noexcept;
  [[nodiscard]] consteval std::meta::access_context access_context() const noexcept;
};

consteval std::meta::info current_scope(
    std::meta::info ctx = std::meta::access_context::current().scope());
consteval std::meta::info current_function(std::meta::info scope = current_scope());
consteval std::meta::info current_class(std::meta::info scope = current_scope());
consteval std::meta::info current_namespace(std::meta::info scope = current_scope());

}  // namespace rsl