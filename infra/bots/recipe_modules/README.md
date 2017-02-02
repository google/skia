Skia Recipe Modules
===================

This directory contains recipe modules designed to be used by recipes. They
are all Skia-specific and some are interrelated:

  * vars - Common variables used by Skia recipes.
  * run - Utilities for running commands. Depends on vars.
  * flavor - Run meta-commands for various platforms. Depends on vars and run.
  * skia - Main module for Skia recipes. Depends on vars, run, and flavor.
  * swarming - Utilities for running Swarming tasks.
