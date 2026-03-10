#pragma once
#include <rsl/macro>

#if $os_is(LINUX)
#  include <unistd.h>
#  include <sys/ioctl.h>
#elif $os_is(WINDOWS)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

#  ifndef STDIN_FILENO
#    define STDIN_FILENO _fileno(stdin)
#  endif
#  ifndef STDOUT_FILENO
#    define STDOUT_FILENO _fileno(stdout)
#  endif
#  ifndef STDERR_FILENO
#    define STDERR_FILENO _fileno(stderr)
#  endif
#else
#  error "Unsupported platform"
#endif

namespace rsl::_impl_platform {
inline bool isatty(int fd) {
#if $os_is(LINUX)
  return bool(::isatty(fd));
#elif $os_is(WINDOWS)
  return bool(::_isatty(fd));
#else
#  error "Unsupported platform"
#endif
}

struct TerminalDimensions {
  int rows;
  int columns;
};

inline TerminalDimensions get_terminal_dimensions() {
  if (not isatty(STDOUT_FILENO)) {
    return {-1, -1};
  }
#if $os_is(LINUX)
  struct winsize buffer;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &buffer);
  return {.rows = buffer.ws_row, .columns = buffer.ws_col};
#elif $os_is(WINDOWS)
  CONSOLE_SCREEN_BUFFER_INFO csbi;

  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  return {.rows    = csbi.srWindow.Bottom - csbi.srWindow.Top + 1,
          .columns = csbi.srWindow.Right - csbi.srWindow.Left + 1};
#endif
}

}  // namespace rsl::_impl_platform