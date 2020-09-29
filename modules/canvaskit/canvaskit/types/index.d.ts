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
    RRectXY(rect: InputRect, rx: number, ry: number): SkRRect;

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
    MakeWebGLCanvasSurface(canvas: HTMLCanvasElement | string, colorSpace?: ColorSpace,
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
                          colorSpace: ColorSpace): SkSurface | null;

    /**
     * Returns the underlying data from SkData as a Uint8Array.
     * @param data
     */
    getSkDataBytes(data: SkData): Uint8Array;

    readonly AlphaType: AlphaTypeEnumValues;
    readonly BlendMode: BlendModeEnumValues;
    readonly ClipOp: ClipOpEnumValues;
    readonly ColorType: ColorTypeEnumValues;
    readonly ImageFormat: ImageFormatEnumValues;
    readonly PointMode: PointModeEnumValues;
    readonly SkColorSpace: ColorSpaceEnumValues;
    readonly TileMode: TileModeEnumValues;

    readonly TRANSPARENT: SkColor;
    readonly BLACK: SkColor;
    readonly WHITE: SkColor;
    readonly RED: SkColor;
    readonly GREEN: SkColor;
    readonly BLUE: SkColor;
    readonly YELLOW: SkColor;
    readonly CYAN: SkColor;
    readonly MAGENTA: SkColor;
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
 * See Paragraph.h for more information on this class. This is only available if Paragraph has
 * been compiled in.
 */
export interface Paragraph extends EmbindObject<SkAnimatedImage> {
    todo: number; // TODO(kjlubick)
}

/**
 * A simple wrapper around SkTextBlob and the simple Text Shaper.
 */
export interface ShapedText extends EmbindObject<SkAnimatedImage> {
    /**
     * Return the bounding area for the given text.
     * @param outputArray - if provided, the bounding box will be copied into this array instead of
     *                      allocating a new one.
     */
    getBounds(outputArray?: SkRect): SkRect;
}

/**
 * See SkAnimatedImage.h for more information on this class.
 */
export interface SkAnimatedImage extends EmbindObject<SkAnimatedImage> {
    todo: number; // TODO(kjlubick)
}

/**
 * See SkCanvas.h for more information on this class.
 */
export interface SkCanvas extends EmbindObject<SkCanvas> {
    /**
     * Fills the current clip with the given color using Src BlendMode.
     * This has the effect of replacing all pixels contained by clip with color.
     * @param color
     */
    clear(color: InputColor): void;

    /**
     * Replaces clip with the intersection or difference of the current clip and path,
     * with an aliased or anti-aliased clip edge.
     * @param path
     * @param op
     * @param doAntiAlias
     */
    clipPath(path: SkPath, op: ClipOp, doAntiAlias: boolean): void;

    /**
     * Replaces clip with the intersection or difference of the current clip and rect,
     * with an aliased or anti-aliased clip edge.
     * @param rect
     * @param op
     * @param doAntiAlias
     */
    clipRect(rect: InputRect, op: ClipOp, doAntiAlias: boolean): void;

    /**
     * Replaces clip with the intersection or difference of the current clip and rrect,
     * with an aliased or anti-aliased clip edge.
     * @param rrect
     * @param op
     * @param doAntiAlias
     */
    clipRRect(rrect: InputRRect, op: ClipOp, doAntiAlias: boolean): void;

    /**
     * Replaces current matrix with m premultiplied with the existing matrix.
     * @param m
     */
    concat(m: InputMatrix): void;

    /**
     * Draws arc using clip, SkMatrix, and SkPaint paint.
     *
     * Arc is part of oval bounded by oval, sweeping from startAngle to startAngle plus
     * sweepAngle. startAngle and sweepAngle are in degrees.
     * @param oval - bounds of oval containing arc to draw
     * @param startAngle - angle in degrees where arc begins
     * @param sweepAngle - sweep angle in degrees; positive is clockwise
     * @param useCenter - if true, include the center of the oval
     * @param paint
     */
    drawArc(oval: InputRect, startAngle: angleInDegrees, sweepAngle: angleInDegrees,
            useCenter: boolean, paint: SkPaint): void;

