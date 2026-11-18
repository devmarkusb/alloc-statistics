# alloc-statistics

<!-- markdownlint-disable-next-line line-length -->
![Continuous Integration Tests](https://github.com/devmarkusb/alloc-statistics/actions/workflows/ci.yml/badge.svg) ![Lint Check (pre-commit)](https://github.com/devmarkusb/alloc-statistics/actions/workflows/pre-commit-check.yml/badge.svg) [![Coverage](https://coveralls.io/repos/github/devmarkusb/alloc-statistics/badge.svg?branch=main)](https://coveralls.io/github/devmarkusb/alloc-statistics?branch=main)

Small C++ library that overrides global `new`/`delete` and records process allocation statistics.

It tracks:

- `new` and `delete` call counts
- total allocated and deallocated bytes
- outstanding bytes (`allocated_size() - deallocated_size()`)
- peak allocated bytes

Linking `mb::alloc-statistics` installs the global allocation hooks for that executable or shared object. Use it only
when you intentionally want process-wide allocation accounting.

## Usage

```cpp
#include "mb/alloc-statistics/alloc-statistics.hpp"

#include <iostream>
#include <memory>

namespace as = mb::alloc_statistics;

int main() {
    auto& stats = as::Statistics::instance();
    const auto new_calls_before = stats.new_calls();
    const auto allocated_before = stats.allocated_size();

    auto value = std::make_unique<int>(42);

    std::cout << "new calls: " << stats.new_calls() - new_calls_before << '\n';
    std::cout << "allocated bytes: " << stats.allocated_size() - allocated_before << '\n';
    std::cout << "process peak bytes: " << stats.peak_size() << '\n';
}
```

With CMake:

```cmake
target_link_libraries(your-target PRIVATE mb::alloc-statistics)
```

## Build

This repository uses `devenv` as a submodule:

```bash
git submodule update --init --recursive
```

Or run `./devenv/scripts/git-sub.sh` to update submodules and pull Git LFS files when the repo uses them.

Configure, build, and test with a preset:

```bash
cmake --preset gcc-debug
cmake --build build/gcc-debug
ctest --preset gcc-debug
```

Other presets include `clang-debug`, `clang-release`, `appleclang-debug`, `appleclang-release`, `msvc-debug`, and
`msvc-release`.

## CMake Options

| Option                               | Default             | Description            |
|--------------------------------------|---------------------|------------------------|
| `MB_ALLOC_STATISTICS_BUILD_TESTS`    | `ON` when top-level | Build tests.           |
| `MB_ALLOC_STATISTICS_BUILD_EXAMPLES` | `ON` when top-level | Build example targets. |

## Notes

- The library is a static target named `mb.alloc-statistics` with alias `mb::alloc-statistics`.
- Referencing `Statistics::instance()` pulls the implementation and hooks from the static archive. If no API symbol is
  referenced, use your linker's whole-archive/force-load mechanism rather than assuming the archive will be extracted.
- Cross-shared-object accounting requires every participating module to resolve the same replacement operators and singleton.
  Never pair an allocation with a different global `new`/`delete` implementation; class-specific allocation functions and
  allocators that bypass global `new` are outside this instrumentation.
- Counter reads are individually atomic, but separate reads do not form a point-in-time snapshot during concurrent activity.
- `reset()` requires a quiescent state: no tracked allocation may remain live, and no thread may concurrently allocate,
  deallocate, or reset statistics. Prefer before/after counter deltas for workload reports, as in the example above.
- Throwing global allocation replacements immediately throw `std::bad_alloc` when allocation fails; nothrow forms return
  `nullptr`. They intentionally do not invoke an installed `new_handler` or retry failed allocations.
- Public headers live under `include/mb/alloc-statistics/`.
- GCC, Clang, and clang-libc++ presets use C++26; AppleClang and MSVC presets use C++23.
- The selected language standard is required; the compiler must support it rather than silently using an older mode.
- TSan builds test the isolated bookkeeping implementation only: the TSan runtime owns global `new`/`delete`, so
  interception-based tests are exercised by the ASan/UBSan and non-sanitizer configurations instead.
- `examples/usage.cpp` contains a complete allocation report example.
