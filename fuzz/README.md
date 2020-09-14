We fuzz Skia using oss-fuzz, which in turn uses fuzzing engines such as libfuzzer, afl-fuzz,
hong-fuzz and others.

We define a `fuzzer` to be a targeted bit of code that takes a randomized input and executes code
in a specific area. For example, we have a codec fuzzer which takes a mutated png/jpeg or similar
file and attempts to turn it into an `SkImage`. We also have a canvas fuzzer which takes in a random
set of bytes and turns them into calls on `SkCanvas`.

See [../site/dev/testing/fuzz.md] for more information on building and running fuzzers.

See also:
  - [Creating a binary fuzzer](https://docs.google.com/document/d/1QDX0o8yDdmhbjoudNsXc66iuRXRF5XNNqGnzDzX7c2I/edit)
  - [Creating an API fuzzer](https://docs.google.com/document/d/1e3ikXO7SwoBsbsi1MF06vydXRlXvYalVORaiUuOXk2Y/edit)