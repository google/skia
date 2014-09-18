/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRange_DEFINED
#define GrPathRange_DEFINED

#include "GrGpuResource.h"
#include "GrResourceCache.h"
#include "SkRefCnt.h"
#include "SkStrokeRec.h"
#include "SkTArray.h"

class SkPath;
class SkDescriptor;

/**
 * Represents a contiguous range of GPU path objects, all with a common stroke.
 * This object is immutable with the exception that individual paths may be
 * initialized lazily.
 */

class GrPathRange : public GrGpuResource {
public:
    SK_DECLARE_INST_COUNT(GrPathRange);

    static const bool kIsWrapped = false;

    /**
     * Return the resourceType intended for cache lookups involving GrPathRange.
     */
    static GrResourceKey::ResourceType resourceType() {
        static const GrResourceKey::ResourceType type = GrResourceKey::GenerateResourceType();
        return type;
    }

    /**
     *  Class that generates the paths for a specific range.
     */
    class PathGenerator : public SkRefCnt {
    public:
        virtual int getNumPaths() = 0;
        virtual void generatePath(int index, SkPath* out) = 0;
        virtual bool isEqualTo(const SkDescriptor&) const { return false; }
        virtual ~PathGenerator() {}
    };

    /**
     * Initialize a lazy-loaded path range. This class will generate an SkPath and call
     * onInitPath() for each path within the range before it is drawn for the first time.
     */
    GrPathRange(GrGpu*, PathGenerator*, const SkStrokeRec& stroke);

    /**
     * Initialize an eager-loaded path range. The subclass is responsible for ensuring all
     * the paths are initialized up front.
     */
    GrPathRange(GrGpu*, int numPaths, const SkStrokeRec& stroke);

    virtual bool isEqualTo(const SkDescriptor& desc) const {
        return NULL != fPathGenerator.get() && fPathGenerator->isEqualTo(desc);
    }

    int getNumPaths() const { return fNumPaths; }
    const SkStrokeRec& getStroke() const { return fStroke; }
    const PathGenerator* getPathGenerator() const { return fPathGenerator.get(); }

protected:
    // Initialize a path in the range before drawing. This is only called when
    // fPathGenerator is non-null. The child class need not call didChangeGpuMemorySize(),
    // GrPathRange will take care of that after the call is complete.
    virtual void onInitPath(int index, const SkPath&) const = 0;

private:
    // Notify when paths will be drawn in case this is a lazy-loaded path range.
    friend class GrGpu;
    void willDrawPaths(const uint32_t indices[], int count) const;

    mutable SkAutoTUnref<PathGenerator> fPathGenerator;
    mutable SkTArray<uint8_t, true /*MEM_COPY*/> fGeneratedPaths;
    const int fNumPaths;
    const SkStrokeRec fStroke;

    typedef GrGpuResource INHERITED;
};

#endif