    /**
     * Draws a set of sprites from atlas, using clip, SkMatrix, and optional SkPaint paint.
     * @param atlas - SkImage containing sprites
     * @param srcRects - SkRect locations of sprites in atlas
     * @param dstXforms - SkRSXform mappings for sprites in atlas
     * @param paint
     * @param blendMode - BlendMode combining colors and sprites
     * @param colors - If provided, will be blended with sprite using blendMode.
     */
    drawAtlas(atlas: SkImage, srcRects: FlattenedRectangleArray,
              dstXforms: FlattenedRSXFormArray, paint: SkPaint,
              blendMode?: BlendMode, colors?: ColorIntArray): void;

    /**
     * Draws a circle at (cx, cy) with the given radius.
     * @param cx
     * @param cy
     * @param radius
     * @param paint
     */
    drawCircle(cx: number, cy: number, radius: number, paint: SkPaint): void;

    /**
     * Fills clip with the given color.
     * @param color
     * @param blendMode - defaults to SrcOver.
     */
    drawColor(color: InputColor, blendMode?: BlendMode): void;

    /**
     * Fills clip with the given color.
     * @param r - red value (typically from 0 to 1.0).
     * @param g - green value (typically from 0 to 1.0).
     * @param b - blue value (typically from 0 to 1.0).
     * @param a - alpha value, range 0 to 1.0 (1.0 is opaque).
     * @param blendMode - defaults to SrcOver.
     */
    drawColorComponents(r: number, g: number, b: number, a: number, blendMode?: BlendMode): void;

    /**
     * Fills clip with the given color.
     * @param color
     * @param blendMode - defaults to SrcOver.
     */
    drawColorInt(color: SkColorInt, blendMode?: BlendMode): void;

    /**
     * Draws SkRRect outer and inner using clip, SkMatrix, and SkPaint paint.
     * outer must contain inner or the drawing is undefined.
     * @param outer
     * @param inner
     * @param paint
     */
    drawDRRect(outer: InputRRect, inner: InputRRect, paint: SkPaint): void;

    /**
     * Draws the given image with its top-left corner at (left, top) using the current clip,
     * the current matrix, and optionally-provided paint.
     * @param img
     * @param left
     * @param top
     * @param paint
     */
    drawImage(img: SkImage, left: number, top: number, paint?: SkPaint): void;

    /**
     * Draws the current frame of the given animated image with its top-left corner at
     * (left, top) using the current clip, the current matrix, and optionally-provided paint.
     * @param aImg
     * @param left
     * @param top
     * @param paint
     */
    drawImageAtCurrentFrame(aImg: SkAnimatedImage, left: number, top: number,
                            paint?: SkPaint): void;

    /**
     *  Draws the provided image stretched proportionally to fit into dst rectangle.
     *  The center rectangle divides the image into nine sections: four sides, four corners, and
     *  the center.
     * @param img
     * @param center
     * @param dest
     * @param paint
     */
    drawImageNine(img: SkImage, center: InputIRect, dest: InputRect, paint: SkPaint): void;

    /**
     * Draws sub-rectangle src from provided image, scaled and translated to fill dst rectangle.
     * @param img
     * @param src
     * @param dest
     * @param paint
     * @param fastSample - if false, will filter strictly within src.
     */
    drawImageRect(img: SkImage, src: InputRect, dest: InputRect, paint: SkPaint,
                  fastSample?: boolean): void;

    /**
     * Draws line segment from (x0, y0) to (x1, y1) using the current clip, current matrix,
     * and the provided paint.
     * @param x0
     * @param y0
     * @param x1
     * @param y1
     * @param paint
     */
    drawLine(x0: number, y0: number, x1: number, y1: number, paint: SkPaint): void;

    /**
     * Draws an oval bounded by the given rectangle using the current clip, current matrix,
     * and the provided paint.
     * @param oval
     * @param paint
     */
    drawOval(oval: InputRect, paint: SkPaint): void;

