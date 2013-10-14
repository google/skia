/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAnnotation_DEFINED
#define SkAnnotation_DEFINED

#include "SkRefCnt.h"
#include "SkString.h"

class SkData;
class SkFlattenableReadBuffer;
class SkFlattenableWriteBuffer;
class SkStream;
class SkWStream;
struct SkPoint;

/**
 *  Experimental class for annotating draws. Do not use directly yet.
 *  Use helper functions at the bottom of this file for now.
 */
class SkAnnotation : public SkRefCnt {
public:
    enum Flags {
        // If set, the associated drawing primitive should not be drawn
        kNoDraw_Flag  = 1 << 0,
    };

    SkAnnotation(const char key[], SkData* value, uint32_t flags);
    virtual ~SkAnnotation();

    uint32_t getFlags() const { return fFlags; }

    bool isNoDraw() const { return SkToBool(fFlags & kNoDraw_Flag); }

    /**
     *  Return the data for the specified key, or NULL.
     */
    SkData* find(const char key[]) const;

    SkAnnotation(SkFlattenableReadBuffer&);
    void writeToBuffer(SkFlattenableWriteBuffer&) const;

private:
    SkString    fKey;
    SkData*     fData;
    uint32_t    fFlags;

    typedef SkRefCnt INHERITED;
};

/**
 *  Experimental collection of predefined Keys into the Annotation dictionary
 */
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

///////////////////////////////////////////////////////////////////////////////
//
// Experimental helper functions to use Annotations
//

struct SkRect;
class SkCanvas;

/**
 *  Experimental!
 *
 *  Annotate the canvas by associating the specified URL with the
 *  specified rectangle (in local coordinates, just like drawRect). If the
 *  backend of this canvas does not support annotations, this call is
 *  safely ignored.
 *
 *  The caller is responsible for managing its ownership of the SkData.
 */
SK_API void SkAnnotateRectWithURL(SkCanvas*, const SkRect&, SkData*);

/**
 *  Experimental!
 *
 *  Annotate the canvas by associating a name with the specified point.
 *
 *  If the backend of this canvas does not support annotations, this call is
 *  safely ignored.
 *
 *  The caller is responsible for managing its ownership of the SkData.
 */
SK_API void SkAnnotateNamedDestination(SkCanvas*, const SkPoint&, SkData*);

/**
 *  Experimental!
 *
 *  Annotate the canvas by making the specified rectangle link to a named
 *  destination.
 *
 *  If the backend of this canvas does not support annotations, this call is
 *  safely ignored.
 *
 *  The caller is responsible for managing its ownership of the SkData.
 */
SK_API void SkAnnotateLinkToDestination(SkCanvas*, const SkRect&, SkData*);


#endif
