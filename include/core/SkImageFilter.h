/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilter_DEFINED
#define SkImageFilter_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#if SK_SUPPORT_GPU
#include "include/gpu/GrTypes.h"
#endif

class GrFragmentProcessor;
class SkColorFilter;
struct SkIPoint;
class GrRecordingContext;
class SkSpecialImage;
class SkImageFilterCache;
struct SkImageFilterCacheKey;

/**
 *  Base class for image filters. If one is installed in the paint, then
 *  all drawing occurs as usual, but it is as if the drawing happened into an
 *  offscreen (before the xfermode is applied). This offscreen bitmap will
 *  then be handed to the imagefilter, who in turn creates a new bitmap which
 *  is what will finally be drawn to the device (using the original xfermode).
 */
class SK_API SkImageFilter : public SkFlattenable {
public:
    // Extra information about the output of a filter DAG. For now, this is just the color space
    // (of the original requesting device). This is used when constructing intermediate rendering
    // surfaces, so that we ensure we land in a surface that's similar/compatible to the final
    // consumer of the DAG's output.
    class OutputProperties {
    public:
        explicit OutputProperties(SkColorType colorType, SkColorSpace* colorSpace)
            : fColorType(colorType), fColorSpace(colorSpace) {}

        SkColorType colorType() const { return fColorType; }
        SkColorSpace* colorSpace() const { return fColorSpace; }

    private:
        SkColorType fColorType;
        // This will be a pointer to the device's color space, and our lifetime is bounded by
        // the device, so we can store a bare pointer.
        SkColorSpace* fColorSpace;
    };

    class Context {
    public:
        Context(const SkMatrix& ctm, const SkIRect& clipBounds, SkImageFilterCache* cache,
                const OutputProperties& outputProperties)
            : fCTM(ctm)
            , fClipBounds(clipBounds)
            , fCache(cache)
            , fOutputProperties(outputProperties)
        {}

        const SkMatrix& ctm() const { return fCTM; }
        const SkIRect& clipBounds() const { return fClipBounds; }
        SkImageFilterCache* cache() const { return fCache; }
        const OutputProperties& outputProperties() const { return fOutputProperties; }

        /**
         *  Since a context can be build directly, its constructor has no chance to
         *  "return null" if it's given invalid or unsupported inputs. Call this to
         *  know of the the context can be used.
         *
         *  The SkImageFilterCache Key, for example, requires a finite ctm (no infinities
         *  or NaN), so that test is part of isValid.
         */
        bool isValid() const { return fCTM.isFinite(); }

    private:
        SkMatrix               fCTM;
        SkIRect                fClipBounds;
        SkImageFilterCache*    fCache;
        OutputProperties       fOutputProperties;
    };

    class CropRect {
    public:
        enum CropEdge {
            kHasLeft_CropEdge   = 0x01,
            kHasTop_CropEdge    = 0x02,
            kHasWidth_CropEdge  = 0x04,
            kHasHeight_CropEdge = 0x08,
            kHasAll_CropEdge    = 0x0F,
        };
        CropRect() {}
        explicit CropRect(const SkRect& rect, uint32_t flags = kHasAll_CropEdge)
            : fRect(rect), fFlags(flags) {}
        uint32_t flags() const { return fFlags; }
        const SkRect& rect() const { return fRect; }

        /**
         *  Apply this cropRect to the imageBounds. If a given edge of the cropRect is not
         *  set, then the corresponding edge from imageBounds will be used. If "embiggen"
         *  is true, the crop rect is allowed to enlarge the size of the rect, otherwise
         *  it may only reduce the rect. Filters that can affect transparent black should
         *  pass "true", while all other filters should pass "false".
         *
         *  Note: imageBounds is in "device" space, as the output cropped rectangle will be,
         *  so the matrix is ignored for those. It is only applied the croprect's bounds.
         */
        void applyTo(const SkIRect& imageBounds, const SkMatrix& matrix, bool embiggen,
                     SkIRect* cropped) const;

    private:
        SkRect fRect;
        uint32_t fFlags;
    };

    enum TileUsage {
        kPossible_TileUsage,    //!< the created device may be drawn tiled
        kNever_TileUsage,       //!< the created device will never be drawn tiled
    };