    /**
     * Fills clip with the given paint.
     * @param paint
     */
    drawPaint(paint: SkPaint): void;

    /**
     * Draws the given Paragraph at the provided coordinates.
     * Requires the Paragraph code to be compiled in.
     * @param p
     * @param x
     * @param y
     */
    drawParagraph(p: Paragraph, x: number, y: number): void;

    /**
     * Draws the given path using the current clip, current matrix, and the provided paint.
     * @param path
     * @param paint
     */
    drawPath(path: SkPath, paint: SkPaint): void;

    /**
     * Draws the given picture using the current clip, current matrix, and the provided paint.
     * @param skp
     */
    drawPicture(skp: SkPicture): void;

    /**
     * Draws the given points using the current clip, current matrix, and the provided paint.
     *
     * See SkCanvas.h for more on the mode and its interaction with paint.
     * @param mode
     * @param points
     * @param paint
     */
    drawPoints(mode: PointMode, points: FlattenedPointArray, paint: SkPaint): void;

    /**
     * Draws the given rectangle using the current clip, current matrix, and the provided paint.
     * @param rect
     * @param paint
     */
    drawRect(rect: InputRect, paint: SkPaint): void;

    /**
     * Draws the given rectangle using the current clip, current matrix, and the provided paint.
     * @param left
     * @param top
     * @param right
     * @param bottom
     * @param paint
     */
    drawRect4f(left: number, top: number, right: number, bottom: number, paint: SkPaint): void;

    /**
     * Draws the given rectangle with rounded corners using the current clip, current matrix,
     * and the provided paint.
     * @param rrect
     * @param paint
     */
    drawRRect(rrect: InputRRect, paint: SkPaint): void;

    /**
     * Draw an offset spot shadow and outlining ambient shadow for the given path using a disc
     * light. See SkShadowUtils.h for more details
     * @param path - The occluder used to generate the shadows.
     * @param zPlaneParams - Values for the plane function which returns the Z offset of the
     *                       occluder from the canvas based on local x and y values (the current
     *                       matrix is not applied).
     * @param lightPos - The 3D position of the light relative to the canvas plane. This is
     *                   independent of the canvas's current matrix.
     * @param lightRadius - The radius of the disc light.
     * @param ambientColor - The color of the ambient shadow.
     * @param spotColor -  The color of the spot shadow.
     * @param flags - See SkShadowFlags.h; 0 means use default options.
     */
    drawShadow(path: SkPath, zPlaneParams: Point3, lightPos: Point3, lightRadius: number,
               ambientColor: InputColor, spotColor: InputColor, flags: number): void;

    /**
     * Draw the given text at the location (x, y) using the provided paint and font. If non-shaped
     * text is provided, the text will be drawn as is; no line-breaking, no ligatures, etc.
     * @param str - either a string or pre-shaped text. Unicode text is supported.
     * @param x
     * @param y
     * @param paint
     * @param font
     */
    drawText(str: string | ShapedText, x: number, y: number, paint: SkPaint, font: SkFont): void;

    /**
     * Draws the given TextBlob at (x, y) using the current clip, current matrix, and the
     * provided paint. Reminder that the fonts used to draw TextBlob are part of the blob.
     * @param blob
     * @param x
     * @param y
     * @param paint
     */
    drawTextBlob(blob: SkTextBlob, x: number, y: number, paint: SkPaint): void;

    /**
     * Draws the given vertices (a triangle mesh) using the current clip, current matrix, and the
     * provided paint.
     *  If paint contains an SkShader and vertices does not contain texCoords, the shader
     *  is mapped using the vertices' positions.
     *  If vertices colors are defined in vertices, and SkPaint paint contains SkShader,
     *  SkBlendMode mode combines vertices colors with SkShader.
     * @param verts
     * @param mode
     * @param paint
     */
    drawVertices(verts: SkVertices, mode: BlendMode, paint: SkPaint): void;

    /**
     * Returns the 4x4 matrix matching the given marker or null if there was none.
     * See also markCTM.
     * @param marker
     */
    findMarkedCTM(marker: string): Matrix4x4 | null;

