/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_PatchWriter_DEFINED
#define skgpu_tessellate_PatchWriter_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkUtils.h"
#include "src/base/SkVx.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/tessellate/LinearTolerances.h"
#include "src/gpu/tessellate/MiddleOutPolygonTriangulator.h"
#include "src/gpu/tessellate/Tessellation.h"
#include "src/gpu/tessellate/WangsFormula.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <math.h>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace skgpu::tess {

/**
 * PatchWriter writes out tessellation patches, formatted with their specific attribs, to a GPU
 * buffer.
 *
 * PatchWriter is a template class that takes traits to configure both its compile-time and runtime
 * behavior for the different tessellation rendering algorithms and GPU backends. The complexity of
 * this system is worthwhile because the attribute writing operations and math already require
 * heavy inlining for performance, and the algorithmic variations tend to only differ slightly, but
 * do so in the inner most loops. Additionally, Graphite and Ganesh use the same fundamental
 * algorithms, but Graphite's architecture and higher required hardware level mean that its
 * attribute configurations can be determined entirely at compile time.
 *
 * Traits are specified in PatchWriter's single var-args template pack. Traits come in two main
 * categories: PatchAttribs configuration and feature/processing configuration. A given PatchAttrib
 * can be always enabled, enabled at runtime, or always disabled. A feature can be either enabled
 * or disabled and are coupled more closely with the control points of the curve. Across the two
 * GPU backends and different path rendering strategies, a "patch" has the following structure:
 *
 *   - 4 control points (8 floats total) defining the curve's geometry
 *      - quadratic curves are converted to equivalent cubics on the CPU during writing
 *      - conic curves store {w, inf} in their last control point
 *      - triangles store {inf, inf} in their last control point
 *      - everything else is presumed to be a cubic defined by all 4 control points
 *   - Enabled PatchAttrib values, constant for the entire instance
 *      - layout is identical to PatchAttrib's definition, skipping disabled attribs
 *      - attribs can be enabled/disabled at runtime by building a mask of attrib values
 *
 * Currently PatchWriter supports the following traits:
 *   - Required<PatchAttrib>
 *   - Optional<PatchAttrib>
 *   - TrackJoinControlPoints
 *   - AddTrianglesWhenChopping
 *   - DiscardFlatCurves
 *
 * In addition to variable traits, PatchWriter's first template argument defines the type used for
 * allocating the GPU instance data. The templated "PatchAllocator" can be any type that provides:
 *    // A GPU-backed vertex writer for a single instance worth of data. The provided
 *    // LinearTolerances value represents the tolerances for the curve that will be written to the
 *    // returned vertex space.
 *    skgpu::VertexWriter append(const LinearTolerances&);
 *
 * Additionally, it must have a constructor that takes the stride as its first argument.
 * PatchWriter forwards any additional constructor args from its ctor to the allocator after
 * computing the necessary stride for its PatchAttribs configuration.
 */

// *** TRAITS ***

// Marks a PatchAttrib is enabled at compile time, i.e. it must always be set and will always be
// written to each patch's instance data. If present, will assert if the runtime attribs do not fit.
template <PatchAttribs A> struct Required {};
// Marks a PatchAttrib as supported, i.e. it can be enabled or disabled at runtime. Optional<A> is
// overridden by Required<A>. If neither Required<A> nor Optional<A> are in a PatchWriter's trait
// list, then the attrib is disabled at compile time and it will assert if the runtime attribs
// attempt to enable it.
template <PatchAttribs A> struct Optional {};

// Enables tracking of the kJoinControlPointAttrib based on control points of the previously
// written patch (automatically taking into account curve chopping). When a patch is first written
// (and there is no prior patch to define the join control point), the PatchWriter automatically
// records the patch to a temporary buffer--sans join--until writeDeferredStrokePatch() is called,
// filling in the now-defined join control point.
//
// This feature must be paired with Required<PatchAttribs::kJoinControlPoint>
struct TrackJoinControlPoints {};

// Write additional triangular patches to fill the resulting empty area when a curve is chopped.
// Normally, the patch geometry covers the curve defined by its control points, up to the implicitly
// closing edge between its first and last control points. When a curve is chopped to fit within
// the maximum segment count, the resulting space between the original closing edge and new closing
// edges is not filled, unless some mechanism of the shader makes it so (e.g. a fan point or
// stroking).
//
// This feature enables automatically writing triangular patches to fill this empty space when a
// curve is chopped.
struct AddTrianglesWhenChopping {};

