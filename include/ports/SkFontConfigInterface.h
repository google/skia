/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontConfigInterface_DEFINED
#define SkFontConfigInterface_DEFINED

#include "SkRefCnt.h"
#include "SkTypeface.h"

/**
 *  \class SkFontConfigInterface
 *
 *  Provides SkFontHost clients with access to fontconfig services. They will
 *  access the global instance found in RefGlobal().
 */
class SkFontConfigInterface : public SkRefCnt {
public:
    /**
     *  Returns the global SkFontConfigInterface instance, and if it is not
     *  NULL, calls ref() on it. The caller must balance this with a call to
     *  unref().
     */
    static SkFontConfigInterface* RefGlobal();

    /**
     *  Replace the current global instance with the specified one, safely
     *  ref'ing the new instance, and unref'ing the previous. Returns its
     *  parameter (the new global instance).
     */
    static SkFontConfigInterface* SetGlobal(SkFontConfigInterface*);

    /**
     *  Given a familyName and style, find the best matching font and return
     *  its fileFaceID and actual style (if outStyle is not null) and return
     *  true. If no matching font can be found, ignore fileFaceID and outStyle
     *  and return false.
     */
    virtual bool match(const char familyName[], SkTypeface::Style requested,
                       unsigned* fileFaceID, SkTypeface::Style* outStyle) = 0;

    /**
     *  Given a fileFaceID (returned by match), return the associated familyName
     *  and return true. If the associated name cannot be returned, ignore the
     *  name parameter, and return false.
     */
    virtual bool getFamilyName(unsigned fileFaceID, SkString* name) = 0;

    /**
     *  Given a fileFaceID (returned by match), open a stream to access its
     *  data, which the caller while take ownership of (and will call unref()
     *  when they're through). On failure, return NULL.
     */
    virtual SkStream* openStream(unsigned fileFaceID) = 0;
};

#endif
