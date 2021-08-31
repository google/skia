#Fuzzing
In this folder, we keep our _fuzzers_ (bits of code that takes a randomized input and executes code
randomly, focusing on specific APIs). For example, we have a codec fuzzer which takes a mutated
png/jpeg or similar file and attempts to turn it into an `SkImage`. We also have a canvas fuzzer
which takes in a random set of bytes and turns them into calls on `SkCanvas`.

## Executables
These fuzzers are packaged in two different ways (see //BUILD.gn). There is a `fuzz` executable
that contains all fuzzers and is a convenient way to reproduce fuzzer-reported bugs. There are also
single fuzzer executables containing exactly one fuzzer, which are convenient to build with
[libfuzzer](https://llvm.org/docs/LibFuzzer.html).

See [../site/dev/testing/fuzz.md] for more information on building and running fuzzers using the
`fuzz` executable.

## Continuous Running
We fuzz Skia using [OSS-Fuzz](https://github.com/google/oss-fuzz), which in turn uses fuzzing
engines such as libfuzzer, afl-fuzz, hong-fuzz, and others to fuzz Skia. OSS-fuzz will automatically
[file and close bugs](https://bugs.chromium.org/p/oss-fuzz/issues/list?q=label:Proj-skia) when
it finds issues.

There is a [Skia folder](https://github.com/google/oss-fuzz/tree/master/projects/skia)
in the OSS-Fuzz repo that we make changes to when we want to add/remove/change the fuzzers that
are automatically run.
[This](https://google.github.io/oss-fuzz/getting-started/new-project-guide/#testing-locally)
describes how to test the OSS-Fuzz build and fuzzers locally using Docker.

When enabling a fuzzer in OSS-Fuzz, we typically need to follow these steps:
  1. *Add a seed corpus to `gs://skia-fuzzer/oss-fuzz/` (in the
     [skia-public project](https://console.cloud.google.com/storage/browser/skia-fuzzer?project=skia-public)).
     Make sure the corpus file is public-readable. It is easiest to add this permission via the web
     UI. This is done by granting the allUsers "name" the Reader role to the zip file. See the infra
     team if you do not have access to this bucket.
  2. *Update [the Dockerfile](https://github.com/google/oss-fuzz/blob/master/projects/skia/Dockerfile)
     to download the seed corpus to the build image.
  3. Update [build.sh](https://github.com/google/oss-fuzz/blob/628264df27f53cc60fcb27406a2da05d2197c025/projects/skia/build.sh#L99)
     to build the desired fuzzer target and move it into $OUT. If there is a seed corpus, move
     it into $OUT and make sure it is the same name as the fuzzer executable with `_seed_corpus.zip`
     as a suffix.

*For fuzzers who depend strongly on the format of the randomized data, e.g. image decoding, SkSL
parsing. These are called _binary fuzzers_, as opposed to _API fuzzers_.

Example PRs for adding fuzzers: [binary](https://github.com/google/oss-fuzz/pull/4108),
[API](https://github.com/google/oss-fuzz/pull/5657)

There is also an [OSS-fuzz folder](https://github.com/google/oss-fuzz/tree/master/projects/skcms)
set up for the [skcms repo](https://skia.googlesource.com/skcms/). The build process is similar,
except instead of compiling using GN targets, the build.sh script compiles the fuzz executables
directly.

### OSS-Fuzz dashboard
<https://oss-fuzz.com/fuzzer-stats> is useful to see metrics on how our fuzzers are running. It
shows things like executions per second (higher is better), edge coverage percent per fuzzer,
what percent of fuzzing runs end in OOM/timeout/crash, the entire corpus of fuzzed inputs
(corpus_backup), etc. Contact aarya@ to get permission to view this dashboard if necessary.
Here are some example dashboards:

 - [Per Fuzzer summary for all Skia fuzzers driven by libFuzzer](https://oss-fuzz.com/fuzzer-stats?group_by=by-fuzzer&date_start=2021-08-16&date_end=2021-08-22&fuzzer=libFuzzer&job=libfuzzer_asan_skia&project=skia)
 - [Five day summary of sksl2glsl driven by afl-fuzz](https://oss-fuzz.com/fuzzer-stats?group_by=by-day&date_start=2021-08-16&date_end=2021-08-22&fuzzer=afl_skia_sksl2glsl&job=afl_asan_skia&project=skia)

That dashboard also has a Coverage Report. Even though it appears the Coverage report is per fuzzer,
the reports always show the aggregated coverage from all fuzzers.
[Example coverage report from 2021 Aug 22](https://storage.googleapis.com/oss-fuzz-coverage/skia/reports/20210822/linux/report.html)

## See Also
  - [Creating a binary fuzzer](https://docs.google.com/document/d/1QDX0o8yDdmhbjoudNsXc66iuRXRF5XNNqGnzDzX7c2I/edit)
  - [Creating an API fuzzer](https://docs.google.com/document/d/1e3ikXO7SwoBsbsi1MF06vydXRlXvYalVORaiUuOXk2Y/edit)