// If a curve requires at most 1 segment to render accurately, it's effectively a straight line.
// This feature turns on automatically ignoring those curves, with the assumption that some other
// render pass will produce equivalent geometry (e.g. middle-out or inner triangulations).
struct DiscardFlatCurves {};

// Upload lines as a cubic with {a, a, b, b} for control points, instead of the truly linear cubic
// of {a, 2/3a + 1/3b, 1/3a + 2/3b, b}. Wang's formula will not return an tight lower bound on the
// number of segments in this case, but it's convenient to detect in the vertex shader and assume
// only a single segment is required. This bypasses numerical stability issues in Wang's formula
// when evaluated on the ideal linear cubic for very large control point coordinates. Other curve
// types with large coordinates do not need this treatment since they would be pre-chopped and
// culled to lines.
struct ReplicateLineEndPoints {};

// *** PatchWriter internals ***

// AttribValue exposes a consistent store and write interface for a PatchAttrib's value while
// abstracting over compile-time enabled, conditionally-enabled, or compile-time disabled attribs.
template <PatchAttribs A, typename T, bool Required, bool Optional>
struct AttribValue {
    using DataType = std::conditional_t<Required, T,
                     std::conditional_t<Optional, std::pair<T, bool>,
                                       /* else */ std::monostate>>;

    static constexpr bool kEnabled = Required || Optional;

    explicit AttribValue(PatchAttribs attribs) : AttribValue(attribs, {}) {}
    AttribValue(PatchAttribs attribs, const T& t) {
        (void) attribs; // may be unused on release builds
        if constexpr (Required) {
            SkASSERT(attribs & A);
        } else if constexpr (Optional) {
            std::get<1>(fV) = attribs & A;
        } else {
            SkASSERT(!(attribs & A));
        }
        *this = t;
    }

    AttribValue& operator=(const T& v) {
        if constexpr (Required) {
            fV = v;
        } else if constexpr (Optional) {
            // for simplicity, store even if disabled and won't be written out to VertexWriter
            std::get<0>(fV) = v;
        } // else ignore for disabled values
        return *this;
    }

    DataType fV;
};

template <PatchAttribs A, typename T, bool Required, bool Optional>
VertexWriter& operator<<(VertexWriter& w, const AttribValue<A, T, Required, Optional>& v) {
    if constexpr (Required) {
        w << v.fV; // always write
    } else if constexpr (Optional) {
        if (std::get<1>(v.fV)) {
            w << std::get<0>(v.fV); // write if enabled
        }
    } // else never write
    return w;
}

// Stores state and deferred patch data when TrackJoinControlPoints is used for a PatchWriter.
template <size_t Stride>
struct PatchStorage {
    float fN_p4    = -1.f; // The parametric segment value to restore on LinearTolerances
    bool  fMustDefer = true;  // True means next patch must be deferred

    // Holds an entire patch, except with an undefined join control point.
    char fData[Stride];

    bool hasPending() const {
        return fN_p4 >= 0.f;
    }
    void reset() {
        fN_p4 = -1.f;
        fMustDefer = true;
    }
};

// An empty object that has the same constructor signature as MiddleOutPolygonTriangulator, used
// as a stand-in when AddTrianglesWhenChopping is not a defined trait.
struct NullTriangulator {
    NullTriangulator(int, SkPoint) {}
};

#define AI SK_ALWAYS_INLINE
#define ENABLE_IF(cond) template <typename Void=void> std::enable_if_t<cond, Void>

// *** PatchWriter ***
template <typename PatchAllocator, typename... Traits>
class PatchWriter {
    // Helpers to extract specifics from the template traits pack.
    template <typename F>     struct has_trait  : std::disjunction<std::is_same<F, Traits>...> {};
    template <PatchAttribs A> using  req_attrib = has_trait<Required<A>>;
    template <PatchAttribs A> using  opt_attrib = has_trait<Optional<A>>;

    // Enabled features and attribute configuration
    static constexpr bool kTrackJoinControlPoints   = has_trait<TrackJoinControlPoints>::value;
    static constexpr bool kAddTrianglesWhenChopping = has_trait<AddTrianglesWhenChopping>::value;
    static constexpr bool kDiscardFlatCurves        = has_trait<DiscardFlatCurves>::value;
    static constexpr bool kReplicateLineEndPoints   = has_trait<ReplicateLineEndPoints>::value;

