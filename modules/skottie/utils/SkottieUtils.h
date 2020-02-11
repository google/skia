/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieUtils_DEFINED
#define SkottieUtils_DEFINED

#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace skottie_utils {

/**
 * CustomPropertyManager implements a property management scheme where color/opacity/transform
 * attributes are grouped and manipulated by name (one-to-many mapping).
 *
 *   - setters apply the value to all properties in a named group
 *
 *   - getters return all the managed property groups, and the first value within each of them
 *     (unchecked assumption: all properties within the same group have the same value)
 *
 * Attach to an Animation::Builder using the utility methods below to intercept properties and
 * markers at build time.
 */
class CustomPropertyManager final {
public:
    CustomPropertyManager();
    ~CustomPropertyManager();

    using PropKey = std::string;

    std::vector<PropKey> getColorProps() const;
    skottie::ColorPropertyValue getColor(const PropKey&) const;
    bool setColor(const PropKey&, const skottie::ColorPropertyValue&);

    std::vector<PropKey> getOpacityProps() const;
    skottie::OpacityPropertyValue getOpacity(const PropKey&) const;
    bool setOpacity(const PropKey&, const skottie::OpacityPropertyValue&);

    std::vector<PropKey> getTransformProps() const;
    skottie::TransformPropertyValue getTransform(const PropKey&) const;
    bool setTransform(const PropKey&, const skottie::TransformPropertyValue&);

    std::vector<PropKey> getTextProps() const;
    skottie::TextPropertyValue getText(const PropKey&) const;
    bool setText(const PropKey&, const skottie::TextPropertyValue&);

    struct MarkerInfo {
        std::string name;
        float       t0, t1;
    };
    const std::vector<MarkerInfo>& markers() const { return fMarkers; }

    // Returns a property observer to be attached to an animation builder.
    sk_sp<skottie::PropertyObserver> getPropertyObserver() const;

    // Returns a marker observer to be attached to an animation builder.
    sk_sp<skottie::MarkerObserver> getMarkerObserver() const;

private:
    class PropertyInterceptor;
    class MarkerInterceptor;

    static std::string acceptKey(const char* name) {
        static constexpr char kPrefix = '$';

        return (name[0] == kPrefix && name[1] != '\0')
            ? std::string(name + 1)
            : std::string();
    }

    sk_sp<PropertyInterceptor> fPropertyInterceptor;
    sk_sp<MarkerInterceptor>   fMarkerInterceptor;

    template <typename T>
    using PropGroup = std::vector<std::unique_ptr<T>>;

    template <typename T>
    using PropMap = std::unordered_map<PropKey, PropGroup<T>>;

    template <typename T>
    std::vector<PropKey> getProps(const PropMap<T>& container) const;

    template <typename V, typename T>
    V get(const PropKey&, const PropMap<T>& container) const;

    template <typename V, typename T>
    bool set(const PropKey&, const V&, const PropMap<T>& container);

    PropMap<skottie::ColorPropertyHandle>     fColorMap;
    PropMap<skottie::OpacityPropertyHandle>   fOpacityMap;
    PropMap<skottie::TransformPropertyHandle> fTransformMap;
    PropMap<skottie::TextPropertyHandle>      fTextMap;
    std::vector<MarkerInfo>                   fMarkers;
    std::string                               fCurrentNode;
};

} // namespace skottie_utils

#endif // SkottieUtils_DEFINED
