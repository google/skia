# GYP Hacking

Note that the instructions below assume that you have the Chromium
[depot tools](http://dev.chromium.org/developers/how-tos/depottools)
installed and configured.
If you don't, you do not pass go, and you cannot collect your $200.

## Getting the sources

Best is to use git to hack on anything, you can set up a git clone of GYP
as follows:

```
git clone https://chromium.googlesource.com/external/gyp.git
cd gyp
```

## Testing your change

GYP has a suite of tests which you can run with the provided test driver
to make sure your changes aren't breaking anything important.

You run the test driver with e.g.

```
python gyptest.py
python gyptest.py test/win  # Only run Windows-related tests.
python gyptest.py -a -f ninja  # Only run ninja-related tests.
```

See [Testing](Testing) for more details on the test framework.

Note that it can be handy to look at the project files output by the tests
to diagnose problems. The easiest way to do that is by kindly asking the
test driver to leave the temporary directories it creates in-place.
This is done by setting the enviroment variable "PRESERVE", e.g.

```
set PRESERVE=all     # On Windows
export PRESERVE=all  # On saner platforms.
```

## Reviewing your change

All changes to GYP must be code reviewed before submission, GYP uses rietveld.

Upload your change with:

```
git cl upload
```

## Submitting

Once you're ready to submit, you can use the GYP try bots to test your change
with e.g.

```
git try
```

Once the change has been approved (LGTMed) and passes trybots, you can submit
it with:

```
git cl land
```

To be allowed to submit, you will need committer rights in the project. You
need to do the new password dance at
https://chromium.googlesource.com/new-password .

## Migrating from an old with-svn checkout

Remove the [svn] entry from .git/config, and the .git/svn subdirs to avoid
having `git cl land` complain that it looks like the repo is a SVN one. It might
be easier to just repull instead.
