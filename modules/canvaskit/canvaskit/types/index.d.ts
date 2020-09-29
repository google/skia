export interface CanvasKitInitOptions {
    locateFile: (path: string) => string;
}

export interface CanvasKit {
    // Helpers
    /**
     * Constructs a Color with the same API as CSS's rgba(), that is
     * Internally, Colors are four unpremultiplied 32-bit floats: r, g, b, a.
     * In order to construct one with more precision or in a wider gamut,
     * use CanvasKit.Color4f().
     *
     * @param r - red value, clamped to [0, 255].
     * @param g - green value, clamped to [0, 255].
     * @param b - blue value, clamped to [0, 255].
     * @param a - alpha value, from 0 to 1.0. By default is 1.0 (opaque).
     */
    Color(r: number, g: number, b: number, a?: number): SkColor;

    /**
     * Construct a 4-float color. Float values are typically between 0.0 and 1.0.
     * @param r - red value.
     * @param g - green value.
     * @param b - blue value.
     * @param a - alpha value. By default is 1.0 (opaque).
     */
    Color4f(r: number, g: number, b: number, a?: number): SkColor;

    /**
     * Constructs a Color as a 32 bit unsigned integer, with 8 bits assigned to each channel.
     * Channels are expected to be between 0 and 255 and will be clamped as such.
     * If a is omitted, it will be 255 (opaque).
     *
     * This is not the preferred way to use colors in Skia APIs, use Color or Color4f.
     * @param r - red value, clamped to [0, 255].
     * @param g - green value, clamped to [0, 255].
     * @param b - blue value, clamped to [0, 255].
     * @param a - alpha value, from 0 to 1.0. By default is 1.0 (opaque).
     */
    ColorAsInt(r: number, g: number, b: number, a?: number): SkColorInt;

    /**
     * Returns a css style [r, g, b, a] where r, g, b are returned as
     * ints in the range [0, 255] and where a is scaled between 0 and 1.0.
     * [Deprecated] - this is trivial now that SkColor is 4 floats.
     */
    getColorComponents(c: SkColor): number[];

    /**
     * Takes in a CSS color value and returns a CanvasKit.Color
     * (which is an array of 4 floats in RGBA order). An optional colorMap
     * may be provided which maps custom strings to values.
     * In the CanvasKit canvas2d shim layer, we provide this map for processing
     * canvas2d calls, but not here for code size reasons.
     */
    parseColorString(color: string, colorMap?: object): SkColor;

    /**
     * Returns a copy of the passed in color with a new alpha value applied.
     * [Deprecated] - this is trivial now that SkColor is 4 floats.
     */
    multiplyByAlpha(c: SkColor, alpha: number): SkColor;

    /**
     * Returns a rectangle with the given paramaters. See SkRect.h for more.
     * @param left - The x coordinate of the upper-left corner.
     * @param top  - The y coordinate of the upper-left corner.
     * @param right - The x coordinate of the lower-right corner.
     * @param bottom - The y coordinate of the lower-right corner.
     */
    LTRBRect(left: number, top: number, right: number, bottom: number): SkRect;

    /**
     * Returns a rectangle with the given paramaters. See SkRect.h for more.
     * @param x - The x coordinate of the upper-left corner.
     * @param y  - The y coordinate of the upper-left corner.
     * @param width - The width of the rectangle.
     * @param height - The height of the rectangle.
     */
    XYWHRect(x: number, y: number, width: number, height: number): SkRect;

    /**
     * Returns a rectangle with the given integer paramaters. See SkRect.h for more.
     * @param left - The x coordinate of the upper-left corner.
     * @param top  - The y coordinate of the upper-left corner.
     * @param right - The x coordinate of the lower-right corner.
     * @param bottom - The y coordinate of the lower-right corner.
     */
    LTRBiRect(left: number, top: number, right: number, bottom: number): SkIRect;

    /**
     * Returns a rectangle with the given paramaters. See SkRect.h for more.
     * @param x - The x coordinate of the upper-left corner.
     * @param y  - The y coordinate of the upper-left corner.
     * @param width - The width of the rectangle.
     * @param height - The height of the rectangle.
     */
    XYWHiRect(x: number, y: number, width: number, height: number): SkIRect;