    /**
     * Returns the current transform from local coordinates to the 'device', which for most
     * purposes means pixels.
     */
    getLocalToDevice(): Matrix4x4;

    /**
     * Returns the number of saved states, each containing: SkMatrix and clip.
     * Equals the number of save() calls less the number of restore() calls plus one.
     * The save count of a new canvas is one.
     */
    getSaveCount(): number;

    /**
     * Legacy version of getLocalToDevice(), which strips away any Z information, and
     * just returns a 3x3 version.
     */
    getTotalMatrix(): Matrix3x3;

    /**
     * Creates SkSurface matching info and props, and associates it with SkCanvas.
     * Returns null if no match found.
     * @param info
     */
    makeSurface(info: SkImageInfo): SkSurface | null;

    /**
     * Record a marker (provided by caller) for the current CTM. This does not change anything
     * about the ctm or clip, but does "name" this matrix value, so it can be referenced by
     * custom effects (who access it by specifying the same name).
     * See also findMarkedCTM.
     * @param marker
     */
    markCTM(marker: string): void;

    /**
     * Copies the given rectangle of pixels into a new Uint8Array and returns it. If alphaType,
     * colorType, and colorSpace are provided, those will describe the output format.
     * @param x
     * @param y
     * @param w
     * @param h
     * @param alphaType - defaults to Unpremul
     * @param colorType - defaults to RGBA_8888
     * @param colorSpace - defaults to SRGB
     * @param dstRowBytes
     */
    readPixels(x: number, y: number, w: number, h: number, alphaType?: AlphaType,
               colorType?: ColorType, colorSpace?: ColorSpace, dstRowBytes?: number): Uint8Array;

    /**
     * Removes changes to the current matrix and clip since SkCanvas state was
     * last saved. The state is removed from the stack.
     * Does nothing if the stack is empty.
     */
    restore(): void;

    /**
     * Restores state to a previous stack value.
     * @param saveCount
     */
    restoreToCount(saveCount: number): void;

    /**
     * Rotates the current matrix by the number of degrees.
     * @param rot - angle of rotation in degrees.
     * @param rx
     * @param ry
     */
    rotate(rot: angleInDegrees, rx: number, ry: number): void;

    /**
     * Saves the current matrix and clip and returns current height of the stack.
     */
    save(): number;

    /**
     * Saves SkMatrix and clip, and allocates a SkBitmap for subsequent drawing.
     * Calling restore() discards changes to SkMatrix and clip, and draws the SkBitmap.
     * It returns the height of the stack.
     * See SkCanvas.h for more.
     * @param paint
     * @param bounds
     */
    saveLayer(paint?: SkPaint, bounds?: InputRect): number;

    /**
     * Scales the current matrix by sx on the x-axis and sy on the y-axis.
     * @param sx
     * @param sy
     */
    scale(sx: number, sy: number): void;

    /**
     *  Skews SkMatrix by sx on the x-axis and sy on the y-axis. A positive value of sx
     *  skews the drawing right as y-axis values increase; a positive value of sy skews
     *  the drawing down as x-axis values increase.
     * @param sx
     * @param sy
     */
    skew(sx: number, sy: number): void;

    /**
     * Translates SkMatrix by dx along the x-axis and dy along the y-axis.
     * @param dx
     * @param dy
     */
    translate(dx: number, dy: number): void;

    /**
     * Writes the given rectangle of pixels to the provided coordinates. The source pixels
     * will be converted to the canvas's alphaType and colorType if they do not match.
     * @param pixels
     * @param srcWidth
     * @param srcHeight
     * @param destX
     * @param destY
     * @param alphaType - defaults to Unpremul
     * @param colorType - defaults to RGBA_8888
     * @param colorSpace - defaults to SRGB
     */
    writePixels(pixels: Uint8Array | number[], srcWidth: number, srcHeight: number,
                destX: number, destY: number, alphaType?: AlphaType, colorType?: ColorType,
                colorSpace?: ColorSpace): boolean;
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
 * See SkFont.h for more on this class.
 */
export interface SkFont extends EmbindObject<SkImage> {
    todo: number; // TODO(kjlubick)
}

/**
 * See SkImage.h for more information on this class.
 */
export interface SkImage extends EmbindObject<SkImage> {
    /**
     * Encodes this image's pixels to PNG and returns them. Must be built with the PNG codec.
     */
    encodeToData(): SkData;

