hooks = [
  {
    # This is a hack to download the code in third_party/externals in gclient
    # (it works fine without this hack when you use svn instead of gclient).
    #
    # Because gclient runs svn update with the --ignore_externals flag set,
    # it will not pick up our external dependencies in third_party/externals.
    # So run "svn update" again for these directories.
    #
    # See https://groups.google.com/a/chromium.org/group/chromium-dev/browse_thread/thread/1f99541c2c5f6c6e
    "pattern": ".*",
    "action": ["svn", "update", "trunk/third_party/externals"],
  },
]
