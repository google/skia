/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTypes_DEFINED
#define GrTypes_DEFINED

#include "include/core/SkMath.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrConfig.h"

class GrBackendSemaphore;

////////////////////////////////////////////////////////////////////////////////

/**
 * Defines overloaded bitwise operators to make it easier to use an enum as a
 * bitfield.
 */
#define GR_MAKE_BITFIELD_OPS(X) \
    inline X operator |(X a, X b) { \
        return (X) (+a | +b); \
    } \
    inline X& operator |=(X& a, X b) { \
        return (a = a | b); \
    } \
    inline X operator &(X a, X b) { \
        return (X) (+a & +b); \
    } \
    inline X& operator &=(X& a, X b) { \
        return (a = a & b); \
    } \
    template <typename T> \
    inline X operator &(T a, X b) { \
        return (X) (+a & +b); \
    } \
    template <typename T> \
    inline X operator &(X a, T b) { \
        return (X) (+a & +b); \
    } \

#define GR_DECL_BITFIELD_OPS_FRIENDS(X) \
    friend X operator |(X a, X b); \
    friend X& operator |=(X& a, X b); \
    \
    friend X operator &(X a, X b); \
    friend X& operator &=(X& a, X b); \
    \
    template <typename T> \
    friend X operator &(T a, X b); \
    \
    template <typename T> \
    friend X operator &(X a, T b); \

/**
 * Wraps a C++11 enum that we use as a bitfield, and enables a limited amount of
 * masking with type safety. Instantiated with the ~ operator.
 */
template<typename TFlags> class GrTFlagsMask {
public:
    constexpr explicit GrTFlagsMask(TFlags value) : GrTFlagsMask(static_cast<int>(value)) {}
    constexpr explicit GrTFlagsMask(int value) : fValue(value) {}
    constexpr int value() const { return fValue; }
private:
    const int fValue;
};

// Or-ing a mask always returns another mask.
template<typename TFlags> constexpr GrTFlagsMask<TFlags> operator|(GrTFlagsMask<TFlags> a,
                                                                   GrTFlagsMask<TFlags> b) {
    return GrTFlagsMask<TFlags>(a.value() | b.value());
}
template<typename TFlags> constexpr GrTFlagsMask<TFlags> operator|(GrTFlagsMask<TFlags> a,
                                                                   TFlags b) {
    return GrTFlagsMask<TFlags>(a.value() | static_cast<int>(b));
}
template<typename TFlags> constexpr GrTFlagsMask<TFlags> operator|(TFlags a,
                                                                   GrTFlagsMask<TFlags> b) {
    return GrTFlagsMask<TFlags>(static_cast<int>(a) | b.value());
}
template<typename TFlags> inline GrTFlagsMask<TFlags>& operator|=(GrTFlagsMask<TFlags>& a,
                                                                  GrTFlagsMask<TFlags> b) {
    return (a = a | b);
}

// And-ing two masks returns another mask; and-ing one with regular flags returns flags.
template<typename TFlags> constexpr GrTFlagsMask<TFlags> operator&(GrTFlagsMask<TFlags> a,
                                                                   GrTFlagsMask<TFlags> b) {
    return GrTFlagsMask<TFlags>(a.value() & b.value());
}
template<typename TFlags> constexpr TFlags operator&(GrTFlagsMask<TFlags> a, TFlags b) {
    return static_cast<TFlags>(a.value() & static_cast<int>(b));
}
template<typename TFlags> constexpr TFlags operator&(TFlags a, GrTFlagsMask<TFlags> b) {
    return static_cast<TFlags>(static_cast<int>(a) & b.value());
}
template<typename TFlags> inline TFlags& operator&=(TFlags& a, GrTFlagsMask<TFlags> b) {
    return (a = a & b);
}

/**
 * Defines bitwise operators that make it possible to use an enum class as a
 * basic bitfield.
 */
#define GR_MAKE_BITFIELD_CLASS_OPS(X) \
    constexpr GrTFlagsMask<X> operator~(X a) { \
        return GrTFlagsMask<X>(~static_cast<int>(a)); \
    } \
    constexpr X operator|(X a, X b) { \
        return static_cast<X>(static_cast<int>(a) | static_cast<int>(b)); \
    } \
    inline X& operator|=(X& a, X b) { \
        return (a = a | b); \
    } \
    constexpr bool operator&(X a, X b) { \
        return SkToBool(static_cast<int>(a) & static_cast<int>(b)); \
    } \

#define GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(X) \
    friend constexpr GrTFlagsMask<X> operator ~(X); \
    friend constexpr X operator |(X, X); \
    friend X& operator |=(X&, X); \
    friend constexpr bool operator &(X, X)

////////////////////////////////////////////////////////////////////////////////

// compile time versions of min/max
#define GR_CT_MAX(a, b) (((b) < (a)) ? (a) : (b))
#define GR_CT_MIN(a, b) (((b) < (a)) ? (b) : (a))

/**
 *  divide, rounding up
 */
static inline constexpr int32_t GrIDivRoundUp(int x, int y) {
    SkASSERT(y > 0);
    return (x + (y-1)) / y;
}
static inline constexpr uint32_t GrUIDivRoundUp(uint32_t x, uint32_t y) {
    return (x + (y-1)) / y;
}
static inline constexpr size_t GrSizeDivRoundUp(size_t x, size_t y) { return (x + (y - 1)) / y; }

/**
 *  align up
 */
