# Editor #

This is an experimental Editor layer that abstracts out SkShaper text layeout
for easy embedding into an application.  The Editor layer is agnostic about the
operating system.

    +--------------------------------+
    |Application                     |
    +-+----+-------------------------+
      |    |
      |    |
      |  +-v-------------------------+
      |  |Editor                     |
      |  +-+----+--------------------+
      |    |    |
      |    |    |
      |    |  +-v--------------------+
      |    |  |SkShaper              |
      |    |  +-+--------+-----------+
      |    |    |        |
      |    |    |        |
    +-v----v----v--+   +-v-----------+
    |Skia          |   |HarfBuzz, ICU|
    +--------------+   +-------------+

The Application layer must interact with the:

  * Windowing system
  * File system
  * Clipboard
  * Keyboard/mouse input.

Try it out:

    tools/git-sync-deps
    bin/gn gen out/default
    ninja -C out/default editor
    out/default/editor whitespace.txt
