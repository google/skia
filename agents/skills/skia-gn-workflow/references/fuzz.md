# Debugging Fuzz Cases & Writing Reproduction Tests

When debugging a crash or assertion failure from a fuzzer (like Clusterfuzz), use this flow to robustly reproduce, diagnose, and fix the issue:

## 1. Build and Run the Fuzzer
Compile the fuzzer tool in Debug mode and execute it on the fuzzed testcase file:
```bash
ninja -C out/Debug fuzz
out/Debug/fuzz --bytes <path_to_testcase>
```

ASAN may also be useful or sometimes ASAN+Release. See ./build_args.md for more.

## 2. Add Precise Debug Logging
If the crash occurs during complex mathematical transformations (like perspective projections) or with extreme coordinate values, do not rely on standard printing or `.dump()`, which can round small/denormalized values to `0.0000`.

Instead:
- Insert temporary logging in the execution paths (using `SKIA_LOG_D`) to print matrix elements and path coordinates using exact scientific notation (`%e`).
- **Use exact Hexadecimal representation**: Call `path.dumpHex()` to print the path as a sequence of builder commands formatted using `SkBits2Float(0x...)` for each coordinate and weight.

## 3. Write a Reproduction Unit Test
Using the exact logged values, write a reproduction unit test (`DEF_TEST`) in a relevant test file (such as `tests/PathTest.cpp`). Strive to use public APIs to set up the exact input state. It is common to use the bug number in the test name (ask the user for this)

When specifying extreme, denormalized, or precise float values (such as those output by `path.dumpHex()`), use the `SkBits2Float(0x...)` format directly in your C++ test code to prevent parsing or precision mismatch during compilation:
```cpp
DEF_TEST(path_b183475561634, reporter) {
    SkPath path = SkPathBuilder()
        .moveTo(SkBits2Float(0x7dfee5c4), SkBits2Float(0x417ea0a5))
        .conicTo(SkBits2Float(0x7dfee5c4), SkBits2Float(0x417ea0a5),
                 SkBits2Float(0x7e8ebca2), SkBits2Float(0x417ea0a5),
                 0.0f)
        .detach();
    ...
}
```

## 4. Verify the Crash
Confirm that your reproduction test crashes in the test runner:
```bash
ninja -C out/Debug dm
out/Debug/dm --src tests --match <your_test_name>
```

## 5. Implement the Fix and Verify
Implement the fix, verify that the unit test passes, and ensure the fuzzer tool also terminates successfully on the testcase.

## 6. Clean Up
Always remove all temporary debug `SKIA_LOG_D` statements before finalizing the changes.