    /**
     *  Request a new filtered image to be created from the src image.
     *
     *  The context contains the environment in which the filter is occurring.
     *  It includes the clip bounds, CTM and cache.
     *
     *  Offset is the amount to translate the resulting image relative to the
     *  src when it is drawn. This is an out-param.
     *
     *  If the result image cannot be created, or the result would be
     *  transparent black, return null, in which case the offset parameter
     *  should be ignored by the caller.
     *
     *  TODO: Right now the imagefilters sometimes return empty result bitmaps/
     *        specialimages. That doesn't seem quite right.
     */
    sk_sp<SkSpecialImage> filterImage(SkSpecialImage* src, const Context& context,
                                      SkIPoint* offset) const;

    enum MapDirection {
        kForward_MapDirection,
        kReverse_MapDirection,
    };
    /**
     * Map a device-space rect recursively forward or backward through the
     * filter DAG. kForward_MapDirection is used to determine which pixels of
     * the destination canvas a source image rect would touch after filtering.
     * kReverse_MapDirection is used to determine which rect of the source
     * image would be required to fill the given rect (typically, clip bounds).
     * Used for clipping and temp-buffer allocations, so the result need not
     * be exact, but should never be smaller than the real answer. The default
     * implementation recursively unions all input bounds, or returns the
     * source rect if no inputs.
     *
     * In kReverse mode, 'inputRect' is the device-space bounds of the input pixels. In kForward
     * mode it should always be null. If 'inputRect' is null in kReverse mode the resulting
     * answer may be incorrect.
     */
    SkIRect filterBounds(const SkIRect& src, const SkMatrix& ctm,
                         MapDirection, const SkIRect* inputRect = nullptr) const;

#if SK_SUPPORT_GPU
    static sk_sp<SkSpecialImage> DrawWithFP(GrRecordingContext* context,
                                            std::unique_ptr<GrFragmentProcessor>
                                                    fp,
                                            const SkIRect& bounds,
                                            const OutputProperties& outputProperties,
                                            GrProtected isProtected = GrProtected::kNo);
#endif

    /**
     *  Returns whether this image filter is a color filter and puts the color filter into the
     *  "filterPtr" parameter if it can. Does nothing otherwise.
     *  If this returns false, then the filterPtr is unchanged.
     *  If this returns true, then if filterPtr is not null, it must be set to a ref'd colorfitler
     *  (i.e. it may not be set to NULL).
     */
    bool isColorFilterNode(SkColorFilter** filterPtr) const {
        return this->onIsColorFilterNode(filterPtr);
    }

    // DEPRECATED : use isColorFilterNode() instead
    bool asColorFilter(SkColorFilter** filterPtr) const {
        return this->isColorFilterNode(filterPtr);
    }

    void removeKey(const SkImageFilterCacheKey& key) const;

    /**
     *  Returns true (and optionally returns a ref'd filter) if this imagefilter can be completely
     *  replaced by the returned colorfilter. i.e. the two effects will affect drawing in the
     *  same way.
     */
    bool asAColorFilter(SkColorFilter** filterPtr) const;

    /**
     *  Returns the number of inputs this filter will accept (some inputs can
     *  be NULL).
     */
    int countInputs() const { return fInputs.count(); }

    /**
     *  Returns the input filter at a given index, or NULL if no input is
     *  connected.  The indices used are filter-specific.
     */
    SkImageFilter* getInput(int i) const {
        SkASSERT(i < fInputs.count());
        return fInputs[i].get();
    }

    /**
     *  Returns whether any edges of the crop rect have been set. The crop
     *  rect is set at construction time, and determines which pixels from the
     *  input image will be processed, and which pixels in the output image will be allowed.
     *  The size of the crop rect should be
     *  used as the size of the destination image. The origin of this rect
     *  should be used to offset access to the input images, and should also
     *  be added to the "offset" parameter in onFilterImage.
     */
    bool cropRectIsSet() const { return fCropRect.flags() != 0x0; }

    CropRect getCropRect() const { return fCropRect; }

    // Default impl returns union of all input bounds.
    virtual SkRect computeFastBounds(const SkRect& bounds) const;

    // Can this filter DAG compute the resulting bounds of an object-space rectangle?
    bool canComputeFastBounds() const;

    /**
     *  If this filter can be represented by another filter + a localMatrix, return that filter,
     *  else return null.
     */
    sk_sp<SkImageFilter> makeWithLocalMatrix(const SkMatrix& matrix) const;

    /**
     *  ImageFilters can natively handle scaling and translate components in the CTM. Only some of
     *  them can handle affine (or more complex) matrices. This call returns true iff the filter
     *  and all of its (non-null) inputs can handle these more complex matrices.
     */
    bool canHandleComplexCTM() const;

