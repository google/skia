Skia Recipe Modules
===================

This directory contains recipe modules designed to be used by recipes (see
infra/bots/recipes). They are all Skia-specific and some are interrelated:

  * builder_name_schema - Helps to derive expected behavior from task (formerly
      builder) names.
  * core - Use as a starting point for most recipes: runs setup and sync steps.
  * ct - Shared Cluster Telemetry utilities.
  * flavor - Allows the caller to specify a high-level command to run, leaving
      the platform-specific details to be handled by the specific flavor
      module.
  * infra - Shared infrastructure-related utilities.
  * run - Utilities for running commands.
  * swarming - Utilities for running Swarming tasks.
  * vars - Common global variables used by Skia recipes/modules.

When you change a recipe module, you generally need to re-train the simulation
test:

	$ python infra/bots/recipes.py test run --train

Or:

	$ cd infra/bots; make train

Each recipe module contains a few files:

  * api.py - This is the meat of the module.
  * \_\_init\_\_.py - Contains a single DEPS variable, indicating the other
      recipe modules on which this module depends.
  * example.py - Optional, this file contains examples which demonstrate how to
      use the module and should contain enough tests to achieve 100% coverage
      for the module. The tests are run using the recipes test command above.
