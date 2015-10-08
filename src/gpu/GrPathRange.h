/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRange_DEFINED
#define GrPathRange_DEFINED

#include "GrGpuResource.h"
#include "SkPath.h"
#include "SkRefCnt.h"
#include "SkTArray.h"

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

    void loadPathsIfNeeded(const void* indices, PathIndexType, int count) const;

    template<typename IndexType> void loadPathsIfNeeded(const IndexType* indices, int count) const {
        if (!fPathGenerator) {
            return;
        }

        bool didLoadPaths = false;

        for (int i = 0; i < count; ++i) {
            SkASSERT(indices[i] < static_cast<uint32_t>(fNumPaths));

            const int groupIndex = indices[i] / kPathsPerGroup;
            const int groupByte = groupIndex / 8;
            const uint8_t groupBit = 1 << (groupIndex % 8);

            const bool hasPath = SkToBool(fGeneratedPaths[groupByte] & groupBit);
            if (!hasPath) {
                // We track which paths are loaded in groups of kPathsPerGroup. To
                // mark a path as loaded we need to load the entire group.
                const int groupFirstPath = groupIndex * kPathsPerGroup;
                const int groupLastPath = SkTMin(groupFirstPath + kPathsPerGroup, fNumPaths) - 1;

                SkPath path;
                for (int pathIdx = groupFirstPath; pathIdx <= groupLastPath; ++pathIdx) {
                    fPathGenerator->generatePath(pathIdx, &path);
                    this->onInitPath(pathIdx, path);
                }

                fGeneratedPaths[groupByte] |= groupBit;
                didLoadPaths = true;
            }
        }

        if (didLoadPaths) {
            this->didChangeGpuMemorySize();
        }
    }

#ifdef SK_DEBUG
    void assertPathsLoaded(const void* indices, PathIndexType, int count) const;

    template<typename IndexType> void assertPathsLoaded(const IndexType* indices, int count) const {
        if (!fPathGenerator) {
            return;
        }

        for (int i = 0; i < count; ++i) {
            SkASSERT(indices[i] < static_cast<uint32_t>(fNumPaths));

            const int groupIndex = indices[i] / kPathsPerGroup;
            const int groupByte = groupIndex / 8;
            const uint8_t groupBit = 1 << (groupIndex % 8);

            SkASSERT(fGeneratedPaths[groupByte] & groupBit);
        }
    }

    virtual bool isEqualTo(const SkDescriptor& desc) const {
        return nullptr != fPathGenerator.get() && fPathGenerator->isEqualTo(desc);
    }
#endif
protected:
    // Initialize a path in the range before drawing. This is only called when
    // fPathGenerator is non-null. The child class need not call didChangeGpuMemorySize(),
    // GrPathRange will take care of that after the call is complete.
    virtual void onInitPath(int index, const SkPath&) const = 0;

private:
    enum {
        kPathsPerGroup = 16 // Paths get tracked in groups of 16 for lazy loading.
    };

    mutable SkAutoTUnref<PathGenerator> fPathGenerator;
    mutable SkTArray<uint8_t, true /*MEM_COPY*/> fGeneratedPaths;
    const int fNumPaths;

    typedef GrGpuResource INHERITED;
};

#endif