    // NOTE: MSVC 19.24 cannot compile constexpr fold expressions referenced in templates, so
    // extract everything into constexpr bool's instead of using `req_attrib` directly, etc. :(
    template <PatchAttribs A, typename T, bool Req/*=req_attrib<A>*/, bool Opt/*=opt_attrib<A>*/>
    using attrib_t = AttribValue<A, T, Req, Opt>;

    // TODO: Remove when MSVC compiler is fixed, in favor of `using Name = attrib_t<>` directly.
#define DEF_ATTRIB_TYPE(name, A, T) \
    static constexpr bool kRequire##name = req_attrib<A>::value; \
    static constexpr bool kOptional##name = opt_attrib<A>::value; \
    using name = attrib_t<A, T, kRequire##name, kOptional##name>

    DEF_ATTRIB_TYPE(JoinAttrib,      PatchAttribs::kJoinControlPoint,  SkPoint);
    DEF_ATTRIB_TYPE(FanPointAttrib,  PatchAttribs::kFanPoint,          SkPoint);
    DEF_ATTRIB_TYPE(StrokeAttrib,    PatchAttribs::kStrokeParams,      StrokeParams);

    // kWideColorIfEnabled does not define an attribute, but changes the type of the kColor attrib.
    static constexpr bool kRequireWideColor  = req_attrib<PatchAttribs::kWideColorIfEnabled>::value;
    static constexpr bool kOptionalWideColor = opt_attrib<PatchAttribs::kWideColorIfEnabled>::value;
    using Color = std::conditional_t<kRequireWideColor,  SkPMColor4f,
                  std::conditional_t<kOptionalWideColor, VertexColor,
                                              /* else */ uint32_t>>;

    DEF_ATTRIB_TYPE(ColorAttrib,     PatchAttribs::kColor,             Color);
    DEF_ATTRIB_TYPE(DepthAttrib,     PatchAttribs::kPaintDepth,        float);
    DEF_ATTRIB_TYPE(CurveTypeAttrib, PatchAttribs::kExplicitCurveType, float);
    DEF_ATTRIB_TYPE(SsboIndexAttrib, PatchAttribs::kSsboIndex,         skvx::ushort2);
#undef DEF_ATTRIB_TYPE

    static constexpr size_t kMaxStride = 4 * sizeof(SkPoint) + // control points
            (JoinAttrib::kEnabled      ? sizeof(SkPoint)                              : 0) +
            (FanPointAttrib::kEnabled  ? sizeof(SkPoint)                              : 0) +
            (StrokeAttrib::kEnabled    ? sizeof(StrokeParams)                         : 0) +
            (ColorAttrib::kEnabled     ? std::min(sizeof(Color), sizeof(SkPMColor4f)) : 0) +
            (DepthAttrib::kEnabled     ? sizeof(float)                                : 0) +
            (CurveTypeAttrib::kEnabled ? sizeof(float)                                : 0) +
            (SsboIndexAttrib::kEnabled ? sizeof(int)                                  : 0);

    // Types that vary depending on the activated features, but do not define the patch data.
    using DeferredPatch = std::conditional_t<kTrackJoinControlPoints,
            PatchStorage<kMaxStride>, std::monostate>;
    using InnerTriangulator = std::conditional_t<kAddTrianglesWhenChopping,
            MiddleOutPolygonTriangulator, NullTriangulator>;

    using float2 = skvx::float2;
    using float4 = skvx::float4;

    static_assert(!kTrackJoinControlPoints || req_attrib<PatchAttribs::kJoinControlPoint>::value,
                  "Deferred patches and auto-updating joins requires kJoinControlPoint attrib");
public:
    template <typename... Args> // forwarded to PatchAllocator
    PatchWriter(PatchAttribs attribs,
                Args&&... allocArgs)
            : fAttribs(attribs)
            , fPatchAllocator(PatchStride(attribs), std::forward<Args>(allocArgs)...)
            , fJoin(attribs)
            , fFanPoint(attribs)
            , fStrokeParams(attribs)
            , fColor(attribs)
            , fDepth(attribs)
            , fSsboIndex(attribs) {
        // Explicit curve types are provided on the writePatch signature, and not a field of
        // PatchWriter, so initialize one in the ctor to validate the provided runtime attribs.
        SkDEBUGCODE((void) CurveTypeAttrib(attribs);)
        // Validate the kWideColorIfEnabled attribute variant flag as well
        if constexpr (req_attrib<PatchAttribs::kWideColorIfEnabled>::value) {
            SkASSERT(attribs & PatchAttribs::kWideColorIfEnabled);    // required
        } else if constexpr (!opt_attrib<PatchAttribs::kWideColorIfEnabled>::value) {
            SkASSERT(!(attribs & PatchAttribs::kWideColorIfEnabled)); // disabled
        }
    }

