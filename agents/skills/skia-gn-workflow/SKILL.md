---
name: skia-gn-workflow
description: Use this skill for building Skia with GN. Use this when asked to compile, run tests, run performance benchmarks or other Skia tools.
---

# skia-gn-workflow

This skill provides expert guidance for building and testing Skia using the GN build system and Ninja. It covers common workflows for correctness testing (`dm`), performance benchmarking (`nanobench`), and interactive visualization (`viewer`).

## Core Principles
- **Hermeticity**: Always use `./bin/gn` for configuration and `./bin/fetch-ninja` to ensure the correct version of Ninja is available.
- **Verification**: Always build and run tests/benchmarks before documenting or finalizing changes.
- **Filtering**: Use the `--match` flag (aliased as `-m`) to target specific tests and reduce execution time.

## Quick Start: Build & Test
1. **Sync Dependencies**:
   ```bash
   python3 tools/git-sync-deps
   ```
2. **Fetch Ninja**:
   ```bash
   ./bin/fetch-ninja
   ```
3. **Configure Build**:
   ```bash
   ./bin/gn gen out/Debug
   ```
4. **Build Tool**:
   ```bash
   ninja -C out/Debug dm
   ```
5. **Run Test**:
   ```bash
   out/Debug/dm --src tests --match MyTest
   ```

## Workflows

### 1. Building Skia
See [Build Arguments](references/build_args.md) for common configurations like Release and Sanitizers.
- **Debug**: `./bin/gn gen out/Debug`
- **Release**: `./bin/gn gen out/Release --args='is_debug=false'`
- **ASAN**: `./bin/gn gen out/ASAN --args='sanitize="ASAN"'`

### 2. Correctness Testing (`dm`)
`dm` is used for unit tests and GMs.
- **Source types**: `tests` (unit tests), `gm` (graphics tests).
- **Match pattern**: Case-sensitive substring.
- **Verbose**: Use `-v` or `--verbose` for detailed output.
- See [dm Usage](references/dm_usage.md) for more details.

### 3. Performance Benchmarking (`nanobench`)
Use `nanobench` to measure execution time.
- **Samples**: `--samples 10`
- **Profiling**: Use `--cpuprofile <file>` (requires `skia_use_pprof=true`).
- See [nanobench Usage](references/nanobench_usage.md) for more details.

### 4. Interactive Visualization (`viewer`)
Use `viewer` to inspect GMs, SKPs, and SVGs interactively.
- **Hotkeys**: `i` for HUD, `z` for zoom, `[`/`]` for navigation.
- **Shader Debugging**: Use the Shaders window in the HUD to view and edit SkSL.
- See [viewer Usage](references/viewer_usage.md) for more details.

### 5. Writing Tests
- **Unit Tests**: Use `DEF_TEST` in `tests/`. See [Testing](references/testing.md).
- **GMs**: Use `DEF_GM` in `gm/`. See [GMs](references/gms.md).

## Available Resources
- [Build Arguments](references/build_args.md)
- [dm Usage](references/dm_usage.md)
- [nanobench Usage](references/nanobench_usage.md)
- [viewer Usage](references/viewer_usage.md)
- [Testing](references/testing.md)
- [GMs](references/gms.md)
- [Other Tools](references/other_tools.md)