    /**
     * Returns a rectangle with rounded corners consisting of the given rectangle and
     * the same radiusX and radiusY for all four corners.
     * @param rect - The base rectangle.
     * @param rx - The radius of the corners in the x direction.
     * @param ry - The radius of the corners in the y direction.
     */
    RRectXY(rect: SkRect, rx: number, ry: number): SkRRect;

    /**
     * Malloc returns a TypedArray backed by the C++ memory of the
     * given length. It should only be used by advanced users who
     * can manage memory and initialize values properly. When used
     * correctly, it can save copying of data between JS and C++.
     * When used incorrectly, it can lead to memory leaks.
     * Any memory allocated by CanvasKit.Malloc needs to be released with CanvasKit.Free.
     *
     * const mObj = CanvasKit.Malloc(Float32Array, 20);
     * Get a TypedArray view around the malloc'd memory (this does not copy anything).
     * const ta = mObj.toTypedArray();
     * // store data into ta
     * const cf = CanvasKit.SkColorFilter.MakeMatrix(ta); // mObj could also be used.
     *
     * // eventually...
     * CanvasKit.Free(mObj);
     *
     * @param typedArray - constructor for the typedArray.
     * @param len - number of *elements* to store.
     */
    Malloc(typedArray: TypedArrayConstructor, len: number): MallocObj;

    /**
     * Free frees the memory returned by Malloc.
     * Any memory allocated by CanvasKit.Malloc needs to be released with CanvasKit.Free.
     */
    Free(m: MallocObj): void;

    // TODO(kjlubick) in helper.js
    //   - SkRectBuilder
    //   - RSXFormBuilder
    //   - SkColorBuilder

    // Surface related functions
    /**
     * Creates a Surface on a given canvas. If both GPU and CPU modes have been compiled in, this
     * will first try to create a GPU surface and then fallback to a CPU one if that fails. If just
     * the CPU mode has been compiled in, a CPU surface will be created.
     * @param canvas - either the canvas element itself or a string with the DOM id of it.
     */
    MakeCanvasSurface(canvas: HTMLCanvasElement | string): SkSurface | null;

    /**
     * Creates a CPU backed (aka raster) surface.
     * @param canvas - either the canvas element itself or a string with the DOM id of it.
     */
    MakeSWCanvasSurface(canvas: HTMLCanvasElement | string): SkSurface | null;

    /**
     * A helper for creating a WebGL backed (aka GPU) surface and falling back to a CPU surface if
     * the GPU one cannot be created. This works for both WebGL 1 and WebGL 2.
     * @param canvas - Either the canvas element itself or a string with the DOM id of it.
     * @param colorSpace - One of the supported color spaces. Default is SRGB.
     * @param opts - Options that will get passed to the creation of the WebGL context.
     */
    MakeWebGLCanvasSurface(canvas: HTMLCanvasElement | string, colorSpace?: SkColorSpace,
                           opts?: WebGLOptions): SkSurface | null;

    /**
     * Returns a CPU backed surface with the given dimensions, an SRGB colorspace, Unpremul
     * alphaType and 8888 color type. The pixels belonging to this surface  will be in memory and
     * not visible.
     * @param width - number of pixels of the width of the drawable area.
     * @param height - number of pixels of the height of the drawable area.
     */
    MakeSurface(width: number, height: number): SkSurface | null;

    /**
     * Creates a WebGL Context from the given canvas with the given options. If options are omitted,
     * sensible defaults will be used.
     * @param canvas
     * @param opts
     */
    GetWebGLContext(canvas: HTMLCanvasElement, opts?: WebGLOptions): WebGLContextHandle;

    /**
     * Creates a GrContext from the given WebGL Context.
     * @param ctx
     */
    MakeGrContext(ctx: WebGLContextHandle): GrContext;

