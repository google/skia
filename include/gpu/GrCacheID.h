/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCacheID_DEFINED
#define GrCacheID_DEFINED

#include "GrTypes.h"

///////////////////////////////////////////////////////////////////////////////
#define GR_DECLARE_RESOURCE_CACHE_TYPE()                         \
    static int8_t GetResourceType();

#define GR_DEFINE_RESOURCE_CACHE_TYPE(ClassName)                 \
    int8_t ClassName::GetResourceType() {                        \
        static int8_t kResourceTypeID = 0;                       \
        if (0 == kResourceTypeID) {                              \
            kResourceTypeID = GrCacheID::GetNextResourceType();  \
        }                                                        \
        return kResourceTypeID;                                  \
    }


///////////////////////////////////////////////////////////////////////////////
#define GR_DECLARE_RESOURCE_CACHE_DOMAIN(AccessorName)           \
    static int8_t AccessorName();

#define GR_DEFINE_RESOURCE_CACHE_DOMAIN(ClassName, AccessorName) \
    int8_t ClassName::AccessorName() {                           \
        static int8_t kDomainID = 0;                             \
        if (0 == kDomainID) {                                    \
            kDomainID = GrCacheID::GetNextDomain();              \
        }                                                        \
        return kDomainID;                                        \
    }

/**
 * The cache ID adds structure to the IDs used for caching GPU resources. It
 * is broken into three portions:
 *      the public portion - which is filled in by Skia clients
 *      the private portion - which is used by the cache (domain & type)
 *      the resource-specific portion - which is filled in by each GrResource-
 *              derived class.
 *
 * For the public portion each client of the cache makes up its own
 * unique-per-resource identifier (e.g., bitmap genID). A public ID of
 * 'kScratch_CacheID' indicates that the resource is a "scratch" resource.
 * When used to acquire a resource it indicates the cache user is
 * looking for a resource that matches a resource-subclass-specific set of
 * “dimensions” such as width, height, buffer size, or pixel config, but not
 * for particular resource contents (e.g., texel or vertex values). The public
 * IDs are unique within a private ID value but not necessarily across
 * private IDs.
 *
 * The domain portion identifies the cache client while the type field
 * indicates the resource type. When the public portion indicates that the
 * resource is a scratch resource, the domain field should be kUnrestricted
 * so that scratch resources can be recycled across domains.
 */
class GrCacheID {
public:
    uint64_t     fPublicID;

    uint32_t     fResourceSpecific32;

    uint8_t      fDomain;
private:
    uint8_t      fResourceType;

public:
    uint16_t     fResourceSpecific16;

    GrCacheID(uint8_t resourceType)
        : fPublicID(kDefaultPublicCacheID)
        , fDomain(GrCacheData::kScratch_ResourceDomain)
        , fResourceType(resourceType) {
    }

    void toRaw(uint32_t v[4]);

    uint8_t getResourceType() const { return fResourceType; }

    /*
     * Default value for public portion of GrCacheID
     */
    static const uint64_t kDefaultPublicCacheID = 0;

    static const uint8_t kInvalid_ResourceType = 0;

    static uint8_t    GetNextDomain();
    static uint8_t    GetNextResourceType();


};

#endif // GrCacheID_DEFINED
