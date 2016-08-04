Runtime Configuration
=====================

Skia supports the configuration of various aspects of its behavior at runtime,
allowing developers to enable\/disable features, or to experiment with numerical
quantities without recompiling.

## Enabling runtime configuration

In order to use a runtime-configurable variable in your source, simply:

<!--?prettify?-->
~~~~
#include "SkRTConf.h"
~~~~

## Declaring a runtime-configurable variable

At file scope, declare your variable like so:

<!--?prettify?-->
~~~~
SK_CONF_DECLARE( confType, varName, confName, defaultValue, description );
~~~~

For example, to declare a boolean variable called ` c_printShaders ` that can be
changed at runtime, you would do something like

<!--?prettify?-->
~~~~
SK_CONF_DECLARE( bool, c_printShaders, "gpu.printShaders", false, "print the
                 source code of any internally generated GPU shaders" ); 
~~~~

It is safe to declare variables this way in header files; the variables will be
declared as static, but since they are read\-only\-ish \(they can be changed
through a special mechanism; see below\), this is safe.

## Using a runtime-configurable variable

The variables created by `SK_CONF_DECLARE` can be used in normal C\+\+ code as
if they were regular contant variables. For example:

<!--?prettify?-->
~~~~
if (c_printShaders) {
    // actually print out the shaders
}
~~~~

## Changing a runtime-configurable variable after launch

If, for some reason, you want to change the value of a runtime-configurable
variable after your program has started, you can do this with the `SK_CONF_SET`
macro:

<!--?prettify?-->
~~~~
SK_CONF_SET( "gpu.printShaders", false )
~~~~

Note that we're using the `confName` parameter to the declaration, not
`varName`. This is because this configuration option may appear in multiple
files \(especially if you declared it in a header!\), and we need to make sure
to update all variables' values, not just the one that's locally visible to the
file you are currently in.

## Changing a runtime-configurable variable before launch

This is the primary intended use of these variables. There are two ways that you
can control the values of runtime-configurable variables at launch time: a
skia.conf configuration file, or through the use of environment variables.

### Using skia.conf

The skia.conf file is a simple line-based configuration file containing
key-value pairs. It supports python-style \# comments. For our example, we might
see a configuration file that looks like:

<!--?prettify?-->
~~~~
gpu.printShaders      true
gpu.somethingElse     3.14159
matrix.invertProperly false    # math is hard
...
~~~~

*Note: boolean values may be set as 1, 0, true, or false. Other values will
result in runtime errors.*

If the skia library detects a skia.conf file at initialization time, it will
parse it and override the default values of any declared configuration variables
with the values found in the file.

*Note: although it might appear that the configuration variables have a
hierarchical naming scheme involving periods, that's just a convention I have
adopted so that when all declared configuration variables are sorted
alphabetically, they are roughly grouped by component.*

## Using environment variables

You can quickly override the value of one runtime-configurable variable using an
environment variable equal to the variable's key with "skia." prepended. So, for
example, one might run:

<!--?prettify?-->
~~~~
prompt% skia.gpu.printShaders=true out/Debug/dm
~~~~

or

<!--?prettify?-->
~~~~
prompt% export skia.gpu.printShaders=true
prompt% out/Debug/dm
~~~~

On many shells, it is illegal to have a period in an environment variable name,
so skia also supports underscores in place of the periods:

<!--?prettify?-->
~~~~
prompt% skia_gpu_printShaders=true out/Debug/dm
~~~~

or

<!--?prettify?-->
~~~~
prompt% export skia_gpu_printShaders=true`
prompt% out/Debug/dm
~~~~

## Discovering all possible configuration variables

As this system becomes more widely used in skia, there may be hundreds of
configuration variables. What are they all? What are their defaults? What do
they do?

In order to find out, simply create a zero-length skia.conf file \(on unix,
`touch skia.conf` will do the trick\). If skia detects a zero-length
configuration file, it will overwrite it with a sorted list of all known
configuration variables, their defaults, and their description strings. Each
line will be commented out and have its value already equal to its default, so
you can then edit this file to your liking.

To trigger this behavior, call the function
`skRTConfRegistry().possiblyDumpFile(); ` or simply use `SkAutoGraphics
ag;`, which also validates your configuration and print out active non-default
options.

## Are these things enabled all the time?

No, they are only enabled in builds where SK_DEBUG is defined. This includes both
`Debug` and `Release_Developer` gyp BUILDTYPES. The `Release_Developer` build type
has exactly the same build flags as `Release`, except it re-enables SK_DEBUG, which
in turn enables runtime configuration behavior.
Specifically:

<!--?prettify?-->
~~~~
prompt% ninja -C BUILDTYPE=Release_Developer
~~~~

... wait a long time ...

<!--?prettify?-->
~~~~
prompt % skia_gpu_printShaders=true out/Release_Developer/dm
~~~~

... enjoy ...

## Known issues / limitations

Lines in 'skia.conf', including comments, are limited to 1024 characters.
Runtime configuration variables of type `char \* ` cannot currently have spaces
in them.
Runtime variables are only fully supported for `int`, `unsigned int`, `float`,
`double`, `bool`, and `char \*`.
