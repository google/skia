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

class GraphiteResourceKey : public skgpu::ResourceKey {
public:
    /** Generate a unique ResourceType. */
    static ResourceType GenerateResourceType();

    /** Creates an invalid key. It must be initialized using a Builder object before use. */
    GraphiteResourceKey() : fShareable(Shareable::kNo) {}

    GraphiteResourceKey(const GraphiteResourceKey& that) { *this = that; }

    /** reset() returns the key to the invalid state. */
    using ResourceKey::reset;

    using ResourceKey::isValid;

    ResourceType resourceType() const { return this->domain(); }

    Shareable shareable() const { return fShareable; }

    GraphiteResourceKey& operator=(const GraphiteResourceKey& that) {
        this->ResourceKey::operator=(that);
        fShareable = that.fShareable;
        return *this;
    }

    bool operator==(const GraphiteResourceKey& that) const {
        bool result = this->ResourceKey::operator==(that);
        SkASSERT(!result || (result == (fShareable == that.fShareable)));
        return result;
    }
    bool operator!=(const GraphiteResourceKey& that) const {
        return !(*this == that);
    }

    class Builder : public ResourceKey::Builder {
    public:
        Builder(GraphiteResourceKey* key, ResourceType type, int data32Count, Shareable shareable)
                : ResourceKey::Builder(key, type, data32Count) {
            key->fShareable = shareable;
        }
    };

private:
    Shareable fShareable;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GraphiteResourceKey_DEFINED
