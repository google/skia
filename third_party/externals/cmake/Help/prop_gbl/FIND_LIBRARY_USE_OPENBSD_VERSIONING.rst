FIND_LIBRARY_USE_OPENBSD_VERSIONING
-----------------------------------

Whether FIND_LIBRARY should find OpenBSD-style shared libraries.

This property is a boolean specifying whether the FIND_LIBRARY command
should find shared libraries with OpenBSD-style versioned extension:
".so.<major>.<minor>".  The property is set to true on OpenBSD and
false on other platforms.
