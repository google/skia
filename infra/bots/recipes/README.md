Skia Recipes
============

These are the top-level scripts which run inside of Swarming tasks to perform
all of Skia's automated testing.

To run a recipe locally:

	$ python infra/bots/recipes.py run --workdir=/tmp/<workdir> <recipe name without .py> key1=value1 key2=value2 ...

Each recipe may have its own required properties which must be entered as
key/value pairs in the command.

When you change a recipe, you generally need to re-train the simulation test:

	$ python infra/bots/recipes.py test run --train

Or:

        $ cd infra/bots; make train

The test generates expectations files for the tests contained within each
recipe which illustrate which steps would run, given a particular set of inputs.
Pay attention to the diffs in these files when making changes to ensure that
your change has the intended effect.