    /**
     * Creates a Surface that will be drawn to the given GrContext (and show up on screen).
     * @param ctx
     * @param width - number of pixels of the width of the visible area.
     * @param height - number of pixels of the height of the visible area.
     * @param colorSpace
     */
    MakeOnScreenGLSurface(ctx: GrContext, width: number, height: number,
                          colorSpace: SkColorSpace): SkSurface | null;

    readonly SkColorSpace: SkColorSpaceFactory;
}

/**
 * CanvasKit is built with Emscripten and Embind. Embind adds the following methods to all objects
 * that are exposed with it.
 */
export interface EmbindObject<T extends EmbindObject<T>> {
    clone(): T;
    delete(): void;
    deleteAfter(): void;
    isAliasOf(other: any): boolean;
    isDeleted(): boolean;
}

export interface EmbindSingleton {
    // Technically Embind includes the other methods too, but they should not be called for a
    // singleton.
    isAliasOf(other: any): boolean;
}

/**
 * Represents the set of enum values.
 */
export interface EmbindEnum {
    readonly values: number[];
}

/**
 * Represents a single member of an enum.
 */
export interface EmbindEnumEntity {
    readonly value: number;
}

/**
 * See GrContext.h for more on this class.
 */
export interface GrContext extends EmbindObject<GrContext> {
    getResourceCacheLimitBytes(): number;
    getResourceCacheUsageBytes(): number;
    releaseResourcesAndAbandonContext(): void;
    setResourceCacheLimitBytes(bytes: number): void;
}

/**
 * This object is a wrapper around a pointer to some memory on the WASM heap. The type of the
 * pointer was determined at creation time.
 */
export interface MallocObj {
    /**
     * The number of objects this pointer refers to.
     */
    readonly length: number;
    /**
     * The "pointer" into the WASM memory. Should be fixed over the lifetime of the object.
     */
    readonly byteOffset: number;
    /**
     * Return a read/write view into a subset of the memory. Do not cache the TypedArray this
     * returns, it may be invalidated if the WASM heap is resized. This is the same as calling
     * .toTypedArray().subarray() except the returned TypedArray can also be passed into an API
     * and not cause an additional copy.
     */
    subarray: (start: number, end: number) => TypedArray;
    /**
     * Return a read/write view of the memory. Do not cache the TypedArray this returns, it may be
     * invalidated if the WASM heap is resized. If this TypedArray is passed into a CanvasKit API,
     * it will not be copied again, only the pointer will be re-used.
     */
    toTypedArray: () => TypedArray;
}

/**
 * See SkCanvas.h for more information on this class.
 */
export interface SkCanvas extends EmbindObject<SkCanvas> {
    // TODO(kjlubick) Fill out the rest
    /**
     * Fills the current clip with the given color using Src BlendMode.
     * This has the effect of replacing all pixels contained by clip with color.
     * @param color
     */
    clear(color: SkColor): void;
}

export type SkColorSpace = EmbindSingleton;

/**
 * The currently supported color spaces. These are all singleton values.
 */
export interface SkColorSpaceFactory {
    readonly SRGB: SkColorSpace;
    readonly DISPLAY_P3: SkColorSpace;
    readonly ADOBE_RGB: SkColorSpace;
}

/**
 * Represents a blob of memory. See SkData.h for more on this class.
 */
export interface SkData extends EmbindObject<SkImage> {
    /**
     * Return the number of bytes in this container.
     */
    size(): number;
}
/**
 * See SkImage.h for more information on this class.
 */
export interface SkImage extends EmbindObject<SkImage> {
    // TODO(kjlubick)
    /**
     * Return the height in pixels of the image.
     */
    height(): number;
}

export interface SkImageInfo {
    alphaType: SkAlphaType;
    colorSpace: SkColorSpace;
    colorType: SkColorType;
    height: number;
    width: number;
}

/**
 * See SkPicture.h for more information on this class.
 */
export interface SkPicture extends EmbindObject<SkPicture> {
    /**
     * Returns the serialized format of this SkPicture. The format may change at anytime and
     * no promises are made for backwards or forward compatibility.
     */
    serialize(): SkData;
}

