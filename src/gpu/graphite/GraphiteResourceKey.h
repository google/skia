/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GraphiteResourceKey_DEFINED
#define skgpu_graphite_GraphiteResourceKey_DEFINED

#include "src/gpu/ResourceKey.h"
#include "src/gpu/graphite/ResourceTypes.h"

namespace skgpu::graphite {

/**
 * GraphiteResourceKey is no different from the base skgpu::ResourceKey except that its type is
 * specific to Graphite.  Graphite does not use different types of keys to manage reusability or
 * resource sharing like in Ganesh. Instead a key simply encodes the underlying configuration of the
 * resource and whether or not the resource can be shared is handled externally through coordination
 * between the ResourceCache and ResourceProvider.
 */
class GraphiteResourceKey : public skgpu::ResourceKey {
public:
    /** Generate a unique ResourceType. */
    static ResourceType GenerateResourceType();

    /** Creates an invalid key. It must be initialized using a Builder object before use. */
    GraphiteResourceKey() {}

    GraphiteResourceKey(const GraphiteResourceKey& that) { *this = that; }

    ResourceType resourceType() const { return this->domain(); }

    GraphiteResourceKey& operator=(const GraphiteResourceKey& that) {
        this->ResourceKey::operator=(that);
        return *this;
    }

    bool operator==(const GraphiteResourceKey& that) const {
        return this->ResourceKey::operator==(that);
    }
    bool operator!=(const GraphiteResourceKey& that) const {
        return !(*this == that);
    }

    class Builder : public ResourceKey::Builder {
    public:
        Builder(GraphiteResourceKey* key, ResourceType type, int data32Count)
                : ResourceKey::Builder(key, type, data32Count) {}
    };
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GraphiteResourceKey_DEFINED
