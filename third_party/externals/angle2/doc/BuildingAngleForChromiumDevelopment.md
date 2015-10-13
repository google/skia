# How to build ANGLE in Chromium for dev

## Introduction

ANGLE is normally built on Windows but parts of it are cross platform including the shader validator and translator. These parts can be built and tested inside a Chromium checkout.

## Details

Steps:

  * Checkout and build [Chromium](http://dev.chromium.org/Home).
  * To setup run these commands (note similarity to [DevSetup](DevSetup.md)):

```bash
cd src/third_party/angle
gclient config --name . --unmanaged https://chromium.googlesource.com/angle/angle.git

gclient sync
git checkout master
```

  * To make the build files run these commands

```bash
cd src/third_party/angle
GYP_GENERATORS=ninja gclient runhooks
```

  * To build

```bash
cd src/third_party/angle
ninja -j 10 -k1 -C out/Debug
```

  * To build a specific target add the target at the end:

```bash
cd src/third_party/angle
ninja -j 10 -k1 -C out/Debug angle_compiler_tests
```

  * To run

```bash
cd src/third_party/angle
./out/Debug/angle_compiler_tests
```

## Working with Top of Tree ANGLE in Chromium

If you're actively developing within ANGLE in your Chromium workspace you will want to work with top of tree ANGLE. To do this do the following:

  * Ignore ANGLE in your `.gclient`

```python
solutions = [
  {
    # ...
    u'custom_deps':
    {
      "src/third_party/angle": None,
    },
  },
]
```

You then have full control over your ANGLE workspace and are responsible for running all git commands (pull, rebase, etc.) for managing your branches.

If you decide you need to go back to the DEPS version of ANGLE:

  * Comment out the `src/third_party/angle` line in your `custom_deps`.
  * Go into your ANGLE workspace and switch back to the master branch (ensure there are no modified or new files).
  * `gclient sync` your Chromium workspace.