    ~PatchWriter() {
        if constexpr (kTrackJoinControlPoints) {
            // flush any pending patch
            this->writeDeferredStrokePatch();
        }
    }

    PatchAttribs attribs() const { return fAttribs; }

    // The max scale factor should be derived from the same matrix that 'xform' was. It's only used
    // in stroking calculations, so can be ignored for path filling.
    void setShaderTransform(const wangs_formula::VectorXform& xform,
                            float maxScale = 1.f) {
        fApproxTransform = xform;
        fMaxScale = maxScale;
    }

    // Completes a closed contour of a stroke by rewriting a deferred patch with now-available
    // join control point information. Automatically resets the join control point attribute.
    ENABLE_IF(kTrackJoinControlPoints) writeDeferredStrokePatch() {
        if (fDeferredPatch.hasPending()) {
            SkASSERT(!fDeferredPatch.fMustDefer);
            // Overwrite join control point with updated value, which is the first attribute
            // after the 4 control points.
            memcpy(SkTAddOffset<void>(fDeferredPatch.fData, 4 * sizeof(SkPoint)),
                   &fJoin, sizeof(SkPoint));
            // Assuming that the stroke parameters aren't changing within a contour, we only have
            // to set the parametric segments in order to recover the LinearTolerances state at the
            // time the deferred patch was recorded.
            fTolerances.setParametricSegments(fDeferredPatch.fN_p4);
            if (VertexWriter vw = fPatchAllocator.append(fTolerances)) {
                vw << VertexWriter::Array<char>(fDeferredPatch.fData, PatchStride(fAttribs));
            }
        }

        fDeferredPatch.reset();
    }

    // Updates the stroke's join control point that will be written out with each patch. This is
    // automatically adjusted when appending various geometries (e.g. Conic/Cubic), but sometimes
    // must be set explicitly.
    ENABLE_IF(JoinAttrib::kEnabled) updateJoinControlPointAttrib(SkPoint lastControlPoint) {
        SkASSERT(fAttribs & PatchAttribs::kJoinControlPoint); // must be runtime enabled as well
        fJoin = lastControlPoint;
        if constexpr (kTrackJoinControlPoints) {
            fDeferredPatch.fMustDefer = false;
        }
    }

    // Updates the fan point that will be written out with each patch (i.e., the point that wedges
    // fan around).
    ENABLE_IF(FanPointAttrib::kEnabled) updateFanPointAttrib(SkPoint fanPoint) {
        SkASSERT(fAttribs & PatchAttribs::kFanPoint);
        fFanPoint = fanPoint;
    }

    // Updates the stroke params that are written out with each patch.
    ENABLE_IF(StrokeAttrib::kEnabled) updateStrokeParamsAttrib(StrokeParams strokeParams) {
        SkASSERT(fAttribs & PatchAttribs::kStrokeParams);
        fStrokeParams = strokeParams;
        fTolerances.setStroke(strokeParams, fMaxScale);
    }
    // Updates tolerances to account for stroke params that are stored as uniforms instead of
    // dynamic instance attributes.
    ENABLE_IF(StrokeAttrib::kEnabled) updateUniformStrokeParams(StrokeParams strokeParams) {
        SkASSERT(!(fAttribs & PatchAttribs::kStrokeParams));
        fTolerances.setStroke(strokeParams, fMaxScale);
    }

    // Updates the color that will be written out with each patch.
    ENABLE_IF(ColorAttrib::kEnabled) updateColorAttrib(const SkPMColor4f& color) {
        SkASSERT(fAttribs & PatchAttribs::kColor);
        // Converts SkPMColor4f to the selected 'Color' attrib type. The always-wide and never-wide
        // branches match what VertexColor does based on the runtime check.
        if constexpr (req_attrib<PatchAttribs::kWideColorIfEnabled>::value) {
            fColor = color;
        } else if constexpr (opt_attrib<PatchAttribs::kWideColorIfEnabled>::value) {
            fColor = VertexColor(color, fAttribs & PatchAttribs::kWideColorIfEnabled);
        } else {
            fColor = color.toBytes_RGBA();
        }
    }

    // Updates the paint depth written out with each patch.
    ENABLE_IF(DepthAttrib::kEnabled) updatePaintDepthAttrib(float depth) {
        SkASSERT(fAttribs & PatchAttribs::kPaintDepth);
        fDepth = depth;
    }