export interface SkSurface extends EmbindObject<SkSurface> {
    /**
     * Call the given callback and save the result of that draw to a SkPicture with the
     * same dimensions as this surface. The SkPicture will be returned.
     * @param drawFrame - callback in which the client should draw something.
     */
    captureFrameAsSkPicture(drawFrame: (canvas: SkCanvas) => void): SkPicture;

    /**
     * Clean up the surface and any extra memory.
     * [Deprecated]: In the future, calls to delete() will be sufficient to clean up the memory.
     */
    dispose(): void;

    /**
     * Make sure any queued draws are sent to the screen or the GPU.
     */
    flush(): void;

    /**
     * Return a canvas that is backed by this surface. Any draws to the canvas will (eventually)
     * show up on the surface. The returned canvas is owned by the surface and does NOT need to
     * be cleaned up by the client.
     */
    getCanvas(): SkCanvas;

    /**
     * Returns the height of this surface in pixels.
     */
    height(): number;

    /**
     * Returns the ImageInfo associated with this surface.
     */
    imageInfo(): SkImageInfo;

    /**
     * Returns current contents of the surface as an SkImage. This image will be optimized to be
     * drawn to another surface of the same type. For example, if this surface is backed by the
     * GPU, the returned SkImage will be backed by a GPU texture.
     */
    makeImageSnapshot(bounds?: SkIRect | number[]): SkImage;

    /**
     * Returns a compatible SkSurface, haring the same raster or GPU properties of the original.
     * The pixels are not shared.
     * @param info - width, height, etc of the SkSurface.
     */
    makeSurface(info: SkImageInfo): SkSurface;

    /**
     * Returns if this Surface is a GPU-backed surface or not.
     */
    reportBackendTypeIsGPU(): boolean;

    /**
     * If this surface is GPU-backed, return the sample count of the surface.
     */
    sampleCnt(): number;

    /**
     * Returns the width of this surface in pixels.
     */
    width(): number;
}

/**
 * Options for configuring a WebGL context. If an option is omitted, a sensible default will
 * be used. These are defined by the WebGL standards.
 */
export interface WebGLOptions {
    alpha?: number;
    antialias?: number;
    depth?: number;
    enableExtensionsByDefault?: number;
    explicitSwapControl?: number;
    failIfMajorPerformanceCaveat?: number;
    majorVersion?: number;
    minorVersion?: number;
    preferLowPowerToHighPerformance?: number;
    premultipliedAlpha?: number;
    preserveDrawingBuffer?: number;
    renderViaOffscreenBackBuffer?: number;
    stencil?: number;
}

/**
 * An SkColor is represented by 4 floats, typically with values between 0 and 1.0. In order,
 * the floats correspond to red, green, blue, alpha.
 */
export type SkColor = Float32Array;
export type SkColorInt = number; // deprecated, prefer SkColor
/**
 * An SkIRect is represented by 4 ints. In order, the ints correspond to left, top,
 * right, bottom. See SkRect.h for more
 */
export type SkIRect = Int32Array;
/**
 * An SkRect is represented by 4 floats. In order, the floats correspond to left, top,
 * right, bottom. See SkRect.h for more
 */
export type SkRect = Float32Array;
/**
 * An SkRRect (rectangle with rounded corners) is represented by 12 floats. In order, the floats
 * correspond to left, top, right, bottom and then in pairs, the radiusX, radiusY for upper-left,
 * upper-right, lower-right, lower-left. See SkRRect.h for more.
 */
export type SkRRect = Float32Array;
export type WebGLContextHandle = number;

export type TypedArrayConstructor = Float32ArrayConstructor | Int32ArrayConstructor |
    Int16ArrayConstructor | Int8ArrayConstructor | Uint32ArrayConstructor |
    Uint16ArrayConstructor | Uint8ArrayConstructor;
export type TypedArray = Float32Array | Int32Array | Int16Array | Int8Array | Uint32Array |
    Uint16Array | Uint8Array;

export type SkAlphaType = EmbindEnumEntity;
export type SkColorType = EmbindEnumEntity;
