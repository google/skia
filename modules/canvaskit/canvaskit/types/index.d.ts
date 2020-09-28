export interface CanvasKitInitOptions {
    locateFile: (path: string) => string;
}

export interface CanvasKit {
    // Helpers

    /** Constructs a Color with the same API as CSS's rgba(), that is
     * r, g, b are 0-255, and a is 0.0 to 1.0.
     * if a is omitted, it will be assumed to be 1.0 (opaque).
     * Internally, Colors are a TypedArray of four unpremultiplied 32-bit floats: a, r, g, b
     * In order to construct one with more precision or in a wider gamut,
     * use CanvasKit.Color4f
     */
    Color(r: number, g: number, b: number, a?: number): SkColor;

    /** Construct a 4-float color. Float values are typically between 0.0 and 1.0
     * Opaque if alpha is omitted.
     */
    Color4f(r: number, g: number, b: number, a?: number): SkColor;

    /** Constructs a Color as a 32 bit unsigned integer, with 8 bits assigned to each channel.
     * Channels are expected to be between 0 and 255 and will be clamped as such.
     * If a is omitted, it will be 255 (opaque).
     *
     * This is not the preferred way to use colors in Skia APIs, use Color or Color4f.
     */
    ColorAsInt(r: number, g: number, b: number, a?: number): SkColorInt;

    /** Returns a css style [r, g, b, a] where r, g, b are returned as
     * ints in the range [0, 255] and where a is scaled between 0 and 1.0.
     */
    getColorComponents(c: SkColor): number[];

    /** parseColorString takes in a CSS color value and returns a CanvasKit.Color
     * (which is an array of 4 floats in RGBA order). An optional colorMap
     * may be provided which maps custom strings to values.
     * In the CanvasKit canvas2d shim layer, we provide this map for processing
     * canvas2d calls, but not here for code size reasons.
     */
    parseColorString(color: string, colorMap?: object): SkColor;

    /** Returns a copy of the passed in color with a new alpha value applied.
     * [Deprecated] - this is trivial now that SkColor is 4 floats.
     */
    multiplyByAlpha(c: SkColor, alpha: number): SkColor;

    LTRBRect(left: number, top: number, right: number, bottom: number): SkRect;
    RRectXY(rect: SkRect, rx: number, ry: number): SkRRect;
    XYWHRect(x: number, y: number, width: number, height: number): SkRect;

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
     * @param {TypedArray} typedArray - constructor for the typedArray.
     * @param {number} len - number of *elements* to store.
     */
    Malloc(typedArray: TypedArray, len: number): MallocObj;

    /**
     * Free frees the memory returned by Malloc.
     * Any memory allocated by CanvasKit.Malloc needs to be released with CanvasKit.Free.
     */
    Free(m: MallocObj): void;

    // TODO(kjlubick) in helper.js
    //   - SkRectBuilder
    //   - RSXFormBuilder
    //   - SkColorBuilder
}

/**
 * This object is a wrapper around a pointer to some memory on the WASM heap. The type of the
 * pointer was determined at creation time.
 */
export interface MallocObj {
    /**
     * The number of objects this pointer refers to.
     */
    length: number;
    byteOffset: number;
    /**
     * Return a read/write view into a subset of the memory. Do not cache the TypedArray this
     * returns, it may be invalidated if the WASM heap is resized. This is the same as calling
     * .toTypedArray().subarray() except the returned TypedArray can also be passed into an API
     * and not cause an additional copy.
     */
    subArray: (start: number, end: number) => TypedArray;
    /**
     * Return a read/write view of the memory. Do not cache the TypedArray this returns, it may be
     * invalidated if the WASM heap is resized. If this TypedArray is passed into a CanvasKit API,
     * it will not be copied again, only the pointer will be re-used.
     */
    toTypedArray: () => TypedArray;
}

export type SkColor = Float32Array;
export type SkColorInt = number; // deprecated, prefer SkColor

export type TypedArray = Float32Array | Int32Array | Int16Array | Int8Array | Uint32Array | Uint16Array | Uint8Array;
