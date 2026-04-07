# alloc-statistics

<!-- markdownlint-disable-next-line line-length -->
![Continuous Integration Tests](https://github.com/devmarkusb/alloc-statistics/actions/workflows/ci.yml/badge.svg) ![Lint Check (pre-commit)](https://github.com/devmarkusb/alloc-statistics/actions/workflows/pre-commit-check.yml/badge.svg) [![Coverage](https://coveralls.io/repos/github/devmarkusb/alloc-statistics/badge.svg?branch=main)](https://coveralls.io/github/devmarkusb/alloc-statistics?branch=main)

A generic C++ library template. Contains the useful basic stuff
probably needed for any C++ library. Especially a jump start
towards connecting to a basic professional infrastructure
for building and CI.

By default, this builds a **static** library with a split header and `.cpp` implementation. Set
`MB_ALLOC_STATISTICS_HEADER_ONLY=ON` to build the same API as a **header-only** `INTERFACE` library (no object code in
the library target; implementation is `inline` in the generated public header).

## Quick start

The repo uses submodules (e.g. for `devenv`). After clone:

```bash
git submodule update --init --recursive
```

To sync submodules (and optionally Git LFS) later, from the repo root:

```bash
devenv/git-sub.sh
```

**Build** (CMake 3.30+, C++26 by default — **MSVC presets use C++23**, Ninja). From repo root:

```bash
cmake --preset gcc-debug
cmake --build build/gcc-debug
```

Run tests:

```bash
ctest --preset gcc-debug
```

**Pre-commit** (optional, for local lint/format): create a venv and install hooks:

```bash
./devenv/bootstrap.sh
```

Then `pre-commit` runs on commit; you can also run `pre-commit run -a` manually.

## Syncing with the upstream template

This repo was generated with `scripts/new-cpp-lib.py`. To pull structural improvements from the original template, run:

    git remote add template https://github.com/devmarkusb/cpp-lib-template.git
    git fetch template

Upstream repository: [devmarkusb/cpp-lib-template](https://github.com/devmarkusb/cpp-lib-template).

## CMake options

| Option                               | Default             | Description                                                                  |
|--------------------------------------|---------------------|------------------------------------------------------------------------------|
| `MB_ALLOC_STATISTICS_HEADER_ONLY`    | `OFF`               | Header-only `INTERFACE` library vs static library with sources under `src/`. |
| `MB_ALLOC_STATISTICS_BUILD_TESTS`    | `ON` when top-level | Build tests and test infra (GoogleTest).                                     |
| `MB_ALLOC_STATISTICS_BUILD_EXAMPLES` | `ON` when top-level | Build example executables.                                                   |

## Build presets

`CMakePresets.json` defines configure/build/test presets for:

- **Compilers:** GCC, Clang, Clang with libc++, AppleClang, MSVC.
- **Configurations:** Debug, RelWithDebInfo (release).
- **Preset names:** e.g. `gcc-debug`, `clang-release`, `msvc-debug`, `appleclang-release`.

C++ standard is **26** for GCC/Clang/AppleClang presets and **23** for `msvc-*` presets (matches the MSVC columns in
CI). Compile commands are exported for tooling. A dependency provider uses the **repo-root**
`fetchcontent-lockfile.json` for pinned Git dependencies (`find_package` entries, optional `cmake_include` / per-dep
`cmake_variables`; see `devenv/README.md` under **fetch-content-from-lockfile.cmake**). If you configure **`devenv/` as
the top-level CMake project** (using `devenv/CMakePresets.json`), that tree uses `devenv/fetchcontent-lockfile.json`
instead — a minimal lockfile for devenv-only dev — so edit the root lockfile when working on the library template.

Note: `SameMajorVersion` is used for the installed CMake package version file (see `mb_devenv_install_library` in
`devenv`), so consumers can typically `find_package` with a compatible major version.

## Directory structure

### `.github/`

- **`workflows/ci.yml`** — CI: preset-based build/test on Linux (GCC/Clang), macOS (AppleClang), Windows (MSVC);
  extended build-and-test matrix (GCC 15, Clang 18/21, sanitizers, coverage); install test.
- **`workflows/pre-commit-check.yml`** — Runs pre-commit on push to `main` and on pull requests.
- **`workflows/pre-commit-update.yml`** — Weekly (and manual) pre-commit hook autoupdate.

### `cmake/`

- **`mb.alloc-statistics-config.cmake.in`** — Template for the CMake config-file package (used when installing the
  library so consumers can `find_package(mb.alloc-statistics)`).

### `scripts/`

- **`new-cpp-lib.py`** — Instantiates a new library from this template (rename CMake/C++/paths, optional `README` badges
  and fresh `git` + `devenv` submodule). See [Syncing with the upstream template](#syncing-with-the-upstream-template).

### `devenv/`

Development and CI support (typically as a submodule): see
<https://github.com/devmarkusb/devenv>.

### `examples/`

- **`CMakeLists.txt`** — Builds example executables (e.g. `usage`) that link to the library.
- **`usage.cpp`** — Example program using the library (e.g. `mb::alloc_statistics::sum`).

### `include/`

- **`mb/alloc-statistics/`** — Public headers, exposed via a FILE_SET.

### `src/`

- **`CMakeLists.txt`** — Adds compiled `.cpp` sources to the static library when `MB_ALLOC_STATISTICS_HEADER_ONLY` is
  OFF; registers tests when `MB_ALLOC_STATISTICS_BUILD_TESTS` is ON.
- **`alloc-statistics.cpp`** — Library implementation (not used in header-only mode).
- **`alloc-statistics.test.cpp`** — GoogleTest sources; built only when `MB_ALLOC_STATISTICS_BUILD_TESTS` is ON.

### Root files

- **`CMakeLists.txt`** — Top-level: project, options, library target (static by default, or header-only `INTERFACE`
  when `MB_ALLOC_STATISTICS_HEADER_ONLY=ON`), install, tests, examples.
- **`CMakePresets.json`** — Configure, build, test, and workflow presets for multiple compilers and configs.
- **`fetchcontent-lockfile.json`** — Pinned Git dependencies for the CMake dependency provider (e.g. Googletest); optional
  fields include `cmake_variables` per dependency. Format is documented in `devenv/README.md`. See **Build presets**
  above for the separate minimal lockfile under `devenv/` when building devenv alone.
- **`.pre-commit-config.yaml`** — Pre-commit hooks: trailing whitespace, EOF, JSON/YAML checks, clang-format,
  gersemi (CMake), markdownlint, codespell (runs on the whole tree, including `devenv/` when checked out as a
  submodule — cheap and catches drift if you touch it). To sync `.clang-format` from
  [devmarkusb/clangformat](https://github.com/devmarkusb/clangformat) (including versioned configs), run
  `cd devenv && ./sync-clang-format.sh [VERSION]` (run from inside `devenv`).
