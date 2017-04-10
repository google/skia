Skia Recipes
============

These are the top-level scripts which run inside of Swarming tasks to perform
all of Skia's automated testing.

To run recipes locally:

$ python recipes.py run --workdir=/tmp/<workdir> <recipe name without .py> key1=value1 key2=value2 ...

Each recipe may have its own required properties which must be entered as
key/value pairs in the command.

When you change a recipe, you generally need to re-train the simulation test:

$ python recipes.py simulation_test train
