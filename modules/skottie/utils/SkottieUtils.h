/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieUtils_DEFINED
#define SkottieUtils_DEFINED

#include "SkColor.h"
#include "Skottie.h"
#include "SkottieProperty.h"
#include "SkString.h"
#include "SkTHash.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class SkAnimCodecPlayer;
class SkData;
class SkImage;

namespace skottie_utils {

class MultiFrameImageAsset final : public skottie::ImageAsset {
public:
    static sk_sp<MultiFrameImageAsset> Make(sk_sp<SkData>);

    bool isMultiFrame() override;

    sk_sp<SkImage> getFrame(float t) override;

private:
    explicit MultiFrameImageAsset(std::unique_ptr<SkAnimCodecPlayer>);

    std::unique_ptr<SkAnimCodecPlayer> fPlayer;

    using INHERITED = skottie::ImageAsset;
};

class FileResourceProvider final : public skottie::ResourceProvider {
public:
    static sk_sp<FileResourceProvider> Make(SkString base_dir);

    sk_sp<SkData> load(const char resource_path[], const char resource_name[]) const override;

    sk_sp<skottie::ImageAsset> loadImageAsset(const char[], const char []) const override;

private:
    explicit FileResourceProvider(SkString);

    const SkString fDir;

    using INHERITED = skottie::ResourceProvider;
};

/**
 * CustomPropertyManager implements a property management scheme where color/opacity/transform
 * attributes are grouped and manipulated by name (one-to-many mapping).
 *
 *   - setters apply the value to all properties in a named group
 *
 *   - getters return all the managed property groups, and the first value within each of them
 *     (unchecked assumption: all properties within the same group have the same value)
 *
 * Use CustomPropertyManagerBuilder to filter nodes at animation build time, and instantiate a
 * CustomPropertyManager.
 */
class CustomPropertyManager final {
public:
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

private:
    friend class CustomPropertyManagerBuilder;

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

    CustomPropertyManager(PropMap<skottie::ColorPropertyHandle>,
                          PropMap<skottie::OpacityPropertyHandle>,
                          PropMap<skottie::TransformPropertyHandle>);

    PropMap<skottie::ColorPropertyHandle>     fColorMap;
    PropMap<skottie::OpacityPropertyHandle>   fOpacityMap;
    PropMap<skottie::TransformPropertyHandle> fTransformMap;
};

/**
 * A builder for CustomPropertyManager.  Only accepts node names starting with '$'.
 */
class CustomPropertyManagerBuilder final : public skottie::PropertyObserver {
public:
    CustomPropertyManagerBuilder();
    ~CustomPropertyManagerBuilder() override;

    std::unique_ptr<CustomPropertyManager> build();

    void onColorProperty    (const char node_name[],
                             const LazyHandle<skottie::ColorPropertyHandle>&) override;
    void onOpacityProperty  (const char node_name[],
                             const LazyHandle<skottie::OpacityPropertyHandle>&) override;
    void onTransformProperty(const char node_name[],
                             const LazyHandle<skottie::TransformPropertyHandle>&) override;

private:
    std::string acceptProperty(const char* name) const {
        static constexpr char kPrefix = '$';

        return (name[0] == kPrefix && name[1] != '\0')
            ? std::string(name + 1)
            : std::string();
    }

    CustomPropertyManager::PropMap<skottie::ColorPropertyHandle>     fColorMap;
    CustomPropertyManager::PropMap<skottie::OpacityPropertyHandle>   fOpacityMap;
    CustomPropertyManager::PropMap<skottie::TransformPropertyHandle> fTransformMap;

    using INHERITED = skottie::PropertyObserver;
};

} // namespace skottie_utils

#endif // SkottieUtils_DEFINED
