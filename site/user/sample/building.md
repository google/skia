Building with Skia Tutorial
===========================

dsinclair@chromium.org


This document describes the steps used to create an application that uses Skia.  The assumptions are that you're using:

  * [git](http://git-scm.com)
  * [gclient](https://code.google.com/p/gclient/)
  * [gyp](https://code.google.com/p/gyp/)
  * [ninja](http://martine.github.io/ninja/)

I'm going to describe up to the point where we can build a simple application that prints out an SkPaint.

Overview
--------

  1. Create remote repository.
  1. Configure and sync using gclient.
  1. Create DEPS file to pull in third party repositories.
  1. Setup gitignore for directories pulled in from DEPS.
  1. Configure GYP.
  1. Setup GYP auto-run when gclient sync is executed.

gclient setup
-------------
The first step is to setup a remote git repo, take your pick of provider. In
my case, the repo is called UsingSkia and lives on
[bitbucket](https://bitbucket.org).

With the remote repo created, we create a .gclient configuration file. The
gclient config command will write the file for us:

    $ gclient config --name=src https://bitbucket.org/dj2/usingskia.git

This will create the following:

    solutions = [
      { "name"        : "src",
        "url"         : "https://bitbucket.org/dj2/usingskia.git",
        "deps_file"   : "DEPS",
        "managed"     : True,
        "custom_deps" : {
        },
        "safesync_url": "",
      },
    ]
    cache_dir = None

The name that we configured is the directory in which the repo will be checked
out. This is done by running gclient sync. There is a bit of magic that
gclient does around the url to determine if the repo is SVN or GIT. I've found
the use of ssh:// and the .git on the end seem to work to get the right SCM
type.

    $ gclient sync

This should execute a bunch of commands (and, in this case, may end with an
error because the repo was empty. That seems to be fine.) When finished, you
should have a src directory with your git repository checked out.

DEPS
----

With the repo created we can go ahead and create our src/DEPS file. The DEPS
file is used by gclient to checkout the dependent repositories of our
application. In this case, the Skia repository.

Create a src/DEPS file with the following:

~~~~

  vars = {
    "skia_revision": "a6a8f00a3977e71dbce9da50a32c5e9a51c49285",
  }

  deps = {
    "src/third_party/skia/":
        "http://skia.googlecode.com/skia.git@" + Var("skia_revision"),
  }

~~~~

There are two sections to the `DEPS` file at the moment, `vars` and `deps`.
The `vars` sections defines variables we can use later in the file with the
`Var()` accessor. In this case, we define our root directory, a shorter name
for any googlecode repositories and a specific revision of Skia that we're
going to use. I've pinned to a specific version to insulate the application
from changes in the Skia tree. This lets us know that when someone checks out
the repo they'll be using the same version of Skia that we've built and tested
against.

The `deps` section defines our dependencies. Currently we have one dependency
which we're going to checkout into the `src/third_party/skia` directory.

Once the deps file is created, commit and push it to the remote repository.
Once done, we can use gclient to checkout our dependencies.

    $ gclient sync

This should output a whole bunch of lines about files that are being added to
your project. This may also be a good time to create a `.gitignore` file. You
don't want to check the `third_party/skia directory` into your repository as
it's being managed by gclient.

Now, we've run into a problem. Skia itself has a `DEPS` file which defines the
`third_party` libraries it needs to build. None of those dependencies are being
checked out so Skia will fail to build.

The way I found around that is to add a second solution to the `.gclient`
file.  This solution tells gclient about Skia and will pull in the needed
dependencies. I edited my `.gclient` file (created by the `gclient config`
command above) to look as follows:

    solutions = [
      { "name"        : "src",
        "url"         : "https://bitbucket.org/dj2/usingskia.git",
        "deps_file"   : "DEPS",
        "managed"     : True,
        "custom_deps" : {
        },
        "safesync_url": "",
      },
      { "name"        : "src/third_party/skia",
        "url"         : "http://skia.googlecode.com/skia.git@a6a8f00a3977e71dbce9da50a32c5e9a51c49285",
        "deps_file"   : "DEPS",
        "managed"     : True,
        "custom_deps" : {
        },
        "safesync_url": "",
      },
    ]
    cache_dir = None

This is a little annoying at the moment since I've duplicated the repository
revision number in the `.gclient` file. I'm hoping to find a way to do this
through the `DEPS` file, but until then, this seems to work.

With that done, re-run `gclient sync` and you should see a whole lot more
repositories being checked out. The
`src/third_party/skia/third_party/externals` directory should now be
populated.

GYP
---

The final piece of infrastructure we need to set up is GYP. GYP is a build
system generator, in this project we're going to have it build ninja
configuration files.

First, we need to add GYP to our project. We'll do that by adding a new entry
to the deps section of the `DEPS` file.

    "src/tools/gyp":
        (Var("googlecode_url") % "gyp") + "/trunk@1700",

As you can see, I'm going to put the library into `src/tools/gyp` and checkout
revision 1700 (note, the revision used here, 1700, was the head revision at
the time the `DEPS` file was written. You're probably safe to use the
tip-of-tree revision in your `DEPS` file).  A quick `gclient sync` and we
should have everything checked out.
 
In order to run GYP we'll create a wrapper script. I've called this
`src/build/gyp_using_skia`.

~~~~
#!/usr/bin/python
import os
import sys

script_dir = os.path.dirname(__file__)
using_skia_src = os.path.abspath(os.path.join(script_dir, os.pardir))

sys.path.insert(0, os.path.join(using_skia_src, 'tools', 'gyp', 'pylib'))
import gyp

if __name__ == '__main__':
  args = sys.argv[1:]

  if not os.environ.get('GYP_GENERATORS'):
    os.environ['GYP_GENERATORS'] = 'ninja'

  args.append('--check')
  args.append('-I%s/third_party/skia/gyp/common.gypi' % using_skia_src)

  args.append(os.path.join(script_dir, '..', 'using_skia.gyp'))

  print 'Updating projects from gyp files...'
  sys.stdout.flush()

  sys.exit(gyp.main(args))
~~~~

Most of this is just setup code. The two interesting bits are:

  1. `args.append('-I%s/third_party/skia/gyp/common.gypi' % using_skia_src)`
  1. `args.append(os.path.join(script_dir, '..', 'using_skia.gyp'))`

In the case of 1, we're telling GYP to include (-I) the
`src/third_party/skia/gyp/common.gypi` file which will define necessary
variables for Skia to compile. In the case of 2, we're telling GYP that the
main configuration file for our application is `src/using_skia.gyp`.

The `src/using_skia.gyp` file is as follows:

~~~~
{
  'targets': [
    {
      'configurations': {
        'Debug': { },
        'Release': { }
      },
      'target_name': 'using_skia',
      'type': 'executable',
      'dependencies': [
        'third_party/skia/gyp/skia_lib.gyp:skia_lib'
      ],
      'include_dirs': [
        'third_party/skia/include/config',
        'third_party/skia/include/core',
      ],
      'sources': [
        'app/main.cpp'
      ],
      'ldflags': [
        '-lskia', '-stdlib=libc++', '-std=c++11'
      ],
      'cflags': [
        '-Werror', '-W', '-Wall', '-Wextra', '-Wno-unused-parameter', '-g', '-O0'
      ]
    }
  ]
}
~~~~

There is a lot going on in there, I'll touch on some of the highlights. The
`configurations` section allows us to have different build flags for our `Debug`
and `Release` build (in this case they're the same, but I wanted to define
them.)  The `target_name` defines the name of the build target which we'll
provide to ninja. It will also be the name of the executable that we build.

The dependencies section lists our build dependencies. These will be built
before our sources are built. In this case, we depend on the `skia_lib` target
inside `third_party/skia/gyp/skia_lib.gyp`.

The include_dirs will be added to the include path when our files are built.
We need to reference code in the config and core directories of Skia.

`sources`, `ldflags` and `cflags` should be obvious.

Our application is defined in `src/app/main.cpp` as:

~~~~
#include "SkPaint.h"
#include "SkString.h"

int main(int argc, char** argv) {
  SkPaint paint;
  paint.setColor(SK_ColorRED);

  SkString str;
  paint.toString(&str);

  fprintf(stdout, "%s\n", str.c_str());

  return 0;
}
~~~~

We're just printing out an SkPaint to show that everything is linking correctly.

Now, we can run:

    $ ./build/gyp_using_skia

And, we get an error. Turns out, Skia is looking for a `find\_mac\_sdk.py` file in
a relative tools directory which doesn't exist. Luckily, that's easy to fix
with another entry in our DEPS file.

    "src/tools/":
        File((Var("googlecode_url") % "skia") + "/trunk/tools/find_mac_sdk.py@" +
            Var("skia_revision")),

Here we using the `File()` function of `gclient` to specify that we're checking
out an individual file. Running `gclient sync` should pull the necessary file
into `src/tools`.

With that, running `build/gyp\_using\_skia` should complete successfully. You
should now have an `out/` directory with a `Debug/` and `Release/` directory inside.
These correspond to the configurations we specified in `using\_skia.gyp`.

With all that out of the way, if you run:

    $ ninja -C out/Debug using_skia

The build should execute and you'll end up with an `out/Debug/using\_skia` which
when executed, prints out our SkPaint entry.

Autorun GYP
-----------

One last thing, having to run `build/gyp\_using\_skia` after each sync is a bit of
a pain. We can fix that by adding a `hooks` section to our `DEPS` file. The `hooks`
section lets you list a set of hooks to execute after `gclient` has finished the
sync.

    hooks = [
      {
        # A change to a .gyp, .gypi or to GYP itself should run the generator.
        "name": "gyp",
        "pattern": ".",
        "action": ["python", "src/build/gyp_using_skia"]
      }
    ]

Adding the above to the end of DEPS and running gclient sync should show the
GYP files being updated at the end of the sync procedure.
