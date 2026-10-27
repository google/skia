# Skia C++ Unit Testing

Skia uses a custom unit testing framework. Most tests are located in the `tests/` directory and use the `DEF_TEST` macro.

## Writing a Test

```cpp
#include "tests/Test.h"

DEF_TEST(MyNewTest, reporter) {
    REPORTER_ASSERT(reporter, 1 + 1 == 2);
    if (false) {
        ERRORF(reporter, "Something went wrong!");
    }
}
```

- `DEF_TEST(name, reporter)`: Defines a test function.
- `reporter`: A pointer to an `skiatest::Reporter` used to report successes, failures, and messages.

## Assertion Macros
- `REPORTER_ASSERT(reporter, cond)`: Basic assertion.
- `ERRORF(reporter, "msg", ...)`: Reports an error with printf-style formatting.

## Adding to the Build
When you create a new test file in `tests/`, you **must** add it to the `tests_sources` list in `gn/tests.gni` for it to be included in the build.

```bash
# Example: Add tests/MyNewTest.cpp to gn/tests.gni
```

## Running Tests
Tests are run using `dm` with the `tests` source.

```bash
out/Debug/dm --src tests --match MyNewTest
```

## Advanced Test Macros
- `DEF_GANESH_TEST(name, reporter, options, ...)`: For tests that require a Ganesh GPU context.
- `DEF_GRAPHITE_TEST(name, reporter, ...)`: For tests that require a Graphite context.
