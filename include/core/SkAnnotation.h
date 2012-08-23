/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAnnotation_DEFINED
#define SkAnnotation_DEFINED

#include "SkFlattenable.h"

class SkData;
class SkDataSet;
class SkStream;
class SkWStream;

/**
 *  Experimental class for annotating draws. Do not use directly yet.
 *  Use helper functions at the bottom of this file for now.
 */
class SkAnnotation : public SkFlattenable {
public:
    enum Flags {
        // If set, the associated drawing primitive should not be drawn
        kNoDraw_Flag  = 1 << 0,
    };

    SkAnnotation(SkDataSet*, uint32_t flags);
    virtual ~SkAnnotation();

    uint32_t getFlags() const { return fFlags; }
    SkDataSet* getDataSet() const { return fDataSet; }

    bool isNoDraw() const { return SkToBool(fFlags & kNoDraw_Flag); }

    /**
     *  Helper for search the annotation's dataset.
     */
    SkData* find(const char name[]) const;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkAnnotation)

protected:
    SkAnnotation(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    SkDataSet*  fDataSet;
    uint32_t    fFlags;

    void writeToStream(SkWStream*) const;
    void readFromStream(SkStream*);

    typedef SkFlattenable INHERITED;
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

#endif
