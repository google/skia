---
title: 'How to download Skia'
linkTitle: 'Download'

weight: 10
menu:
  main:
    weight: 50
---

## Install `depot_tools` and Git

Follow the instructions on [Installing Chromium's
depot_tools](http://www.chromium.org/developers/how-tos/install-depot-tools)
to download `depot_tools` (which includes gclient, git-cl, and Ninja).
Below is a summary of the necessary steps.

<!--?prettify lang=sh?-->

    git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
    export PATH="${PWD}/depot_tools:${PATH}"

`depot_tools` will also install Git on your system, if it wasn't installed
already.

### Install `bazelisk`
If you intend to add or remove files, or change #includes, you will need to use Bazel to
regenerate parts of the BUILD.bazel files. Instead of installing Bazel manually, we recommend
you install [Bazelisk](https://github.com/bazelbuild/bazelisk#installation), which will fetch the
appropriate version of [Bazel](https://bazel.build/) for you (as specified by //.bazelversion).

### Install `ninja`
Ninja can be supplied using `gclient` or with `bin/fetch-ninja`.

## Clone the Skia repository

Skia can either be cloned using `git` or the `fetch` tool that is
installed with `depot_tools`.

<!--?prettify lang=sh?-->

    git clone https://skia.googlesource.com/skia.git
    # or
    # fetch skia
    cd skia
    python3 tools/git-sync-deps
    bin/fetch-ninja

## Getting started with Skia

You will probably now want to [build](../build) Skia.

## Changing and contributing to Skia

At this point, you have everything you need to build and use Skia! If
you want to make changes, and possibly contribute them back to the Skia
project, read [How To Submit a Patch](/docs/dev/contrib/submit/).
