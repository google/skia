/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAnnotationKeys_DEFINED
#define SkAnnotationKeys_DEFINED

#include "include/core/SkTypes.h"

class SkAnnotationKeys {
public:
    /**
     *  Returns the canonical key whose payload is a URL
     */
    static const char* URL_Key();

    /**
     *  Returns the canonical key whose payload is the name of a destination to
     *  be defined.
     */
    static const char* Define_Named_Dest_Key();

    /**
     *  Returns the canonical key whose payload is the name of a destination to
     *  be linked to.
     */
    static const char* Link_Named_Dest_Key();
};

#endif
