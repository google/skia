/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHdrMetadata_DEFINED
#define SkHdrMetadata_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/private/base/SkAPI.h"

#include <memory>
#include <optional>
#include <vector>

class SkColorFilter;
class SkData;
class SkString;

namespace skhdr {

/**
 * Content light level metadata.
 * The semantics of this metadata is defined in:
 *   ANSI/CTA-861-H A DTV Profile for Uncompressed High Speed Digital Interfaces
 *   Annex P Calculation of MaxCLL and MaxFALL
 * Slightly different semantics for this metadata are defined in:
 *   Portable Network Graphics (PNG) Specification (Third Edition)
 *   11.3.2.8 cLLI Content Light Level Information
 *   https://www.w3.org/TR/png-3/#cLLI-chunk
 * This metadata should only be used in ways that work with both semantics.
 */
struct SK_API ContentLightLevelInformation {
    float fMaxCLL = 0.f;
    float fMaxFALL = 0.f;

    /**
     * Decode from the binary encoding listed at:
     *   AV1 Bitstream & Decoding Process Specification Version 1.0.0 Errata 1
     *   https://aomediacodec.github.io/av1-spec/av1-spec.pdf
     *   5.8.3 Metadata high dynamic range content light level syntax
     * This encoding is equivalent to:
     *   ITU-T H.265 (V10) (07/2024)
     *   D.2.35 Content light level information SEI message syntax
     * Return false if parsing fails.
     */
    bool parse(const SkData* data);

    /**
     * Serialize to the encoding used by parse().
     */
    sk_sp<SkData> serialize() const;

    /**
     * Decode from the binary encoding listed at:
     *   Portable Network Graphics (PNG) Specification (Third Edition)
     *   11.3.2.8 cLLI Content Light Level Information
     *   https://www.w3.org/TR/png-3/#cLLI-chunk
     * This encoding is not equivalent to the encoding used by parse().
     * Return false if parsing fails.
     */
    bool parsePngChunk(const SkData* data);

    /**
     * Serialize to the encoding used by parsePngChunk().
     */
    sk_sp<SkData> serializePngChunk() const;

    /**
     * Return a human-readable description.
     */
    SkString toString() const;

    bool operator==(const ContentLightLevelInformation& other) const;
    bool operator!=(const ContentLightLevelInformation& other) const {
        return !(*this == other);
    }
};

/**
 * Mastering display color volume metadata.
 * The semantics of this metadata is defined in:
 * SMPTE ST 2086:2018 Mastering Display Color Volume Metadata Supporting
 * High Luminance and Wide Color Gamut Images
 */
struct SK_API MasteringDisplayColorVolume {
    SkColorSpacePrimaries fDisplayPrimaries = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
    float fMaximumDisplayMasteringLuminance = 0.f;
    float fMinimumDisplayMasteringLuminance = 0.f;

    /**
     * The encoding as defined in:
     *   AV1 Bitstream & Decoding Process Specification Version 1.0.0 Errata 1
     *   https://aomediacodec.github.io/av1-spec/av1-spec.pdf
     *   5.8.4 Metadata high dynamic range mastering display color volume syntax
     * This encoding is equivalent to:
     *   ITU-T H.265 (V10) (07/2024)
     *   D.2.35 Content light level information SEI message syntax
     * This encoding is also equivalent to:
     *   Portable Network Graphics (PNG) Specification (Third Edition)
     *   11.3.2.7 mDCV Mastering Display Color Volume
     *   https://www.w3.org/TR/png-3/#mDCV-chunk
     * Return false if parsing fails.
     */
    bool parse(const SkData* data);

    /**
     * Serialize to the encoding used by parse().
     */
    sk_sp<SkData> serialize() const;

    /**
     * Return a human-readable description.
     */
    SkString toString() const;

    bool operator==(const MasteringDisplayColorVolume& other) const;
    bool operator!=(const MasteringDisplayColorVolume& other) const {
        return !(*this == other);
    }
};

/**
 * Adaptive Global Tone Map
 * This structure contains the metadata items from the ColorVolumeTransform metadata group
 * in Clause 7.1: Metadata set of SMPTE ST 2094-50: Dynamic metadata for color volume transform
 * Application #5
 * https://github.com/SMPTE/st2094-50
 * WARNING: This specification is still a DRAFT and is subject to change before publication.
 */
struct SK_API AdaptiveGlobalToneMap {
    // A GainCurve metadata group.
    struct SK_API GainCurve {
        // Structure holding one entry of the GainCurveControlPointX, GainCurveControlPointY, and
        // GainCurveControlPointM metadata items.
        struct ControlPoint {
            float fX = 0.f;
            float fY = 0.f;
            float fM = 0.f;