    /**
     * Return an imagefilter which transforms its input by the given matrix.
     */
    static sk_sp<SkImageFilter> MakeMatrixFilter(const SkMatrix& matrix,
                                                 SkFilterQuality quality,
                                                 sk_sp<SkImageFilter> input);

    static void RegisterFlattenables();

    static SkFlattenable::Type GetFlattenableType() {
        return kSkImageFilter_Type;
    }

    SkFlattenable::Type getFlattenableType() const override {
        return kSkImageFilter_Type;
    }

    static sk_sp<SkImageFilter> Deserialize(const void* data, size_t size,
                                          const SkDeserialProcs* procs = nullptr) {
        return sk_sp<SkImageFilter>(static_cast<SkImageFilter*>(
                                  SkFlattenable::Deserialize(
                                  kSkImageFilter_Type, data, size, procs).release()));
    }

protected:
    class Common {
    public:
        /**
         *  Attempt to unflatten the cropRect and the expected number of input filters.
         *  If any number of input filters is valid, pass -1.
         *  If this fails (i.e. corrupt buffer or contents) then return false and common will
         *  be left uninitialized.
         *  If this returns true, then inputCount() is the number of found input filters, each
         *  of which may be NULL or a valid imagefilter.
         */
        bool unflatten(SkReadBuffer&, int expectedInputs);

        const CropRect& cropRect() const { return fCropRect; }
        int             inputCount() const { return fInputs.count(); }
        sk_sp<SkImageFilter>* inputs() { return fInputs.begin(); }

        sk_sp<SkImageFilter> getInput(int index) { return fInputs[index]; }

    private:
        CropRect fCropRect;
        // most filters accept at most 2 input-filters
        SkSTArray<2, sk_sp<SkImageFilter>, true> fInputs;
    };

    SkImageFilter(sk_sp<SkImageFilter> const* inputs, int inputCount, const CropRect* cropRect);

    ~SkImageFilter() override;

    /**
     *  Constructs a new SkImageFilter read from an SkReadBuffer object.
     *
     *  @param inputCount    The exact number of inputs expected for this SkImageFilter object.
     *                       -1 can be used if the filter accepts any number of inputs.
     *  @param rb            SkReadBuffer object from which the SkImageFilter is read.
     */
    explicit SkImageFilter(int inputCount, SkReadBuffer& rb);

    void flatten(SkWriteBuffer&) const override;

    const CropRect* getCropRectIfSet() const {
        return this->cropRectIsSet() ? &fCropRect : nullptr;
    }