    /**
     * Encodes this image's pixels to the specified format and returns them. Must be built with
     * the specified codec.
     * @param fmt
     * @param quality - a value from 0 to 100; 100 is the least lossy. May be ignored.
     */
    encodeToDataWithFormat(fmt: EncodedImageFormat, quality: number): SkData;

    /**
     * Return the height in pixels of the image.
     */
    height(): number;

    /**
     * Returns this image as a shader with the specified tiling.
     * @param tx - tile mode in the x direction.
     * @param ty - tile mode in the y direction.
     * @param localMatrix
     */
    makeShader(tx: TileMode, ty: TileMode, localMatrix?: InputMatrix): SkShader;

    /**
     * Returns a TypedArray containing the pixels reading starting at (srcX, srcY) and does not
     * exceed the size indicated by imageInfo. See SkImage.h for more on the caveats.
     *
     * @param imageInfo - describes the destination format of the pixels.
     * @param srcX
     * @param srcY
     * @returns a Uint8Array if RGB_8888 was requested, Float32Array if RGBA_F32 was requested.
     */
    readPixels(imageInfo: SkImageInfo, srcX: number, srcY: number): Uint8Array | Float32Array | null;

    /**
     * Return the width in pixels of the image.
     */
    width(): number;
}

export interface SkImageInfo {
    alphaType: AlphaType;
    colorSpace: ColorSpace;
    colorType: ColorType;
    height: number;
    width: number;
}

/**
 * See SkPaint.h for more information on this class.
 */
export interface SkPaint extends EmbindObject<SkPaint> {
    todo: number; // TODO(kjlubick)
}

/**
 * See SkPath.h for more information on this class.
 */
export interface SkPath extends EmbindObject<SkPath> {
    todo: number; // TODO(kjlubick)
}

/**
 * See SkPicture.h for more information on this class.
 *
 * Of note, SkPicture is *not* what is colloquially thought of as a "picture" (what we
 * call a bitmap). An SkPicture is a series of draw commands.
 */
export interface SkPicture extends EmbindObject<SkPicture> {
    /**
     * Returns the serialized format of this SkPicture. The format may change at anytime and
     * no promises are made for backwards or forward compatibility.
     */
    serialize(): SkData;
}

export interface SkShader extends EmbindObject<SkShader> {
    todo: number; // TODO(kjlubick)
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
 * See SkTextBlob.h for more on this class.
 */
export interface SkTextBlob extends EmbindObject<SkImage> {
    todo: number; // TODO(kjlubick)
}

/**
 * See SkVertices.h for more on this class.
 */
export interface SkVertices extends EmbindObject<SkImage> {
    todo: number; // TODO(kjlubick)
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
export type angleInDegrees = number;

export type TypedArrayConstructor = Float32ArrayConstructor | Int32ArrayConstructor |
    Int16ArrayConstructor | Int8ArrayConstructor | Uint32ArrayConstructor |
    Uint16ArrayConstructor | Uint8ArrayConstructor;
export type TypedArray = Float32Array | Int32Array | Int16Array | Int8Array | Uint32Array |
    Uint16Array | Uint8Array;
/**
 * FlattenedPointArray represents n points by 2*n float values. In order, the values should
 * be the x, y for each point.
 */
export type FlattenedPointArray = MallocObj | Float32Array | number[];
/**
 * FlattenedRectangleArray represents n rectangles by 4*n float values. In order, the values should
 * be the top, left, right, bottom point for each rectangle.
 */
export type FlattenedRectangleArray = MallocObj | Float32Array | number[];
/**
 * FlattenedRSXFormArray represents n RSXforms by 4*n float values. In order, the values should
 * be scos, ssin, tx, ty for each RSXForm. See RSXForm.h for more details.
 */
export type FlattenedRSXFormArray = MallocObj | Float32Array | number[];
export type ColorIntArray = MallocObj | Uint32Array | number[];

export type Matrix4x4 = Float32Array;
export type Matrix3x3 = Float32Array;
export type Matrix3x2 = Float32Array;
/**
 * Point3 represents an x, y, coordinate.
 */
export type Point3 = number[]; // TODO(kjlubick) make this include typed array and malloc'd.

/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as colors.
 */
export type InputColor = MallocObj | SkColor | number[];
/**
 * CanvasKit APIs accept all of these matrix types. Under the hood, we generally use 4x4 matrices.
 */
export type InputMatrix = MallocObj | Matrix4x4 | Matrix3x3 | Matrix3x2 | DOMMatrix | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as rectangles.
 */
export type InputRect = MallocObj | SkRect | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as rectangles.
 */
export type InputIRect = MallocObj | SkRect | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as rectangles with
 * rounded corners.
 */
export type InputRRect = MallocObj | SkRRect | number[];

export type AlphaType = EmbindEnumEntity;
export type BlendMode = EmbindEnumEntity;
export type ClipOp = EmbindEnumEntity;
export type ColorSpace = EmbindSingleton;
export type ColorType = EmbindEnumEntity;
export type EncodedImageFormat = EmbindEnumEntity;
export type PointMode = EmbindEnumEntity;
export type TileMode = EmbindEnumEntity;

export interface AlphaTypeEnumValues extends EmbindEnum {
    Opaque: AlphaType;
    Premul: AlphaType;
    Unpremul: AlphaType;
}

export interface BlendModeEnumValues extends EmbindEnum {
    Clear: BlendMode;
    Src: BlendMode;
    Dst: BlendMode;
    SrcOver: BlendMode;
    DstOver: BlendMode;
    SrcIn: BlendMode;
    DstIn: BlendMode;
    SrcOut: BlendMode;
    DstOut: BlendMode;
    SrcATop: BlendMode;
    DstATop: BlendMode;
    Xor: BlendMode;
    Plus: BlendMode;
    Modulate: BlendMode;
    Screen: BlendMode;
    Overlay: BlendMode;
    Darken: BlendMode;
    Lighten: BlendMode;
    ColorDodge: BlendMode;
    ColorBurn: BlendMode;
    HardLight: BlendMode;
    SoftLight: BlendMode;
    Difference: BlendMode;
    Exclusion: BlendMode;
    Multiply: BlendMode;
    Hue: BlendMode;
    Saturation: BlendMode;
    Color: BlendMode;
    Luminosity: BlendMode;
}

export interface ClipOpEnumValues extends EmbindEnum {
    Difference: ClipOp;
    Intersect: ClipOp;
}

/**
 * The currently supported color spaces. These are all singleton values.
 */
export interface ColorSpaceEnumValues { // not a typical enum, but effectively like one.
    readonly SRGB: ColorSpace;
    readonly DISPLAY_P3: ColorSpace;
    readonly ADOBE_RGB: ColorSpace;
}

export interface ColorTypeEnumValues extends EmbindEnum {
    Alpha_8: ColorType;
    RGB_565: ColorType;
    RGBA_8888: ColorType;
    BGRA_8888: ColorType;
    RGBA_1010102: ColorType;
    RGB_101010x: ColorType;
    Gray_8: ColorType;
    RGBA_F16: ColorType;
    RGBA_F32: ColorType;
}

export interface ImageFormatEnumValues extends EmbindEnum {
    // TODO(kjlubick) When these are compiled in depending on the availability of the codecs,
    PNG: EncodedImageFormat;
    JPEG: EncodedImageFormat;
    WEBP: EncodedImageFormat;
}

export interface PointModeEnumValues extends EmbindEnum {
    Points: PointMode;
    Lines: PointMode;
    Polygon: PointMode;
}

export interface TileModeEnumValues extends EmbindEnum {
    Clamp: TileMode;
    Decal: TileMode;
    Mirror: TileMode;
    Repeat: TileMode;
}