            bool operator==(const ControlPoint& other) const {
                return fX == other.fX && fY == other.fY && fM == other.fM;
            }
        };

        // The size of this vector is the value of the GainCurveNumControlPoints metadata item.
        static constexpr size_t kMinNumControlPoints = 1u;
        static constexpr size_t kMaxNumControlPoints = 32u;
        std::vector<ControlPoint> fControlPoints = {};

        bool operator==(const GainCurve& other) const {
            return fControlPoints == other.fControlPoints;
        }
    };

    // A ComponentMix metadata group.
    struct SK_API ComponentMixingFunction {
        // The ComponentMixRed/Green/Blue/Max/Min/Component metadata items.
        float fRed = 0.f;
        float fGreen = 0.f;
        float fBlue = 0.f;
        float fMax = 0.f;
        float fMin = 0.f;
        float fComponent = 0.f;

        bool operator==(const ComponentMixingFunction& other) const {
            return fRed == other.fRed && fGreen == other.fGreen && fBlue == other.fBlue &&
                   fMax == other.fMax && fMin == other.fMin && fComponent == other.fComponent;
        }
    };

    // A ColorGainFunction metadata group.
    struct SK_API ColorGainFunction {
        // The ComponentMix metadata group.
        ComponentMixingFunction fComponentMixing = {};

        // The GainCurve metadata group.
        GainCurve fGainCurve = {};

        bool operator==(const ColorGainFunction& other) const {
            return fComponentMixing == other.fComponentMixing && fGainCurve == other.fGainCurve;
        }
    };

    // Structure holding the metadata items and groups for an alternate image.
    struct SK_API AlternateImage {
        // The AlternateHdrHeadroom metadata item.
        float fHdrHeadroom = 0.f;

        // The ColorGainFunction metadata group.
        ColorGainFunction fColorGainFunction = {};

        bool operator==(const AlternateImage& other) const {
            return fHdrHeadroom == other.fHdrHeadroom && fColorGainFunction == other.fColorGainFunction;
        }
    };

    // HeadroomAdaptiveToneMap metadata group.
    struct SK_API HeadroomAdaptiveToneMap {
        // The BaselineHdrHeadroom metadata item.
        float fBaselineHdrHeadroom = 0.f;

        // The GainApplicationSpaceColorPrimaries metadata item.
        SkColorSpacePrimaries fGainApplicationSpacePrimaries =
            {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

        // The size of this vector is the NumAlternateImages metadata item.
        static constexpr size_t kMaxNumAlternateImages = 4u;
        std::vector<AlternateImage> fAlternateImages = {};

        bool operator==(const HeadroomAdaptiveToneMap& other) const {
            return fBaselineHdrHeadroom == other.fBaselineHdrHeadroom &&
                   fGainApplicationSpacePrimaries == other.fGainApplicationSpacePrimaries &&
                   fAlternateImages == other.fAlternateImages;
        }
    };

    // The default value for the HdrReferenceWhite metadata item.
    static constexpr float kDefaultHdrReferenceWhite = 203.f;

    // The HdrReferenceWhite metadata item.
    float fHdrReferenceWhite = kDefaultHdrReferenceWhite;

    // The HeadroomAdaptiveToneMap metadata group.
    std::optional<HeadroomAdaptiveToneMap> fHeadroomAdaptiveToneMap;

    /**
     * Decode from the binary encoding in Annex C.
     */
    bool parse(const SkData* data);

    /**
     * Serialize to the encoding used by parse().
     */
    sk_sp<SkData> serialize() const;

    /**
     * Return a human-readable description.
     */
    SkString toString() const;

    bool operator==(const AdaptiveGlobalToneMap& other) const {
        return fHdrReferenceWhite == other.fHdrReferenceWhite &&
               fHeadroomAdaptiveToneMap == other.fHeadroomAdaptiveToneMap;
    }
};

/**
 * TODO(https://crbug.com/468928417): This structure was originally designed to be the interface
 * for parsing SMPTE ST 2094-50 metadata. It is no longer being used in this way, and should be
 * removed or recycled.
 */
class SK_API Agtm {
  public:
    /**
     * Parse the specified SkData. Returns nullptr if the data fails to parse.
     */
    static std::unique_ptr<Agtm> Make(const SkData* data);

    /**
     * Generate reference white tone mapping metadata for the specified baseline HDR headroom and
     * HDR reference white values.
     */
    static std::unique_ptr<Agtm> MakeReferenceWhite(float hdrReferenceWhite,
                                                    float baselineHdrHeadroom);

