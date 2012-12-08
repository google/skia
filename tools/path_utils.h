/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef path_utils_DEFINED
#define path_utils_DEFINED

class SkFILEWStream;
class SkPath;

namespace sk_tools {
    // These utilities help write paths to a .cpp file in a compileable form.
    // To use call them in the order:
    //      dump_path_prefix - once per program invocation
    //      dump_path - once for each path of interest
    //      dump_path_suffix - once per program invocation
    //
    // The output system relies on a global current path ID and assumes that
    // only one set of aggregation arrays will be written per program
    // invocation. These utilities are not thread safe.

    // Write of the headers needed to compile the resulting .cpp file
    void dump_path_prefix(SkFILEWStream* pathStream);

    // Write out a single path in the form:
    //      static const int numPts# = ...;
    //      SkPoint pts#[] = { ... };
    //      static const int numVerbs# = ...;
    //      uint8_t verbs#[] = { ... };
    // Where # is a globally unique identifier
    void dump_path(SkFILEWStream* pathStream, const SkPath& path);

    // Write out structures to aggregate info about the written paths:
    //      int numPaths = ...;
    //      int sizes[] = {
    //          numPts#, numVerbs#,
    //          ...
    //      };
    //      const SkPoint* points[] = { pts#, ... };
    //      const uint8_t* verbs[] = { verbs#, ... };
    void dump_path_suffix(SkFILEWStream* pathStream);
}

#endif
