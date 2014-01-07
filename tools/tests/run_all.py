#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Run all self-tests that were written in Python, raising an exception if any
of them fail.
"""

import render_pictures_test
import skimage_self_test

def main():
  """Run all self-tests, raising an exception if any of them fail."""
  render_pictures_test.main()
  skimage_self_test.main()

if __name__ == '__main__':
  main()