    /**
     *  This is the virtual which should be overridden by the derived class
     *  to perform image filtering.
     *
     *  src is the original primitive bitmap. If the filter has a connected
     *  input, it should recurse on that input and use that in place of src.
     *
     *  The matrix is the current matrix on the canvas.
     *
     *  Offset is the amount to translate the resulting image relative to the
     *  src when it is drawn. This is an out-param.
     *
     *  If the result image cannot be created (either because of error or if, say, the result
     *  is entirely clipped out), this should return nullptr.
     *  Callers that affect transparent black should explicitly handle nullptr
     *  results and press on. In the error case this behavior will produce a better result
     *  than nothing and is necessary for the clipped out case.
     *  If the return value is nullptr then offset should be ignored.
     */
    virtual sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* src, const Context&,
                                                SkIPoint* offset) const = 0;

    /**
     * This function recurses into its inputs with the given rect (first
     * argument), calls filterBounds() with the given map direction on each,
     * and returns the union of those results. If a derived class has special
     * recursion requirements (e.g., it has an input which does not participate
     * in bounds computation), it can be overridden here.
     * In kReverse mode, 'inputRect' is the device-space bounds of the input pixels. In kForward
     * mode it should always be null. If 'inputRect' is null in kReverse mode the resulting
     * answer may be incorrect.
     *
     * Note that this function is *not* responsible for mapping the rect for
     * this node's filter bounds requirements (i.e., calling
     * onFilterNodeBounds()); that is handled by filterBounds().
     */
    virtual SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                                   MapDirection, const SkIRect* inputRect) const;

    /**
     * Performs a forwards or reverse mapping of the given rect to accommodate
     * this filter's margin requirements. kForward_MapDirection is used to
     * determine the destination pixels which would be touched by filtering
     * the given source rect (e.g., given source bitmap bounds,
     * determine the optimal bounds of the filtered offscreen bitmap).
     * kReverse_MapDirection is used to determine which pixels of the
     * input(s) would be required to fill the given destination rect
     * (e.g., clip bounds). NOTE: these operations may not be the
     * inverse of the other. For example, blurring expands the given rect
     * in both forward and reverse directions. Unlike
     * onFilterBounds(), this function is non-recursive.
     * In kReverse mode, 'inputRect' will be the device space bounds of the input pixels. In
     * kForward mode, 'inputRect' should always be null. If 'inputRect' is null in kReverse mode
     * the resulting answer may be incorrect.
     */
    virtual SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                                       MapDirection, const SkIRect* inputRect) const;

    // Helper function which invokes filter processing on the input at the
    // specified "index". If the input is null, it returns "src" and leaves
    // "offset" untouched. If the input is non-null, it
    // calls filterImage() on that input, and returns the result.
    sk_sp<SkSpecialImage> filterInput(int index,
                                      SkSpecialImage* src,
                                      const Context&,
                                      SkIPoint* offset) const;

    /**
     *  Return true (and return a ref'd colorfilter) if this node in the DAG is just a
     *  colorfilter w/o CropRect constraints.
     */
    virtual bool onIsColorFilterNode(SkColorFilter** /*filterPtr*/) const {
        return false;
    }

    /**
     *  Override this to describe the behavior of your subclass - as a leaf node. The caller will
     *  take care of calling your inputs (and return false if any of them could not handle it).
     */
    virtual bool onCanHandleComplexCTM() const { return false; }

    /** Given a "srcBounds" rect, computes destination bounds for this filter.
     *  "dstBounds" are computed by transforming the crop rect by the context's
     *  CTM, applying it to the initial bounds, and intersecting the result with
     *  the context's clip bounds.  "srcBounds" (if non-null) are computed by
     *  intersecting the initial bounds with "dstBounds", to ensure that we never
     *  sample outside of the crop rect (this restriction may be relaxed in the
     *  future).
     */
    bool applyCropRect(const Context&, const SkIRect& srcBounds, SkIRect* dstBounds) const;

    /** A variant of the above call which takes the original source bitmap and
     *  source offset. If the resulting crop rect is not entirely contained by
     *  the source bitmap's bounds, it creates a new bitmap in "result" and
     *  pads the edges with transparent black. In that case, the srcOffset is
     *  modified to be the same as the bounds, since no further adjustment is
     *  needed by the caller. This version should only be used by filters
     *  which are not capable of processing a smaller source bitmap into a
     *  larger destination.
     */
    sk_sp<SkSpecialImage> applyCropRectAndPad(const Context&, SkSpecialImage* src,
                                              SkIPoint* srcOffset, SkIRect* bounds) const;

    /**
     *  Creates a modified Context for use when recursing up the image filter DAG.
     *  The clip bounds are adjusted to accommodate any margins that this
     *  filter requires by calling this node's
     *  onFilterNodeBounds(..., kReverse_MapDirection).
     */
    Context mapContext(const Context& ctx) const;

#if SK_SUPPORT_GPU
    /**
     *  Returns a version of the passed-in image (possibly the original), that is in a colorspace
     *  with the same gamut as the one from the OutputProperties. This allows filters that do many
     *  texture samples to guarantee that any color space conversion has happened before running.
     */
    static sk_sp<SkSpecialImage> ImageToColorSpace(SkSpecialImage* src, const OutputProperties&);
#endif

    sk_sp<SkImageFilter> refMe() const {
        return sk_ref_sp(const_cast<SkImageFilter*>(this));
    }

    // If 'srcBounds' will sample outside the border of 'originalSrcBounds' (i.e., the sample
    // will wrap around to the other side) we must preserve the far side of the src along that
    // axis (e.g., if we will sample beyond the left edge of the src, the right side must be
    // preserved for the repeat sampling to work).
    static SkIRect DetermineRepeatedSrcBound(const SkIRect& srcBounds,
                                             const SkIVector& filterOffset,
                                             const SkISize& filterSize,
                                             const SkIRect& originalSrcBounds);

private:
    friend class SkGraphics;
    friend bool SkIsSameFilter(const SkImageFilter* a, const SkImageFilter* b);

    static void PurgeCache();

    void init(sk_sp<SkImageFilter> const* inputs, int inputCount, const CropRect* cropRect);

    bool usesSrcInput() const { return fUsesSrcInput; }
    virtual bool affectsTransparentBlack() const { return false; }

    SkAutoSTArray<2, sk_sp<SkImageFilter>> fInputs;

    bool fUsesSrcInput;
    CropRect fCropRect;
    uint32_t fUniqueID; // Globally unique

    typedef SkFlattenable INHERITED;
};

#endif