static inline constexpr uint32_t GrUIAlignUp(uint32_t x, uint32_t alignment) {
    return GrUIDivRoundUp(x, alignment) * alignment;
}
static inline constexpr size_t GrSizeAlignUp(size_t x, size_t alignment) {
    return GrSizeDivRoundUp(x, alignment) * alignment;
}

/**
 * amount of pad needed to align up
 */
static inline constexpr uint32_t GrUIAlignUpPad(uint32_t x, uint32_t alignment) {
    return (alignment - x % alignment) % alignment;
}
static inline constexpr size_t GrSizeAlignUpPad(size_t x, size_t alignment) {
    return (alignment - x % alignment) % alignment;
}

/**
 *  align down
 */
static inline constexpr uint32_t GrUIAlignDown(uint32_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}
static inline constexpr size_t GrSizeAlignDown(size_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Possible 3D APIs that may be used by Ganesh.
 */
enum class GrBackendApi : unsigned {
    kMetal,
    kOpenGL,
    kVulkan,
    /**
     * Mock is a backend that does not draw anything. It is used for unit tests
     * and to measure CPU overhead.
     */
    kMock,

    /**
     * Added here to support the legacy GrBackend enum value and clients who referenced it using
     * GrBackend::kOpenGL_GrBackend.
     */
    kOpenGL_GrBackend = kOpenGL,
};

/**
 * Previously the above enum was not an enum class but a normal enum. To support the legacy use of
 * the enum values we define them below so that no clients break.
 */
typedef GrBackendApi GrBackend;

static constexpr GrBackendApi kMetal_GrBackend = GrBackendApi::kMetal;
static constexpr GrBackendApi kVulkan_GrBackend = GrBackendApi::kVulkan;
static constexpr GrBackendApi kMock_GrBackend = GrBackendApi::kMock;

///////////////////////////////////////////////////////////////////////////////

/**
 * Used to say whether a texture has mip levels allocated or not.
 */
enum class GrMipMapped : bool {
    kNo = false,
    kYes = true
};

///////////////////////////////////////////////////////////////////////////////

/**
 * GPU SkImage and SkSurfaces can be stored such that (0, 0) in texture space may correspond to
 * either the top-left or bottom-left content pixel.
 */
enum GrSurfaceOrigin : int {
    kTopLeft_GrSurfaceOrigin,
    kBottomLeft_GrSurfaceOrigin,
};

/**
 * A GrContext's cache of backend context state can be partially invalidated.
 * These enums are specific to the GL backend and we'd add a new set for an alternative backend.
 */
enum GrGLBackendState {
    kRenderTarget_GrGLBackendState     = 1 << 0,
    // Also includes samplers bound to texture units.
    kTextureBinding_GrGLBackendState   = 1 << 1,
    // View state stands for scissor and viewport
    kView_GrGLBackendState             = 1 << 2,
    kBlend_GrGLBackendState            = 1 << 3,
    kMSAAEnable_GrGLBackendState       = 1 << 4,
    kVertex_GrGLBackendState           = 1 << 5,
    kStencil_GrGLBackendState          = 1 << 6,
    kPixelStore_GrGLBackendState       = 1 << 7,
    kProgram_GrGLBackendState          = 1 << 8,
    kFixedFunction_GrGLBackendState    = 1 << 9,
    kMisc_GrGLBackendState             = 1 << 10,
    kPathRendering_GrGLBackendState    = 1 << 11,
    kALL_GrGLBackendState              = 0xffff
};

/**
 * This value translates to reseting all the context state for any backend.
 */
static const uint32_t kAll_GrBackendState = 0xffffffff;

enum GrFlushFlags {
    kNone_GrFlushFlags = 0,
    // flush will wait till all submitted GPU work is finished before returning.
    kSyncCpu_GrFlushFlag = 0x1,
};

typedef void* GrGpuFinishedContext;
typedef void (*GrGpuFinishedProc)(GrGpuFinishedContext finishedContext);

/**
 * Struct to supply options to flush calls.
 *
 * After issuing all commands, fNumSemaphore semaphores will be signaled by the gpu. The client
 * passes in an array of fNumSemaphores GrBackendSemaphores. In general these GrBackendSemaphore's
 * can be either initialized or not. If they are initialized, the backend uses the passed in
 * semaphore. If it is not initialized, a new semaphore is created and the GrBackendSemaphore
 * object is initialized with that semaphore.
 *
 * The client will own and be responsible for deleting the underlying semaphores that are stored
 * and returned in initialized GrBackendSemaphore objects. The GrBackendSemaphore objects
 * themselves can be deleted as soon as this function returns.
 *
 * If a finishedProc is provided, the finishedProc will be called when all work submitted to the gpu
 * from this flush call and all previous flush calls has finished on the GPU. If the flush call
 * fails due to an error and nothing ends up getting sent to the GPU, the finished proc is called
 * immediately.
 */
struct GrFlushInfo {
    GrFlushFlags         fFlags = kNone_GrFlushFlags;
    int                  fNumSemaphores = 0;
    GrBackendSemaphore*  fSignalSemaphores = nullptr;
    GrGpuFinishedProc    fFinishedProc = nullptr;
    GrGpuFinishedContext fFinishedContext = nullptr;
};

/**
 * Enum used as return value when flush with semaphores so the client knows whether the semaphores
 * were submitted to GPU or not.
 */
enum class GrSemaphoresSubmitted : bool {
    kNo = false,
    kYes = true
};

#endif
