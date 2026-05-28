---
name: skia-add-test
description: Add a new test to the Skia repository. Use this skill when you need to create a new unit or integration test in the tests/ directory and ensure it's part of the build system.
---

# Skia Add Test Skill

This skill guides you through adding a new test to Skia. Tests are primarily written in C++ and located in the `tests/` directory.

## Workflow

### 1. Create the Test File
Create a new `.cpp` file in the `tests/` directory. Use the provided template as a starting point.
- **Template Location:** `assets/test_template.cpp`

### 2. Update the Build System
Add the path to your new test file to the `tests_sources` list in `gn/tests.gni`.
- Paths in `gn/tests.gni` should be relative to the directory or use the `$_tests` variable.

### 3. Build the Test
Compile the `dm` tool, which runs the tests.
```bash
ninja -C out/Debug dm
```

### 4. Run the Test
Use `dm` with the `--match` flag to run your specific test.
```bash
out/Debug/dm --match [test_name]
```
Note: Replace `[test_name]` with the name provided to the `DEF_TEST` (or similar) macro.

## Key Concepts

### Macros for Defining Tests
- `DEF_TEST(name, reporter)`: Standard CPU test.
- `DEF_GANESH_TEST(name, reporter, options, ctsEnforcement)`: Ganesh GPU test.
- `DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(name, reporter, context, ctsEnforcement)`: Graphite GPU test.

### CtsEnforcement
`CtsEnforcement` is used to track tests that should be run as part of Android's Conformance Test Suite (CTS) for a specific milestone (e.g., `kApiLevel_T`) or later.
- `CtsEnforcement::kNever`: The test is not part of CTS.
- `CtsEnforcement::kApiLevel_U`: The test is required for Android U and later.

### Reporter and Assertions
- `REPORTER_ASSERT(reporter, condition, ...)`: Standard assertion.
- `ERRORF(reporter, ...)`: Report a failure with a formatted message.
- `skiatest::ReporterContext`: Use this stack-allocated object to provide additional context (like subtest names) for failure reports.

Example of `ReporterContext`:
```cpp
{
    skiatest::ReporterContext subtest(reporter, SkString("My Subtest"));
    REPORTER_ASSERT(reporter, condition);
}
```
If `condition` fails, the error will include "My Subtest".