    // Updates the storage buffer index used to access uniforms.
    ENABLE_IF(SsboIndexAttrib::kEnabled)
    updateSsboIndexAttrib(skvx::ushort2 ssboIndex) {
        SkASSERT(fAttribs & PatchAttribs::kSsboIndex);
        fSsboIndex = ssboIndex;
    }

    /**
     * writeX functions for supported patch geometry types. Every geometric type is converted to an
     * equivalent cubic or conic, so this will always write at minimum 8 floats for the four control
     * points (cubic) or three control points and {w, inf} (conics). The PatchWriter additionally
     * writes the current values of all attributes enabled in its PatchAttribs flags.
     */

    // Write a cubic curve with its four control points.
    AI void writeCubic(float2 p0, float2 p1, float2 p2, float2 p3) {
        float n4 = wangs_formula::cubic_p4(kPrecision, p0, p1, p2, p3, fApproxTransform);
        if constexpr (kDiscardFlatCurves) {
            if (n4 <= 1.f) {
                // This cubic only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (int numPatches = this->accountForCurve(n4)) {
            this->chopAndWriteCubics(p0, p1, p2, p3, numPatches);
        } else {
            this->writeCubicPatch(p0, p1, p2, p3);
        }
    }
    AI void writeCubic(const SkPoint pts[4]) {
        float4 p0p1 = float4::Load(pts);
        float4 p2p3 = float4::Load(pts + 2);
        this->writeCubic(p0p1.lo, p0p1.hi, p2p3.lo, p2p3.hi);
    }

    // Write a conic curve with three control points and 'w', with the last coord of the last
    // control point signaling a conic by being set to infinity.
    AI void writeConic(float2 p0, float2 p1, float2 p2, float w) {
        float n2 = wangs_formula::conic_p2(kPrecision, p0, p1, p2, w, fApproxTransform);
        if constexpr (kDiscardFlatCurves) {
            if (n2 <= 1.f) {
                // This conic only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (int numPatches = this->accountForCurve(n2 * n2)) {
            this->chopAndWriteConics(p0, p1, p2, w, numPatches);
        } else {
            this->writeConicPatch(p0, p1, p2, w);
        }
    }
    AI void writeConic(const SkPoint pts[3], float w) {
        this->writeConic(sk_bit_cast<float2>(pts[0]),
                         sk_bit_cast<float2>(pts[1]),
                         sk_bit_cast<float2>(pts[2]),
                         w);
    }

    // Write a quadratic curve that automatically converts its three control points into an
    // equivalent cubic.
    AI void writeQuadratic(float2 p0, float2 p1, float2 p2) {
        float n4 = wangs_formula::quadratic_p4(kPrecision, p0, p1, p2, fApproxTransform);
        if constexpr (kDiscardFlatCurves) {
            if (n4 <= 1.f) {
                // This quad only needs one segment (e.g. a line) but we're not filling space with
                // fans or stroking, so nothing actually needs to be drawn.
                return;
            }
        }
        if (int numPatches = this->accountForCurve(n4)) {
            this->chopAndWriteQuads(p0, p1, p2, numPatches);
        } else {
            this->writeQuadPatch(p0, p1, p2);
        }
    }
    AI void writeQuadratic(const SkPoint pts[3]) {
        this->writeQuadratic(sk_bit_cast<float2>(pts[0]),
                             sk_bit_cast<float2>(pts[1]),
                             sk_bit_cast<float2>(pts[2]));
    }

    // Write a line that is automatically converted into an equivalent cubic.
    AI void writeLine(float4 p0p1) {
        // No chopping needed, a line only ever requires one segment (the minimum required already).
        fTolerances.setParametricSegments(1.f);
        if constexpr (kReplicateLineEndPoints) {
            // Visually this cubic is still a line, but 't' does not move linearly over the line,
            // so Wang's formula is more pessimistic. Shaders should avoid evaluating Wang's
            // formula when a patch has control points in this arrangement.
            this->writeCubicPatch(p0p1.lo, p0p1.lo, p0p1.hi, p0p1.hi);
        } else {
            // In exact math, this cubic structure should have Wang's formula return 0. Due to
            // floating point math, this isn't always the case, so shaders need some way to restrict
            // the number of parametric segments if Wang's formula numerically blows up.
            this->writeCubicPatch(p0p1.lo, (p0p1.zwxy() - p0p1) * (1/3.f) + p0p1, p0p1.hi);
        }
    }
    AI void writeLine(float2 p0, float2 p1) { this->writeLine({p0, p1}); }
    AI void writeLine(SkPoint p0, SkPoint p1) {
        this->writeLine(sk_bit_cast<float2>(p0), sk_bit_cast<float2>(p1));
    }

    // Write a triangle by setting it to a conic with w=Inf, and using a distinct
    // explicit curve type for when inf isn't supported in shaders.
    AI void writeTriangle(float2 p0, float2 p1, float2 p2) {
        // No chopping needed, the max supported segment count should always support 2 lines
        // (which form a triangle when implicitly closed).
        static constexpr float kTriangleSegments_p4 = 2.f * 2.f * 2.f * 2.f;
        fTolerances.setParametricSegments(kTriangleSegments_p4);
        this->writePatch(p0, p1, p2, {SK_FloatInfinity, SK_FloatInfinity},
                         kTriangularConicCurveType);
    }
    AI void writeTriangle(SkPoint p0, SkPoint p1, SkPoint p2) {
        this->writeTriangle(sk_bit_cast<float2>(p0),
                            sk_bit_cast<float2>(p1),
                            sk_bit_cast<float2>(p2));
    }

    // Writes a circle used for round caps and joins in stroking, encoded as a cubic with
    // identical control points and an empty join.
    AI void writeCircle(SkPoint p) {
        // This does not use writePatch() because it uses its own location as the join attribute
        // value instead of fJoin and never defers.
        fTolerances.setParametricSegments(0.f);
        if (VertexWriter vw = fPatchAllocator.append(fTolerances)) {
            vw << VertexWriter::Repeat<4>(p); // p0,p1,p2,p3 = p -> 4 copies
            this->emitPatchAttribs(std::move(vw), {fAttribs, p}, kCubicCurveType);
        }
    }

private:
    AI void emitPatchAttribs(VertexWriter vertexWriter,
                             const JoinAttrib& join,
                             float explicitCurveType) {
        // NOTE: operator<< overrides automatically handle optional and disabled attribs.
        vertexWriter << join << fFanPoint << fStrokeParams << fColor << fDepth
                     << CurveTypeAttrib{fAttribs, explicitCurveType} << fSsboIndex;
    }

    AI VertexWriter appendPatch() {
        if constexpr (kTrackJoinControlPoints) {
            if (fDeferredPatch.fMustDefer) {
                SkASSERT(!fDeferredPatch.hasPending());
                SkASSERT(PatchStride(fAttribs) <= kMaxStride);
                // Save the computed parametric segment tolerance value so that we can pass that to
                // the PatchAllocator when flushing the deferred patch.
                fDeferredPatch.fN_p4 = fTolerances.numParametricSegments_p4();
                return {fDeferredPatch.fData, PatchStride(fAttribs)};
            }
        }
        return fPatchAllocator.append(fTolerances);
    }

    AI void writePatch(float2 p0, float2 p1, float2 p2, float2 p3, float explicitCurveType) {
        if (VertexWriter vw = this->appendPatch()) {
            // NOTE: fJoin will be undefined if we're writing to a deferred patch. If that's the
            // case, correct data will overwrite it when the contour is closed (this is fine since a
            // deferred patch writes to CPU memory instead of directly to the GPU buffer).
            vw << p0 << p1 << p2 << p3;
            this->emitPatchAttribs(std::move(vw), fJoin, explicitCurveType);

            // Automatically update join control point for next patch.
            if constexpr (kTrackJoinControlPoints) {
                if (explicitCurveType == kCubicCurveType && any(p3 != p2)) {
                    // p2 is control point defining the tangent vector into the next patch.
                    p2.store(&fJoin);
                } else if (any(p2 != p1)) {
                    // p1 is the control point defining the tangent vector.
                    p1.store(&fJoin);
                } else {
                    // p0 is the control point defining the tangent vector.
                    p0.store(&fJoin);
                }
                fDeferredPatch.fMustDefer = false;
            }
        }
    }

    // Helpers that normalize curves to a generic patch, but do no other work.
    AI void writeCubicPatch(float2 p0, float2 p1, float2 p2, float2 p3) {
        this->writePatch(p0, p1, p2, p3, kCubicCurveType);
    }
    AI void writeCubicPatch(float2 p0, float4 p1p2, float2 p3) {
        this->writeCubicPatch(p0, p1p2.lo, p1p2.hi, p3);
    }
    AI void writeQuadPatch(float2 p0, float2 p1, float2 p2) {
        this->writeCubicPatch(p0, mix(float4(p0, p2), p1.xyxy(), 2/3.f), p2);
    }
    AI void writeConicPatch(float2 p0, float2 p1, float2 p2, float w) {
        this->writePatch(p0, p1, p2, {w, SK_FloatInfinity}, kConicCurveType);
    }

    int accountForCurve(float n4) {
        if (n4 <= kMaxParametricSegments_p4) {
            // Record n^4 and return 0 to signal no chopping
            fTolerances.setParametricSegments(n4);
            return 0;
        } else {
            // Clamp to max allowed segmentation for a patch and return required number of chops
            // to achieve visual correctness.
            fTolerances.setParametricSegments(kMaxParametricSegments_p4);
            return SkScalarCeilToInt(wangs_formula::root4(std::min(n4, kMaxSegmentsPerCurve_p4) /
                                                          kMaxParametricSegments_p4));
        }
    }

    // This does not return b when t==1, but it otherwise seems to get better precision than
    // "a*(1 - t) + b*t" for things like chopping cubics on exact cusp points.
    // The responsibility falls on the caller to check that t != 1 before calling.
    static AI float4 mix(float4 a, float4 b, float4 T) {
        SkASSERT(all((0 <= T) & (T < 1)));
        return (b - a)*T + a;
    }

    // Helpers that chop the curve type into 'numPatches' parametrically uniform curves. It is
    // assumed that 'numPatches' is calculated such that the resulting curves require the maximum
    // number of segments to draw appropriately (since the original presumably needed even more).
    void chopAndWriteQuads(float2 p0, float2 p1, float2 p2, int numPatches) {
        InnerTriangulator triangulator(numPatches, sk_bit_cast<SkPoint>(p0));
        for (; numPatches >= 3; numPatches -= 2) {
            // Chop into 3 quads.
            float4 T = float4(1,1,2,2) / numPatches;
            float4 ab = mix(p0.xyxy(), p1.xyxy(), T);
            float4 bc = mix(p1.xyxy(), p2.xyxy(), T);
            float4 abc = mix(ab, bc, T);
            // p1 & p2 of the cubic representation of the middle quad.
            float4 middle = mix(ab, bc, mix(T, T.zwxy(), 2/3.f));

            this->writeQuadPatch(p0, ab.lo, abc.lo);  // Write the 1st quad.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abc.lo, abc.hi);
            }
            this->writeCubicPatch(abc.lo, middle, abc.hi);  // Write the 2nd quad (already a cubic)
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(abc.hi)));
            }
            std::tie(p0, p1) = {abc.hi, bc.hi};  // Save the 3rd quad.
        }
        if (numPatches == 2) {
            // Chop into 2 quads.
            float2 ab = (p0 + p1) * .5f;
            float2 bc = (p1 + p2) * .5f;
            float2 abc = (ab + bc) * .5f;

            this->writeQuadPatch(p0, ab, abc);  // Write the 1st quad.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abc, p2);
            }
            this->writeQuadPatch(abc, bc, p2);  // Write the 2nd quad.
        } else {
            SkASSERT(numPatches == 1);
            this->writeQuadPatch(p0, p1, p2);  // Write the single remaining quad.
        }
        if constexpr (kAddTrianglesWhenChopping) {
            this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(p2)));
            this->writeTriangleStack(triangulator.close());
        }
    }

    void chopAndWriteConics(float2 p0, float2 p1, float2 p2, float w, int numPatches) {
        InnerTriangulator triangulator(numPatches, sk_bit_cast<SkPoint>(p0));
        // Load the conic in 3d homogeneous (unprojected) space.
        float4 h0 = float4(p0,1,1);
        float4 h1 = float4(p1,1,1) * w;
        float4 h2 = float4(p2,1,1);
        for (; numPatches >= 2; --numPatches) {
            // Chop in homogeneous space.
            float T = 1.f/numPatches;
            float4 ab = mix(h0, h1, T);
            float4 bc = mix(h1, h2, T);
            float4 abc = mix(ab, bc, T);

            // Project and write the 1st conic.
            float2 midpoint = abc.xy() / abc.w();
            this->writeConicPatch(h0.xy() / h0.w(),
                                  ab.xy() / ab.w(),
                                  midpoint,
                                  ab.w() / sqrtf(h0.w() * abc.w()));
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(midpoint)));
            }
            std::tie(h0, h1) = {abc, bc};  // Save the 2nd conic (in homogeneous space).
        }
        // Project and write the remaining conic.
        SkASSERT(numPatches == 1);
        this->writeConicPatch(h0.xy() / h0.w(),
                              h1.xy() / h1.w(),
                              h2.xy(), // h2.w == 1
                              h1.w() / sqrtf(h0.w()));
        if constexpr (kAddTrianglesWhenChopping) {
            this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(h2.xy())));
            this->writeTriangleStack(triangulator.close());
        }
    }

    void chopAndWriteCubics(float2 p0, float2 p1, float2 p2, float2 p3, int numPatches) {
        InnerTriangulator triangulator(numPatches, sk_bit_cast<SkPoint>(p0));
        for (; numPatches >= 3; numPatches -= 2) {
            // Chop into 3 cubics.
            float4 T = float4(1,1,2,2) / numPatches;
            float4 ab = mix(p0.xyxy(), p1.xyxy(), T);
            float4 bc = mix(p1.xyxy(), p2.xyxy(), T);
            float4 cd = mix(p2.xyxy(), p3.xyxy(), T);
            float4 abc = mix(ab, bc, T);
            float4 bcd = mix(bc, cd, T);
            float4 abcd = mix(abc, bcd, T);
            float4 middle = mix(abc, bcd, T.zwxy());  // p1 & p2 of the middle cubic.

            this->writeCubicPatch(p0, ab.lo, abc.lo, abcd.lo);  // Write the 1st cubic.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abcd.lo, abcd.hi);
            }
            this->writeCubicPatch(abcd.lo, middle, abcd.hi);  // Write the 2nd cubic.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(abcd.hi)));
            }
            std::tie(p0, p1, p2) = {abcd.hi, bcd.hi, cd.hi};  // Save the 3rd cubic.
        }
        if (numPatches == 2) {
            // Chop into 2 cubics.
            float2 ab = (p0 + p1) * .5f;
            float2 bc = (p1 + p2) * .5f;
            float2 cd = (p2 + p3) * .5f;
            float2 abc = (ab + bc) * .5f;
            float2 bcd = (bc + cd) * .5f;
            float2 abcd = (abc + bcd) * .5f;

            this->writeCubicPatch(p0, ab, abc, abcd);  // Write the 1st cubic.
            if constexpr (kAddTrianglesWhenChopping) {
                this->writeTriangle(p0, abcd, p3);
            }
            this->writeCubicPatch(abcd, bcd, cd, p3);  // Write the 2nd cubic.
        } else {
            SkASSERT(numPatches == 1);
            this->writeCubicPatch(p0, p1, p2, p3);  // Write the single remaining cubic.
        }
        if constexpr (kAddTrianglesWhenChopping) {
            this->writeTriangleStack(triangulator.pushVertex(sk_bit_cast<SkPoint>(p3)));
            this->writeTriangleStack(triangulator.close());
        }
    }

    ENABLE_IF(kAddTrianglesWhenChopping)
    writeTriangleStack(MiddleOutPolygonTriangulator::PoppedTriangleStack&& stack) {
        for (auto [p0, p1, p2] : stack) {
            this->writeTriangle(p0, p1, p2);
        }
    }

    // Runtime configuration, will always contain required attribs but may not have all optional
    // attribs enabled (e.g. depending on caps or batching).
    const PatchAttribs fAttribs;

    // The 2x2 approximation of the local-to-device transform that will affect subsequently
    // recorded curves (when fully transformed in the vertex shader).
    wangs_formula::VectorXform fApproxTransform = {};
    // A maximum scale factor extracted from the current approximate transform.
    float fMaxScale = 1.0f;
    // Tracks the linear tolerances for the most recently written patches.
    LinearTolerances fTolerances;

    PatchAllocator fPatchAllocator;
    DeferredPatch  fDeferredPatch; // only usable if kTrackJoinControlPoints is true

    // Instance attribute state written after the 4 control points of a patch
    JoinAttrib     fJoin;
    FanPointAttrib fFanPoint;
    StrokeAttrib   fStrokeParams;
    ColorAttrib    fColor;
    DepthAttrib    fDepth;

    // Index into a shared storage buffer containing this PatchWriter's patches' corresponding
    // uniforms. Written out as an attribute with every patch, to read the appropriate uniform
    // values from the storage buffer on draw.
    SsboIndexAttrib fSsboIndex;
};

}  // namespace skgpu::tess

#undef ENABLE_IF
#undef AI

#endif  // skgpu_tessellate_PatchWriter_DEFINED
