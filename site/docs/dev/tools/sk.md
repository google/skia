---
title: 'SK CLI Tool'
linkTitle: 'SK CLI Tool'
---

## Introduction

`sk` is a command-line tool which provides common functionality useful for
working on Skia.

## Commands

The set of supported commands will probably grow or change over time.

### asset

Used for managing versioned non-code assets used by Skia developers and in CI.
These are stored in [CIPD](https://chrome-infra-packages.appspot.com/p/skia/bots)
and their versions are pinned under
[//infra/bots/assets](https://skia.googlesource.com/skia/+/main/infra/bots/assets)
in Skia.

* add - Add an entry for a new asset. This does not create an initial version.
* remove - Remove the entry for an existing asset. This does not remove uploaded
  versions.
* download - Download the pinned version of the asset into the given directory.
* upload - Upload a new version of the asset and update the pinned version. If
  a script exists to automate creation of the asset, `sk` that runs script and
  uploads the resulting files. Otherwise, it expects a target directory to be
  provided.
* get-version - Print the pinned version of the asset.
* set-version - Set the pinned version of the asset. `sk` verifies that the
  given version actually exists in CIPD.
* list-versions - Print all versions of the asset which exist in CIPD.

### release-branch

This automates the processes involved in creating a new release branch of Skia,
including creating the Git branch itself, setting up the commit queue on the
new branch (as well as retiring the commit queue for the oldest release branch),
and updating the current Skia milestone.  This requires administrator rights.

### try

Trigger try jobs on the current active CL.  Accepts zero or more job names or
regular expressions.  If none is provided, `try` lists all of the available try
jobs and exits.

## Development

The code for `sk` is located in the
[Skia Infra repo](https://skia.googlesource.com/buildbot). Development in that
repo follows similar practices to Skia.  See
[README.md](https://skia.googlesource.com/buildbot/+/main/README.md) for
instructions to get started.

Code for the `sk` tool itself is located under
[//sk/go/](https://skia.googlesource.com/buildbot/+/main/sk/go/). Each
sub-command has an associated package.

## Deployment

New versions of `sk` are automatically built and uploaded to
[CIPD](https://chrome-infra-packages.appspot.com/p/skia/tools/sk) as part of
Skia Infra's CI/CD pipeline.  The version used by Skia is pinned in
[DEPS](https://skia.googlesource.com/skia/+/main/DEPS) and updated by an
[autoroller](https://autoroll.skia.org/r/sk-tool-skia).
