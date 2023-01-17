
---
title: "Using Bazel"
linkTitle: "Using Bazel"

---

## Overview

Skia is currently migrating towards using [Bazel](https://bazel.build/) as a build system, due to
the ability to more tightly control what and how gets built.

When referring to a file in this doc, we use
[Bazel label notation](https://bazel.build/concepts/labels), so to refer the file located at
`$SKIA_ROOT/docs/examples/Arc.cpp`, we would say `//docs/examples/Arc.cpp`.

## Learning more about Bazel
The Bazel docs are quite good. Suggested reading order if you are new to Bazel:
 - [Getting Started with Bazel and C++](https://bazel.build/tutorials/cpp)
 - [WORKSPACE.bazel and external deps](https://bazel.build/docs/external)
 - [Targets and Labels](https://bazel.build/concepts/labels)
 - [Understanding a build](https://bazel.build/docs/build)
 - [Configuration with .bazelrc files](https://bazel.build/docs/bazelrc)

Googlers, check out [go/bazel-bites](http://go/bazel-bites) for more tips.

## Building with Bazel

All this assumes you have [downloaded Skia](/docs/user/download), especially having synced the
third_party deps using `./tools/git-sync-deps`.

### Linux Hosts (you are running Bazel on a Linux machine)
You can run a command like:
```
bazel build //example:hello_world_gl
```

This uses a hermetic C++ toolchain we put together to compile Skia on a Linux host
(implementation is in `//toolchain`. It builds the _target_ defined in
`//examples/BUILD.bazel` named "hello_world_gl", which uses the `sk_app` framework
we designed to make simple applications using Skia.

Bazel will put this executable in `//bazel-bin/example/hello_world_gl` and tell you it did so in
the logs. You can run this executable yourself, or have Bazel run it by modifying the command to
be:
```
bazel run //example:hello_world_gl
```

If you want to pass one or more flags to `bazel run`, add them on the end after a `--` like:
```
bazel run //example:hello_world_gl -- --flag_one=apple --flag_two=cherry
```

### Mac Hosts (you are running Bazel on a Mac machine)
You can run a command like:
```
bazel build //example:bazel_test_exe
```

When building for Mac, we require the user to have Xcode installed on their device so that we can
use system headers and Mac-specific includes when compiling. Googlers, as per usual, follow the
instructions at [go/skia-corp-xcode](http://go/skia-corp-xcode) to install Xcode.

Our Bazel toolchain assumes you have `xcode-select` in your path so that we may symlink the
user's current Xcode directory in the toolchain's cache. Make sure `xcode-select -p`
returns a valid path.

## .bazelrc Tips
You should make a [.bazelrc file](https://bazel.build/docs/bazelrc) in your home directory where
you can specify settings that apply only to you. These can augment or replace the ones we define
in the `//.bazelrc` configuration file.

Skia defines some [configs](https://bazel.build/docs/bazelrc#config), that is, group of settings
and features in `//bazel/buildrc`. This file contains configs for builds that we use  regularly
(for example, in our continuous integration system).

If you want to define Skia-specific configs (and options which do not conflict with other Bazel
projects), you make a file in `//bazel/user/buildrc` which will automatically be read in. This
file is covered by a `.gitignore` rule and should not be checked in.

You may want some or all of the following entries in your `~/.bazelrc` or `//bazel/user/buildrc`
file.

### Build Skia faster locally
Many Linux machines have a [RAM disk mounted at /dev/shm](https://www.cyberciti.biz/tips/what-is-devshm-and-its-practical-usage.html)
and using this as the location for the Bazel sandbox can dramatically improve compile times because
[sandboxing](https://bazel.build/docs/sandboxing) has been observed to be I/O intensive.

Add the following to `~/.bazelrc` if you have a `/dev/shm` partition that is 4+ GB big.
```
build --sandbox_base=/dev/shm
```

### Authenticate to RBE on a Linux VM
We are in the process of setting up Remote Build Execution (RBE) for Bazel. Some users have reported
errors when trying to use RBE (via `--config=linux_rbe`) on Linux VMs such as:
```
ERROR: Failed to query remote execution capabilities:
Error code 404 trying to get security access token from Compute Engine metadata for the default
service account. This may be because the virtual machine instance does not have permission
scopes specified. It is possible to skip checking for Compute Engine metadata by specifying the
environment variable NO_GCE_CHECK=true.
```
For instances where it is not possible to set the `cloud-platform` scope
[on the VM](https://skia-review.googlesource.com/c/skia/+/525577), one can directly link to their
GCP credentials by adding the following to `~/.bazelrc` (substituting their username for &lt;user>)
after logging in via `gcloud auth login`:
```
build:remote --google_credentials=/usr/local/google/home/<user>/.config/gcloud/application_default_credentials.json
```

### Make local builds compatible with remote builds (e.g. better caching)
Add the following to `//bazel/user/buildrc` if you are on a Linux x64 box and want to be able to
share cached build results between things you build locally and build with `--config=linux_rbe`.
```
build --host_platform=//bazel/platform:linux_x64_hermetic
```
For example, if you are on a laptop, using `--config=linux_rbe` will speed up builds when you have
access to Internet, but then if you need to go offline, you can still build locally and use the
previous build results from the remote builds.