    /**
     * Generate metadata with a HDR reference white set to `hdrReferenceWhite`, that specifies that
     * no tone mapping is to be done (that is, just clamping is to be performed), and that the
     * content has HDR headroom specified by `baselineHdrHeadroom`.
     */
    static std::unique_ptr<Agtm> MakeClamp(float hdrReferenceWhite, float baselineHdrHeadroom);

    Agtm() = default;
    Agtm(const Agtm&) = delete;
    Agtm& operator=(const Agtm&) = delete;
    Agtm(Agtm&&) = delete;
    Agtm& operator=(Agtm&&) = delete;
    virtual ~Agtm() = default;

    /**
     * Serialize the data to the format parsed by Make.
     */
    virtual sk_sp<SkData> serialize() const = 0;

    /**
     * The default value for the HdrReferenceWhite metadata item.
     */
    static constexpr float kDefaultHdrReferenceWhite = 203.f;

    /**
     * Return the HdrReferenceWhite metadata item value.
     */
    virtual float getHdrReferenceWhite() const = 0;

    /**
     * Functions to query if the BaselineHdrHeadroom metadata item was specified and retrieve it
     * (which will assert if was not specified).
     */
    virtual bool hasBaselineHdrHeadroom() const = 0;
    virtual float getBaselineHdrHeadroom() const = 0;

    /**
     * Return true if this metadata specifies not to do any tone mapping (it is the type that
     * was created using MakeClamp).
     */
    virtual bool isClamp() const = 0;

    /**
     * Return the SkColorFilter to tone map to the specified targeted HDR headroom.
     */
    virtual sk_sp<SkColorFilter> makeColorFilter(float targetedHdrHeadroom) const = 0;

    /**
     * Return a human-readable description.
     */
    virtual SkString toString() const = 0;
};

/**
 * Structure containing all HDR metadata that can be attached to an image or video frame.
 */
class SK_API Metadata {
  public:
    /**
     * Return a container with no metadata.
     */
    static Metadata MakeEmpty();

    /**
     * If there does not exists Content Light Level Information metadata, then return false.
     * Otherwise return true and if `clli` is non-nullptr then write the metadata to `clli`.
     */
    bool getContentLightLevelInformation(ContentLightLevelInformation* clli) const;

    /**
     * Set the Content Light Level Information metadata.
     */
    void setContentLightLevelInformation(const ContentLightLevelInformation& clli);

    /**
     * If there does not exists Mastering Display Color Volume metadata, then return false.
     * Otherwise return true and if `mdcv` is non-nullptr then write the metadata to `mdcv`.
     */
    bool getMasteringDisplayColorVolume(MasteringDisplayColorVolume* mdcv) const;

    /**
     * Set the Mastering Display Color Volume metadata.
     */
    void setMasteringDisplayColorVolume(const MasteringDisplayColorVolume& mdcv);

    /**
     * If there does not exists Adaptive Global Tone Map metadata, then return false.
     * Otherwise return true and if `agtm` is non-nullptr then write the metadata to `agtm`.
     */
    bool getAdaptiveGlobalToneMap(AdaptiveGlobalToneMap* agtm) const;

    /**
     * Set the Adaptive Global Tone Map metadata.
     */
    void setAdaptiveGlobalToneMap(const AdaptiveGlobalToneMap& agtm);

    /**
     * Return the serialized Adaptive Global Tone Mapping metadata, or nullptr if none has been set.
     */
    sk_sp<const SkData> getSerializedAgtm() const;

    /**
     * Set the serialized Adaptive Global Tone Mapping metadata.
     */
    void setSerializedAgtm(sk_sp<const SkData>);

    /**
     * Return a human-readable description.
     */
    SkString toString() const;

    /**
     * Return the SkColorFilter to tone map to the specified targeted HDR headroom.
     *
     * The `inputColorSpace` parameter is the color space of the input image that will be
     * used with the image. If it is PQ or HLG, then the color filter will effectively
     * reinterpret the image as having the HDR reference white parameter indicated in the
     * metadata.
     *
     * If `inputColorSpace` is PQ or HLG, then a default tone mapping will be provided,
     * inferring the baseline HDR headroom from the CLLI or MDCV metadata, if present.
     */
    sk_sp<SkColorFilter> makeToneMapColorFilter(
        float targetedHdrHeadroom, const SkColorSpace* inputColorSpace = nullptr) const;

    bool operator==(const Metadata& other) const;
    bool operator!=(const Metadata& other) const {
      return !(*this == other);
    }

  private:
    std::optional<ContentLightLevelInformation> fContentLightLevelInformation;
    std::optional<MasteringDisplayColorVolume> fMasteringDisplayColorVolume;
    sk_sp<const SkData> fAgtm;
};

}  // namespace skhdr

#endif
