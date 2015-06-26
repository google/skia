/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRange_DEFINED
#define GrPathRange_DEFINED

#include "GrGpuResource.h"
#include "SkRefCnt.h"
#include "SkTArray.h"

class SkPath;
class SkDescriptor;

/**
 * Represents a contiguous range of GPU path objects.
 * This object is immutable with the exception that individual paths may be
 * initialized lazily.
 */

class GrPathRange : public GrGpuResource {
public:
    

    enum PathIndexType {
        kU8_PathIndexType,   //!< uint8_t
        kU16_PathIndexType,  //!< uint16_t
        kU32_PathIndexType,  //!< uint32_t

        kLast_PathIndexType = kU32_PathIndexType
    };

    static inline int PathIndexSizeInBytes(PathIndexType type) {
        GR_STATIC_ASSERT(0 == kU8_PathIndexType);
        GR_STATIC_ASSERT(1 == kU16_PathIndexType);
        GR_STATIC_ASSERT(2 == kU32_PathIndexType);
        GR_STATIC_ASSERT(kU32_PathIndexType == kLast_PathIndexType);

        return 1 << type;
    }

    /**
     * Class that generates the paths for a specific range.
     */
    class PathGenerator : public SkRefCnt {
    public:
        virtual int getNumPaths() = 0;
        virtual void generatePath(int index, SkPath* out) = 0;
#ifdef SK_DEBUG
        virtual bool isEqualTo(const SkDescriptor&) const { return false; }
#endif
        virtual ~PathGenerator() {}
    };

    /**
     * Initialize a lazy-loaded path range. This class will generate an SkPath and call
     * onInitPath() for each path within the range before it is drawn for the first time.
     */
    GrPathRange(GrGpu*, PathGenerator*);

    /**
     * Initialize an eager-loaded path range. The subclass is responsible for ensuring all
     * the paths are initialized up front.
     */
    GrPathRange(GrGpu*, int numPaths);

    int getNumPaths() const { return fNumPaths; }
    const PathGenerator* getPathGenerator() const { return fPathGenerator.get(); }

#ifdef SK_DEBUG
    virtual bool isEqualTo(const SkDescriptor& desc) const {
        return NULL != fPathGenerator.get() && fPathGenerator->isEqualTo(desc);
    }
#endif
protected:
    // Initialize a path in the range before drawing. This is only called when
    // fPathGenerator is non-null. The child class need not call didChangeGpuMemorySize(),
    // GrPathRange will take care of that after the call is complete.
    virtual void onInitPath(int index, const SkPath&) const = 0;

private:
    // Notify when paths will be drawn in case this is a lazy-loaded path range.
    friend class GrPathRendering;
    void willDrawPaths(const void* indices, PathIndexType, int count) const;
    template<typename IndexType> void willDrawPaths(const void* indices, int count) const;

    mutable SkAutoTUnref<PathGenerator> fPathGenerator;
    mutable SkTArray<uint8_t, true /*MEM_COPY*/> fGeneratedPaths;
    const int fNumPaths;

    typedef GrGpuResource INHERITED;
};

#endif
