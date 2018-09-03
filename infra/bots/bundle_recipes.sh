#!/bin/bash
# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -x -e

cd skia
git init
git add .
git commit -m "Commit Recipes"
python infra/bots/recipes.py bundle --destination ${1}/recipe_bundle
