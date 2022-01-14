// Minimum TypeScript Version: 3.7
export function CanvasKitInit(opts: CanvasKitInitOptions): Promise<CanvasKit>;

export interface CanvasKitInitOptions {
    /**
     * This callback will be invoked when the CanvasKit loader needs to fetch a file (e.g.
     * the blob of WASM code). The correct url prefix should be applied.
     * @param file - the name of the file that is about to be loaded.
     */
    locateFile(file: string): string;
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
    Color(r: number, g: number, b: number, a?: number): Color;

    /**
     * Construct a 4-float color. Float values are typically between 0.0 and 1.0.
     * @param r - red value.
     * @param g - green value.
     * @param b - blue value.
     * @param a - alpha value. By default is 1.0 (opaque).
     */
    Color4f(r: number, g: number, b: number, a?: number): Color;

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
    ColorAsInt(r: number, g: number, b: number, a?: number): ColorInt;

    /**
     * Returns a css style [r, g, b, a] where r, g, b are returned as
     * ints in the range [0, 255] and where a is scaled between 0 and 1.0.
     * [Deprecated] - this is trivial now that Color is 4 floats.
     */
    getColorComponents(c: Color): number[];

    /**
     * Takes in a CSS color value and returns a CanvasKit.Color
     * (which is an array of 4 floats in RGBA order). An optional colorMap
     * may be provided which maps custom strings to values.
     * In the CanvasKit canvas2d shim layer, we provide this map for processing
     * canvas2d calls, but not here for code size reasons.
     */
    parseColorString(color: string, colorMap?: Record<string, Color>): Color;

    /**
     * Returns a copy of the passed in color with a new alpha value applied.
     * [Deprecated] - this is trivial now that Color is 4 floats.
     */
    multiplyByAlpha(c: Color, alpha: number): Color;

    /**
     * Computes color values for one-pass tonal alpha.
     * Note, if malloced colors are passed in, the memory pointed at by the MallocObj
     * will be overwritten with the computed tonal colors (and thus the return val can be
     * ignored).
     * @param colors
     */
    computeTonalColors(colors: TonalColorsInput): TonalColorsOutput;

    /**
     * Returns a rectangle with the given paramaters. See Rect.h for more.
     * @param left - The x coordinate of the upper-left corner.
     * @param top  - The y coordinate of the upper-left corner.
     * @param right - The x coordinate of the lower-right corner.
     * @param bottom - The y coordinate of the lower-right corner.
     */
    LTRBRect(left: number, top: number, right: number, bottom: number): Rect;

    /**
     * Returns a rectangle with the given paramaters. See Rect.h for more.
     * @param x - The x coordinate of the upper-left corner.
     * @param y  - The y coordinate of the upper-left corner.
     * @param width - The width of the rectangle.
     * @param height - The height of the rectangle.
     */
    XYWHRect(x: number, y: number, width: number, height: number): Rect;

    /**
     * Returns a rectangle with the given integer paramaters. See Rect.h for more.
     * @param left - The x coordinate of the upper-left corner.
     * @param top  - The y coordinate of the upper-left corner.
     * @param right - The x coordinate of the lower-right corner.
     * @param bottom - The y coordinate of the lower-right corner.
     */
    LTRBiRect(left: number, top: number, right: number, bottom: number): IRect;

    /**
     * Returns a rectangle with the given paramaters. See Rect.h for more.
     * @param x - The x coordinate of the upper-left corner.
     * @param y  - The y coordinate of the upper-left corner.
     * @param width - The width of the rectangle.
     * @param height - The height of the rectangle.
     */
    XYWHiRect(x: number, y: number, width: number, height: number): IRect;

    /**
     * Returns a rectangle with rounded corners consisting of the given rectangle and
     * the same radiusX and radiusY for all four corners.
     * @param rect - The base rectangle.
     * @param rx - The radius of the corners in the x direction.
     * @param ry - The radius of the corners in the y direction.
     */
    RRectXY(rect: InputRect, rx: number, ry: number): RRect;

    /**
     * Generate bounding box for shadows relative to path. Includes both the ambient and spot
     * shadow bounds. This pairs with Canvas.drawShadow().
     * See SkShadowUtils.h for more details.
     * @param ctm - Current transformation matrix to device space.
     * @param path - The occluder used to generate the shadows.
     * @param zPlaneParams - Values for the plane function which returns the Z offset of the
     *                       occluder from the canvas based on local x and y values (the current
     *                       matrix is not applied).
     * @param lightPos - The 3D position of the light relative to the canvas plane. This is
     *                   independent of the canvas's current matrix.
     * @param lightRadius - The radius of the disc light.
     * @param flags - See SkShadowFlags.h; 0 means use default options.
     * @param dstRect - if provided, the bounds will be copied into this rect instead of allocating
     *                  a new one.
     * @returns The bounding rectangle or null if it could not be computed.
     */
    getShadowLocalBounds(ctm: InputMatrix, path: Path, zPlaneParams: InputVector3,
                         lightPos: InputVector3, lightRadius: number, flags: number,
                         dstRect?: Rect): Rect | null;

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
     * const cf = CanvasKit.ColorFilter.MakeMatrix(ta); // mObj could also be used.
     *
     * // eventually...
     * CanvasKit.Free(mObj);
     *
     * @param typedArray - constructor for the typedArray.
     * @param len - number of *elements* to store.
     */
    Malloc(typedArray: TypedArrayConstructor, len: number): MallocObj;

    /**
     * As Malloc but for GlyphIDs. This helper exists to make sure the JS side and the C++ side
     * stay in agreement with how wide GlyphIDs are.
     * @param len - number of GlyphIDs to make space for.
     */
    MallocGlyphIDs(len: number): MallocObj;

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
    MakeCanvasSurface(canvas: HTMLCanvasElement | string): Surface | null;

    /**
     * Creates a Raster (CPU) Surface that will draw into the provided Malloc'd buffer. This allows
     * clients to efficiently be able to read the current pixels w/o having to copy.
     * The length of pixels must be at least height * bytesPerRow bytes big.
     * @param ii
     * @param pixels
     * @param bytesPerRow - How many bytes are per row. This is at least width * bytesPerColorType. For example,
     *                      an 8888 ColorType has 4 bytes per pixel, so a 5 pixel wide 8888 surface needs at least
     *                      5 * 4 = 20 bytesPerRow. Some clients may have more than the usual to make the data line
     *                      up with a particular multiple.
     */
    MakeRasterDirectSurface(ii: ImageInfo, pixels: MallocObj, bytesPerRow: number): Surface | null;

    /**
     * Creates a CPU backed (aka raster) surface.
     * @param canvas - either the canvas element itself or a string with the DOM id of it.
     */
    MakeSWCanvasSurface(canvas: HTMLCanvasElement | string): Surface | null;

    /**
     * A helper for creating a WebGL backed (aka GPU) surface and falling back to a CPU surface if
     * the GPU one cannot be created. This works for both WebGL 1 and WebGL 2.
     * @param canvas - Either the canvas element itself or a string with the DOM id of it.
     * @param colorSpace - One of the supported color spaces. Default is SRGB.
     * @param opts - Options that will get passed to the creation of the WebGL context.
     */
    MakeWebGLCanvasSurface(canvas: HTMLCanvasElement | string, colorSpace?: ColorSpace,
                           opts?: WebGLOptions): Surface | null;

    /**
     * Returns a CPU backed surface with the given dimensions, an SRGB colorspace, Unpremul
     * alphaType and 8888 color type. The pixels belonging to this surface  will be in memory and
     * not visible.
     * @param width - number of pixels of the width of the drawable area.
     * @param height - number of pixels of the height of the drawable area.
     */
    MakeSurface(width: number, height: number): Surface | null;

    /**
     * Creates a WebGL Context from the given canvas with the given options. If options are omitted,
     * sensible defaults will be used.
     * @param canvas
     * @param opts
     */
    GetWebGLContext(canvas: HTMLCanvasElement, opts?: WebGLOptions): WebGLContextHandle;

    /**
     * Creates a GrDirectContext from the given WebGL Context.
     * @param ctx
     */
    MakeGrContext(ctx: WebGLContextHandle): GrDirectContext | null;

    /**
     * Creates a Surface that will be drawn to the given GrDirectContext (and show up on screen).
     * @param ctx
     * @param width - number of pixels of the width of the visible area.
     * @param height - number of pixels of the height of the visible area.
     * @param colorSpace
     */
    MakeOnScreenGLSurface(ctx: GrDirectContext, width: number, height: number,
                          colorSpace: ColorSpace): Surface | null;

    /**
     * Returns a (non-visible) Surface on the GPU. It has the given dimensions and uses 8888
     * color depth and premultiplied alpha. See Surface.h for more details.
     * @param ctx
     * @param width
     * @param height
     */
    MakeRenderTarget(ctx: GrDirectContext, width: number, height: number): Surface | null;

    /**
     * Returns a (non-visible) Surface on the GPU. It has the settings provided by image info.
     * See Surface.h for more details.
     * @param ctx
     * @param info
     */
    MakeRenderTarget(ctx: GrDirectContext, info: ImageInfo): Surface | null;

    /**
     * Returns a texture-backed image based on the content in src. It assumes the image is
     * RGBA_8888, unpremul and SRGB. This image can be re-used across multiple surfaces.
     *
     * Not available for software-backed surfaces.
     * @param src - CanvasKit will take ownership of the TextureSource and clean it up when
     *              the image is destroyed.
     * @param info - If provided, will be used to determine the width/height/format of the
     *               source image. If not, sensible defaults will be used.
     */
    MakeLazyImageFromTextureSource(src: TextureSource, info?: ImageInfo | PartialImageInfo): Image;

    /**
     * Deletes the associated WebGLContext. Function not available on the CPU version.
     * @param ctx
     */
    deleteContext(ctx: WebGLContextHandle): void;

    /**
     * Returns the max size of the global cache for bitmaps used by CanvasKit.
     */
    getDecodeCacheLimitBytes(): number;
    /**
     * Returns the current size of the global cache for bitmaps used by CanvasKit.
     */
    getDecodeCacheUsedBytes(): number;

    /**
     * Sets the max size of the global cache for bitmaps used by CanvasKit.
     * @param size - number of bytes that can be used to cache bitmaps.
     */
    setDecodeCacheLimitBytes(size: number): void;

    /**
     * Decodes the given bytes into an animated image. Returns null if the bytes were invalid.
     * The passed in bytes will be copied into the WASM heap, so the caller can dispose of them.
     *
     * The returned AnimatedImage will be "pointing to" the first frame, i.e. currentFrameDuration
     * and makeImageAtCurrentFrame will be referring to the first frame.
     * @param bytes
     */
    MakeAnimatedImageFromEncoded(bytes: Uint8Array | ArrayBuffer): AnimatedImage | null;

    /**
     * Returns an emulated Canvas2D of the given size.
     * @param width
     * @param height
     */
    MakeCanvas(width: number, height: number): EmulatedCanvas2D;

    /**
     * Returns an image with the given pixel data and format.
     * Note that we will always make a copy of the pixel data, because of inconsistencies in
     * behavior between GPU and CPU (i.e. the pixel data will be turned into a GPU texture and
     * not modifiable after creation).
     *
     * @param info
     * @param bytes - bytes representing the pixel data.
     * @param bytesPerRow
     */
    MakeImage(info: ImageInfo, bytes: number[] | Uint8Array | Uint8ClampedArray,
              bytesPerRow: number): Image | null;

    /**
     * Return an Image backed by the encoded data, but attempt to defer decoding until the image
     * is actually used/drawn. This deferral allows the system to cache the result, either on the
     * CPU or on the GPU, depending on where the image is drawn.
     * This decoding uses the codecs that have been compiled into CanvasKit. If the bytes are
     * invalid (or an unrecognized codec), null will be returned. See Image.h for more details.
     * @param bytes
     */
    MakeImageFromEncoded(bytes: Uint8Array | ArrayBuffer): Image | null;

    /**
     * Returns an Image with the data from the provided CanvasImageSource (e.g. <img>). This will
     * use the browser's built in codecs, in that src will be drawn to a canvas and then readback
     * and placed into an Image.
     * @param src
     */
    MakeImageFromCanvasImageSource(src: CanvasImageSource): Image;

    /**
     * Returns an SkPicture which has been serialized previously to the given bytes.
     * @param bytes
     */
    MakePicture(bytes: Uint8Array | ArrayBuffer): SkPicture | null;

    /**
     * Returns an Vertices based on the given positions and optional parameters.
     * See SkVertices.h (especially the Builder) for more details.
     * @param mode
     * @param positions
     * @param textureCoordinates
     * @param colors - either a list of int colors or a flattened color array.
     * @param indices
     * @param isVolatile
     */
    MakeVertices(mode: VertexMode, positions: InputFlattenedPointArray,
                 textureCoordinates?: InputFlattenedPointArray | null,
                 colors?: Float32Array | ColorIntArray | null, indices?: number[] | null,
                 isVolatile?: boolean): Vertices;

    /**
     * Returns a Skottie animation built from the provided json string.
     * Requires that Skottie be compiled into CanvasKit.
     * @param json
     */
    MakeAnimation(json: string): SkottieAnimation;

    /**
     * Returns a managed Skottie animation built from the provided json string and assets.
     * Requires that Skottie be compiled into CanvasKit.
     * @param json
     * @param assets - a dictionary of named blobs: { key: ArrayBuffer, ... }
     * @param filterPrefix - an optional string acting as a name filter for selecting "interesting"
     *                       Lottie properties (surfaced in the embedded player controls)
     * @param soundMap - an optional mapping of sound identifiers (strings) to AudioPlayers.
     *                   Only needed if the animation supports sound.
     */
    MakeManagedAnimation(json: string, assets?: Record<string, ArrayBuffer>,
                         filterPrefix?: string, soundMap?: SoundMap): ManagedSkottieAnimation;

    /**
     * Returns a Particles effect built from the provided json string and assets.
     * Requires that Particles be compiled into CanvasKit
     * @param json
     * @param assets
     */
    MakeParticles(json: string, assets?: Record<string, ArrayBuffer>): Particles;

    // Constructors, i.e. things made with `new CanvasKit.Foo()`;
    readonly ImageData: ImageDataConstructor;
    readonly ParagraphStyle: ParagraphStyleConstructor;
    readonly ContourMeasureIter: ContourMeasureIterConstructor;
    readonly Font: FontConstructor;
    readonly Paint: DefaultConstructor<Paint>;
    readonly Path: PathConstructorAndFactory;
    readonly PictureRecorder: DefaultConstructor<PictureRecorder>;
    readonly TextStyle: TextStyleConstructor;

    // Factories, i.e. things made with CanvasKit.Foo.MakeTurboEncabulator()
    readonly ParagraphBuilder: ParagraphBuilderFactory;
    readonly ColorFilter: ColorFilterFactory;
    readonly FontMgr: FontMgrFactory;
    readonly ImageFilter: ImageFilterFactory;
    readonly MaskFilter: MaskFilterFactory;
    readonly PathEffect: PathEffectFactory;
    readonly RuntimeEffect: RuntimeEffectFactory;
    readonly Shader: ShaderFactory;
    readonly TextBlob: TextBlobFactory;
    readonly Typeface: TypefaceFactory;
    readonly TypefaceFontProvider: TypefaceFontProviderFactory;

    // Misc
    readonly ColorMatrix: ColorMatrixHelpers;
    readonly Matrix: Matrix3x3Helpers;
    readonly M44: Matrix4x4Helpers;
    readonly Vector: VectorHelpers;

    // Core Enums
    readonly AlphaType: AlphaTypeEnumValues;
    readonly BlendMode: BlendModeEnumValues;
    readonly BlurStyle: BlurStyleEnumValues;
    readonly ClipOp: ClipOpEnumValues;
    readonly ColorType: ColorTypeEnumValues;
    readonly FillType: FillTypeEnumValues;
    readonly FilterMode: FilterModeEnumValues;
    readonly FontEdging: FontEdgingEnumValues;
    readonly FontHinting: FontHintingEnumValues;
    readonly GlyphRunFlags: GlyphRunFlagValues;
    readonly ImageFormat: ImageFormatEnumValues;
    readonly MipmapMode: MipmapModeEnumValues;
    readonly PaintStyle: PaintStyleEnumValues;
    readonly PathOp: PathOpEnumValues;
    readonly PointMode: PointModeEnumValues;
    readonly ColorSpace: ColorSpaceEnumValues;
    readonly StrokeCap: StrokeCapEnumValues;
    readonly StrokeJoin: StrokeJoinEnumValues;
    readonly TileMode: TileModeEnumValues;
    readonly VertexMode: VertexModeEnumValues;

    // Core Constants
    readonly TRANSPARENT: Color;
    readonly BLACK: Color;
    readonly WHITE: Color;
    readonly RED: Color;
    readonly GREEN: Color;
    readonly BLUE: Color;
    readonly YELLOW: Color;
    readonly CYAN: Color;
    readonly MAGENTA: Color;

    readonly MOVE_VERB: number;
    readonly LINE_VERB: number;
    readonly QUAD_VERB: number;
    readonly CONIC_VERB: number;
    readonly CUBIC_VERB: number;
    readonly CLOSE_VERB: number;

    readonly SaveLayerInitWithPrevious: SaveLayerFlag;
    readonly SaveLayerF16ColorType: SaveLayerFlag;

    /**
     * Use this shadow flag to indicate the occluding object is not opaque. Knowing that the
     * occluder is opaque allows us to cull shadow geometry behind it and improve performance.
     */
    readonly ShadowTransparentOccluder: number;
    /**
     * Use this shadow flag to not use analytic shadows.
     */
    readonly ShadowGeometricOnly: number;
    /**
     * Use this shadow flag to indicate the light position represents a direction and light radius
     * is blur radius at elevation 1.
     */
    readonly ShadowDirectionalLight: number;

    readonly gpu?: boolean; // true if GPU code was compiled in
    readonly managed_skottie?: boolean; // true if advanced (managed) Skottie code was compiled in
    readonly particles?: boolean; // true if Particles code was compiled in
    readonly rt_effect?: boolean; // true if RuntimeEffect was compiled in
    readonly skottie?: boolean; // true if base Skottie code was compiled in

    // Paragraph Enums
    readonly Affinity: AffinityEnumValues;
    readonly DecorationStyle: DecorationStyleEnumValues;
    readonly FontSlant: FontSlantEnumValues;
    readonly FontWeight: FontWeightEnumValues;
    readonly FontWidth: FontWidthEnumValues;
    readonly PlaceholderAlignment: PlaceholderAlignmentEnumValues;
    readonly RectHeightStyle: RectHeightStyleEnumValues;
    readonly RectWidthStyle: RectWidthStyleEnumValues;
    readonly TextAlign: TextAlignEnumValues;
    readonly TextBaseline: TextBaselineEnumValues;
    readonly TextDirection: TextDirectionEnumValues;
    readonly TextHeightBehavior: TextHeightBehaviorEnumValues;

    // Paragraph Constants
    readonly NoDecoration: number;
    readonly UnderlineDecoration: number;
    readonly OverlineDecoration: number;
    readonly LineThroughDecoration: number;
}

export interface Camera {
    /** a 3d point locating the camera. */
    eye: Vector3;
    /** center of attention - the 3d point the camera is looking at. */
    coa: Vector3;
    /**
     * A unit vector pointing the cameras up direction. Note that using only eye and coa
     * would leave the roll of the camera unspecified.
     */
    up: Vector3;
    /** near clipping plane distance */
    near: number;
    /** far clipping plane distance */
    far: number;
    /** field of view in radians */
    angle: AngleInRadians;
}

/**
 * CanvasKit is built with Emscripten and Embind. Embind adds the following methods to all objects
 * that are exposed with it.
 */
export interface EmbindObject<T extends EmbindObject<T>> {
    clone(): T;
    delete(): void;
    deleteLater(): void;
    isAliasOf(other: any): boolean;
    isDeleted(): boolean;
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

export interface EmulatedCanvas2D {
    /**
     * Cleans up all resources associated with this emulated canvas.
     */
    dispose(): void;
    /**
     * Decodes an image with the given bytes.
     * @param bytes
     */
    decodeImage(bytes: ArrayBuffer | Uint8Array): Image;

    /**
     * Returns an emulated canvas2d context if type == '2d', null otherwise.
     * @param type
     */
    getContext(type: string): EmulatedCanvas2DContext | null;

    /**
     * Loads the given font with the given descriptors. Emulates new FontFace().
     * @param bytes
     * @param descriptors
     */
    loadFont(bytes: ArrayBuffer | Uint8Array, descriptors: Record<string, string>): void;

    /**
     * Returns an new emulated Path2D object.
     * @param str - an SVG string representing a path.
     */
    makePath2D(str?: string): EmulatedPath2D;

    /**
     * Returns the current canvas as a base64 encoded image string.
     * @param codec - image/png by default; image/jpeg also supported.
     * @param quality
     */
    toDataURL(codec?: string, quality?: number): string;
}

/** Part of the Canvas2D emulation code */
export type EmulatedCanvas2DContext = CanvasRenderingContext2D;
export type EmulatedImageData = ImageData;
export type EmulatedPath2D = Path2D;

export interface FontStyle {
    weight?: FontWeight;
    width?: FontWidth;
    slant?: FontSlant;
}

/**
 * See GrDirectContext.h for more on this class.
 */
export interface GrDirectContext extends EmbindObject<GrDirectContext> {
    getResourceCacheLimitBytes(): number;
    getResourceCacheUsageBytes(): number;
    releaseResourcesAndAbandonContext(): void;
    setResourceCacheLimitBytes(bytes: number): void;
}

/**
 * See Metrics.h for more on this struct.
 */
export interface LineMetrics {
    /** The index in the text buffer the line begins. */
    startIndex: number;
    /** The index in the text buffer the line ends. */
    endIndex: number;
    endExcludingWhitespaces: number;
    endIncludingNewline: number;
    /** True if the line ends in a hard break (e.g. newline) */
    isHardBreak: boolean;
    /**
     * The final computed ascent for the line. This can be impacted by
     * the strut, height, scaling, as well as outlying runs that are very tall.
     */
    ascent: number;
    /**
     * The final computed descent for the line. This can be impacted by
     * the strut, height, scaling, as well as outlying runs that are very tall.
     */
    descent: number;
    /** round(ascent + descent) */
    height: number;
    /** width of the line */
    width: number;
    /** The left edge of the line. The right edge can be obtained with `left + width` */
    left: number;
    /** The y position of the baseline for this line from the top of the paragraph. */
    baseline: number;
    /** Zero indexed line number. */
    lineNumber: number;
}

export interface Range {
    first: number;
    last: number;
}

/**
 * Information for a run of shaped text. See Paragraph.getShapedLines()
 *
 * Notes:
 * positions is documented as Float32, but it holds twice as many as you expect, and they
 * are treated logically as pairs of floats: {x0, y0}, {x1, y1}, ... for each glyph.
 *
 * positions and offsets arrays have 1 extra slot (actually 2 for positions)
 * to describe the location "after" the last glyph in the glyphs array.
 */
export interface GlyphRun {
    typeface: Typeface;     // currently set to null (temporary)
    size: number;
    fakeBold: boolean;
    fakeItalic: boolean;

    glyphs: Uint16Array;
    positions: Float32Array;    // alternating x0, y0, x1, y1, ...
    offsets: Uint32Array;
    flags: number;              // see GlyphRunFlags
}

/**
 * Information for a paragraph of text. See Paragraph.getShapedLines()
 */
 export interface ShapedLine {
    textRange: Range;   // first and last character offsets for the line (derived from runs[])
    top: number;        // top y-coordinate for the line
    bottom: number;     // bottom y-coordinate for the line
    baseline: number;   // baseline y-coordinate for the line
    runs: GlyphRun[];   // array of GlyphRun objects for the line
}

/**
 * Input to ShapeText(..., FontBlock[], ...);
 */
export interface FontBlock {
    length: number;     // number of text codepoints this block is applied to

    typeface: Typeface;
    size: number;
    fakeBold: boolean;
    fakeItalic: boolean;
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
    subarray(start: number, end: number): TypedArray;
    /**
     * Return a read/write view of the memory. Do not cache the TypedArray this returns, it may be
     * invalidated if the WASM heap is resized. If this TypedArray is passed into a CanvasKit API,
     * it will not be copied again, only the pointer will be re-used.
     */
    toTypedArray(): TypedArray;
}

/**
 * This represents a subset of an animation's duration.
 */
export interface AnimationMarker {
    name: string;
    t0: number; // 0.0 to 1.0
    t1: number; // 0.0 to 1.0
}

/**
 * This object maintains a single audio layer during skottie playback
 */
export interface AudioPlayer {
    /**
     * Playback control callback, emitted for each corresponding Animation::seek().
     *
     * Will seek to time t (seconds) relative to the layer's timeline origin.
     * Negative t values are used to signal off state (stop playback outside layer span).
     */
    seek(t: number): void;
}

/**
 * Mapping of sound names (strings) to AudioPlayers
 */
export interface SoundMap {
    /**
     * Returns AudioPlayer for a certain audio layer
     * @param key string identifier, name of audio file the desired AudioPlayer manages
     */
    getPlayer(key: string): AudioPlayer;
}

/**
 * Named color property.
 */
export interface ColorProperty {
    /**
     * Property identifier, usually the node name.
     */
    key: string;
    /**
     * Property value (RGBA, 255-based).
     */
    value: ColorInt;
}

/**
 * Named opacity property.
 */
export interface OpacityProperty {
    /**
     * Property identifier, usually the node name.
     */
    key: string;
    /**
     * Property value (0..100).
     */
    value: number;
}

/**
 * Text property value.
 */
export interface TextValue {
    /**
     * The text string payload.
     */
    text: string;
    /**
     * Font size.
     */
    size: number;
}

/**
 * Named text property.
 */
export interface TextProperty {
    /**
     * Property identifier, usually the node name.
     */
    key: string;
    /**
     * Property value.
     */
    value: TextValue;
}

export interface ManagedSkottieAnimation extends SkottieAnimation {
    setColor(key: string, color: InputColor): boolean;
    setOpacity(key: string, opacity: number): boolean;
    setText(key: string, text: string, size: number): boolean;
    getMarkers(): AnimationMarker[];
    getColorProps(): ColorProperty[];
    getOpacityProps(): OpacityProperty[];
    getTextProps(): TextProperty[];
}

/**
 * See Paragraph.h for more information on this class. This is only available if Paragraph has
 * been compiled in.
 */
export interface Paragraph extends EmbindObject<Paragraph> {
    didExceedMaxLines(): boolean;
    getAlphabeticBaseline(): number;

    /**
     * Returns the index of the glyph that corresponds to the provided coordinate,
     * with the top left corner as the origin, and +y direction as down.
     */
    getGlyphPositionAtCoordinate(dx: number, dy: number): PositionWithAffinity;

    getHeight(): number;
    getIdeographicBaseline(): number;
    getLineMetrics(): LineMetrics[];
    getLongestLine(): number;
    getMaxIntrinsicWidth(): number;
    getMaxWidth(): number;
    getMinIntrinsicWidth(): number;
    getRectsForPlaceholders(): FlattenedRectangleArray;

    /**
     * Returns bounding boxes that enclose all text in the range of glpyh indexes [start, end).
     * @param start
     * @param end
     * @param hStyle
     * @param wStyle
     */
    getRectsForRange(start: number, end: number, hStyle: RectHeightStyle,
                     wStyle: RectWidthStyle): FlattenedRectangleArray;

    /**
     * Finds the first and last glyphs that define a word containing the glyph at index offset.
     * @param offset
     */
    getWordBoundary(offset: number): URange;

    /**
     * Returns an array of ShapedLine objects, describing the paragraph.
     */
    getShapedLines(): ShapedLine[];

    /**
     * Lays out the text in the paragraph so it is wrapped to the given width.
     * @param width
     */
    layout(width: number): void;
}

export interface ParagraphBuilder extends EmbindObject<ParagraphBuilder> {
    /**
     * Pushes the information required to leave an open space.
     * @param width
     * @param height
     * @param alignment
     * @param baseline
     * @param offset
     */
    addPlaceholder(width?: number, height?: number, alignment?: PlaceholderAlignment,
                   baseline?: TextBaseline, offset?: number): void;

    /**
     * Adds text to the builder. Forms the proper runs to use the upper-most style
     * on the style_stack.
     * @param str
     */
    addText(str: string): void;

    /**
     * Returns a Paragraph object that can be used to be layout and paint the text to an
     * Canvas.
     */
    build(): Paragraph;

    /**
     * Remove a style from the stack. Useful to apply different styles to chunks
     * of text such as bolding.
     */
    pop(): void;

    /**
     * Push a style to the stack. The corresponding text added with addText will
     * use the top-most style.
     * @param text
     */
    pushStyle(text: TextStyle): void;

    /**
     * Pushes a TextStyle using paints instead of colors for foreground and background.
     * @param textStyle
     * @param fg
     * @param bg
     */
    pushPaintStyle(textStyle: TextStyle, fg: Paint, bg: Paint): void;
}

export interface ParagraphStyle {
    disableHinting?: boolean;
    ellipsis?: string;
    heightMultiplier?: number;
    maxLines?: number;
    strutStyle?: StrutStyle;
    textAlign?: TextAlign;
    textDirection?: TextDirection;
    textHeightBehavior?: TextHeightBehavior;
    textStyle?: TextStyle;
}

export interface PositionWithAffinity {
    pos: number;
    affinity: Affinity;
}

/**
 * See SkParticleEffect.h for more details.
 */
export interface Particles extends EmbindObject<Particles> {
    /**
     * Draws the current state of the particles on the given canvas.
     * @param canvas
     */
    draw(canvas: Canvas): void;

    /**
     * Returns a Float32Array bound to the WASM memory of these uniforms. Changing these
     * floats will change the corresponding uniforms instantly.
     */
    uniforms(): Float32Array;

    /**
     * Returns the nth uniform from the effect.
     * @param index
     */
    getUniform(index: number): SkSLUniform;

    /**
     * Returns the number of uniforms on the effect.
     */
    getUniformCount(): number;

    /**
     * Returns the total number of floats across all uniforms on the effect. This is the length
     * of the array returned by `uniforms()`. For example, an effect with a single float3 uniform,
     * would return 1 from `getUniformCount()`, but 3 from `getUniformFloatCount()`.
     */
    getUniformFloatCount(): number;

    /**
     * Returns the name of the nth effect uniform.
     * @param index
     */
    getUniformName(index: number): string;

    /**
     * Sets the base position of the effect.
     * @param point
     */
    setPosition(point: InputPoint): void;

    /**
     * Sets the base rate of the effect.
     * @param rate
     */
    setRate(rate: number): void;

    /**
     * Starts playing the effect.
     * @param now
     * @param looping
     */
    start(now: number, looping: boolean): void;

    /**
     * Updates the effect using the new time.
     * @param now
     */
    update(now: number): void;
}

export interface SkSLUniform {
    columns: number;
    rows: number;
    /** The index into the uniforms array that this uniform begins. */
    slot: number;
}

/**
 * See SkAnimatedImage.h for more information on this class.
 */
export interface AnimatedImage extends EmbindObject<AnimatedImage> {
    /**
     * Returns the length of the current frame in ms.
     */
    currentFrameDuration(): number;
    /**
     * Decodes the next frame. Returns the length of that new frame in ms.
     * Returns -1 when the animation is on the last frame.
     */
    decodeNextFrame(): number;

    /**
     * Return the total number of frames in the animation.
     */
    getFrameCount(): number;

    /**
     * Return the repetition count for this animation.
     */
    getRepetitionCount(): number;

    /**
     * Returns the possibly scaled height of the image.
     */
    height(): number;

    /**
     * Returns a still image of the current frame or null if there is no current frame.
     */
    makeImageAtCurrentFrame(): Image | null;

    /**
     * Reset the animation to the beginning.
     */
    reset(): void;

    /**
     * Returns the possibly scaled width of the image.
     */
    width(): number;
}

/**
 * See SkCanvas.h for more information on this class.
 */
export interface Canvas extends EmbindObject<Canvas> {
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
    clipPath(path: Path, op: ClipOp, doAntiAlias: boolean): void;

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
     * Draws arc using clip, Matrix, and Paint paint.
     *
     * Arc is part of oval bounded by oval, sweeping from startAngle to startAngle plus
     * sweepAngle. startAngle and sweepAngle are in degrees.
     * @param oval - bounds of oval containing arc to draw
     * @param startAngle - angle in degrees where arc begins
     * @param sweepAngle - sweep angle in degrees; positive is clockwise
     * @param useCenter - if true, include the center of the oval
     * @param paint
     */
    drawArc(oval: InputRect, startAngle: AngleInDegrees, sweepAngle: AngleInDegrees,
            useCenter: boolean, paint: Paint): void;

    /**
     * Draws a set of sprites from atlas, using clip, Matrix, and optional Paint paint.
     * @param atlas - Image containing sprites
     * @param srcRects - Rect locations of sprites in atlas
     * @param dstXforms - RSXform mappings for sprites in atlas
     * @param paint
     * @param blendMode - BlendMode combining colors and sprites
     * @param colors - If provided, will be blended with sprite using blendMode.
     * @param sampling - Specifies sampling options. If null, bilinear is used.
     */
    drawAtlas(atlas: Image, srcRects: InputFlattenedRectangleArray,
              dstXforms: InputFlattenedRSXFormArray, paint: Paint,
              blendMode?: BlendMode | null, colors?: ColorIntArray | null,
              sampling?: CubicResampler | FilterOptions): void;

    /**
     * Draws a circle at (cx, cy) with the given radius.
     * @param cx
     * @param cy
     * @param radius
     * @param paint
     */
    drawCircle(cx: number, cy: number, radius: number, paint: Paint): void;

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
    drawColorInt(color: ColorInt, blendMode?: BlendMode): void;

    /**
     * Draws RRect outer and inner using clip, Matrix, and Paint paint.
     * outer must contain inner or the drawing is undefined.
     * @param outer
     * @param inner
     * @param paint
     */
    drawDRRect(outer: InputRRect, inner: InputRRect, paint: Paint): void;

    /**
     * Draws a run of glyphs, at corresponding positions, in a given font.
     * @param glyphs the array of glyph IDs (Uint16TypedArray)
     * @param positions the array of x,y floats to position each glyph
     * @param x x-coordinate of the origin of the entire run
     * @param y y-coordinate of the origin of the entire run
     * @param font the font that contains the glyphs
     * @param paint
     */
    drawGlyphs(glyphs: InputGlyphIDArray,
               positions: InputFlattenedPointArray,
               x: number, y: number,
               font: Font, paint: Paint): void;

    /**
     * Draws the given image with its top-left corner at (left, top) using the current clip,
     * the current matrix, and optionally-provided paint.
     * @param img
     * @param left
     * @param top
     * @param paint
     */
    drawImage(img: Image, left: number, top: number, paint?: Paint | null): void;

    /**
     * Draws the given image with its top-left corner at (left, top) using the current clip,
     * the current matrix. It will use the cubic sampling options B and C if necessary.
     * @param img
     * @param left
     * @param top
     * @param B - See CubicResampler in SkSamplingOptions.h for more information
     * @param C - See CubicResampler in SkSamplingOptions.h for more information
     * @param paint
     */
    drawImageCubic(img: Image, left: number, top: number, B: number, C: number,
                   paint?: Paint | null): void;

    /**
     * Draws the given image with its top-left corner at (left, top) using the current clip,
     * the current matrix. It will use the provided sampling options if necessary.
     * @param img
     * @param left
     * @param top
     * @param fm - The filter mode.
     * @param mm - The mipmap mode. Note: for settings other than None, the image must have mipmaps
     *             calculated with makeCopyWithDefaultMipmaps;
     * @param paint
     */
    drawImageOptions(img: Image, left: number, top: number, fm: FilterMode,
                     mm: MipmapMode, paint?: Paint | null): void;

    /**
     *  Draws the provided image stretched proportionally to fit into dst rectangle.
     *  The center rectangle divides the image into nine sections: four sides, four corners, and
     *  the center.
     * @param img
     * @param center
     * @param dest
     * @param filter - what technique to use when sampling the image
     * @param paint
     */
    drawImageNine(img: Image, center: InputIRect, dest: InputRect, filter: FilterMode,
                  paint?: Paint | null): void;

    /**
     * Draws sub-rectangle src from provided image, scaled and translated to fill dst rectangle.
     * @param img
     * @param src
     * @param dest
     * @param paint
     * @param fastSample - if false, will filter strictly within src.
     */
    drawImageRect(img: Image, src: InputRect, dest: InputRect, paint: Paint,
                  fastSample?: boolean): void;

    /**
     * Draws sub-rectangle src from provided image, scaled and translated to fill dst rectangle.
     * It will use the cubic sampling options B and C if necessary.
     * @param img
     * @param src
     * @param dest
     * @param B - See CubicResampler in SkSamplingOptions.h for more information
     * @param C - See CubicResampler in SkSamplingOptions.h for more information
     * @param paint
     */
    drawImageRectCubic(img: Image, src: InputRect, dest: InputRect,
                       B: number, C: number, paint?: Paint | null): void;

    /**
     * Draws sub-rectangle src from provided image, scaled and translated to fill dst rectangle.
     * It will use the provided sampling options if necessary.
     * @param img
     * @param src
     * @param dest
     * @param fm - The filter mode.
     * @param mm - The mipmap mode. Note: for settings other than None, the image must have mipmaps
     *             calculated with makeCopyWithDefaultMipmaps;
     * @param paint
     */
    drawImageRectOptions(img: Image, src: InputRect, dest: InputRect, fm: FilterMode,
                         mm: MipmapMode, paint?: Paint | null): void;

    /**
     * Draws line segment from (x0, y0) to (x1, y1) using the current clip, current matrix,
     * and the provided paint.
     * @param x0
     * @param y0
     * @param x1
     * @param y1
     * @param paint
     */
    drawLine(x0: number, y0: number, x1: number, y1: number, paint: Paint): void;

    /**
     * Draws an oval bounded by the given rectangle using the current clip, current matrix,
     * and the provided paint.
     * @param oval
     * @param paint
     */
    drawOval(oval: InputRect, paint: Paint): void;

    /**
     * Fills clip with the given paint.
     * @param paint
     */
    drawPaint(paint: Paint): void;

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
    drawPath(path: Path, paint: Paint): void;

    /**
     * Draws a cubic patch defined by 12 control points [top, right, bottom, left] with optional
     * colors and shader-coordinates [4] specifed for each corner [top-left, top-right, bottom-right, bottom-left]
     * @param cubics 12 points : 4 connected cubics specifying the boundary of the patch
     * @param colors optional colors interpolated across the patch
     * @param texs optional shader coordinates interpolated across the patch
     * @param mode Specifies how shader and colors blend (if both are specified)
     * @param paint
     */
    drawPatch(cubics: InputFlattenedPointArray,
              colors?: ColorIntArray | Color[] | null,
              texs?: InputFlattenedPointArray | null,
              mode?: BlendMode | null,
              paint?: Paint): void;

    /**
     * Draws the given picture using the current clip, current matrix, and the provided paint.
     * @param skp
     */
    drawPicture(skp: SkPicture): void;

    /**
     * Draws the given points using the current clip, current matrix, and the provided paint.
     *
     * See Canvas.h for more on the mode and its interaction with paint.
     * @param mode
     * @param points
     * @param paint
     */
    drawPoints(mode: PointMode, points: InputFlattenedPointArray, paint: Paint): void;

    /**
     * Draws the given rectangle using the current clip, current matrix, and the provided paint.
     * @param rect
     * @param paint
     */
    drawRect(rect: InputRect, paint: Paint): void;

    /**
     * Draws the given rectangle using the current clip, current matrix, and the provided paint.
     * @param left
     * @param top
     * @param right
     * @param bottom
     * @param paint
     */
    drawRect4f(left: number, top: number, right: number, bottom: number, paint: Paint): void;

    /**
     * Draws the given rectangle with rounded corners using the current clip, current matrix,
     * and the provided paint.
     * @param rrect
     * @param paint
     */
    drawRRect(rrect: InputRRect, paint: Paint): void;

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
    drawShadow(path: Path, zPlaneParams: InputVector3, lightPos: InputVector3, lightRadius: number,
               ambientColor: InputColor, spotColor: InputColor, flags: number): void;

    /**
     * Draw the given text at the location (x, y) using the provided paint and font. The text will
     * be drawn as is; no shaping, left-to-right, etc.
     * @param str
     * @param x
     * @param y
     * @param paint
     * @param font
     */
    drawText(str: string, x: number, y: number, paint: Paint, font: Font): void;

    /**
     * Draws the given TextBlob at (x, y) using the current clip, current matrix, and the
     * provided paint. Reminder that the fonts used to draw TextBlob are part of the blob.
     * @param blob
     * @param x
     * @param y
     * @param paint
     */
    drawTextBlob(blob: TextBlob, x: number, y: number, paint: Paint): void;

    /**
     * Draws the given vertices (a triangle mesh) using the current clip, current matrix, and the
     * provided paint.
     *  If paint contains an Shader and vertices does not contain texCoords, the shader
     *  is mapped using the vertices' positions.
     *  If vertices colors are defined in vertices, and Paint paint contains Shader,
     *  BlendMode mode combines vertices colors with Shader.
     * @param verts
     * @param mode
     * @param paint
     */
    drawVertices(verts: Vertices, mode: BlendMode, paint: Paint): void;

    /**
     * Returns the current transform from local coordinates to the 'device', which for most
     * purposes means pixels.
     */
    getLocalToDevice(): Matrix4x4;

    /**
     * Returns the number of saved states, each containing: Matrix and clip.
     * Equals the number of save() calls less the number of restore() calls plus one.
     * The save count of a new canvas is one.
     */
    getSaveCount(): number;

    /**
     * Legacy version of getLocalToDevice(), which strips away any Z information, and
     * just returns a 3x3 version.
     */
    getTotalMatrix(): number[];

    /**
     * Creates Surface matching info and props, and associates it with Canvas.
     * Returns null if no match found.
     * @param info
     */
    makeSurface(info: ImageInfo): Surface | null;

    /**
     * Returns a TypedArray containing the pixels reading starting at (srcX, srcY) and does not
     * exceed the size indicated by imageInfo. See SkCanvas.h for more on the caveats.
     *
     * If dest is not provided, we allocate memory equal to the provided height * the provided
     * bytesPerRow to fill the data with.
     *
     * This is generally a very expensive call for the GPU backend.
     *
     * @param srcX
     * @param srcY
     * @param imageInfo - describes the destination format of the pixels.
     * @param dest - If provided, the pixels will be copied into the allocated buffer allowing
     *        access to the pixels without allocating a new TypedArray.
     * @param bytesPerRow - number of bytes per row. Must be provided if dest is set. This
     *        depends on destination ColorType. For example, it must be at least 4 * width for
     *        the 8888 color type.
     * @returns a TypedArray appropriate for the specified ColorType. Note that 16 bit floats are
     *          not supported in JS, so that colorType corresponds to raw bytes Uint8Array.
     */
    readPixels(srcX: number, srcY: number, imageInfo: ImageInfo, dest?: MallocObj,
               bytesPerRow?: number): Uint8Array | Float32Array | null;

    /**
     * Removes changes to the current matrix and clip since Canvas state was
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
    rotate(rot: AngleInDegrees, rx: number, ry: number): void;

    /**
     * Saves the current matrix and clip and returns current height of the stack.
     */
    save(): number;

    /**
     * Saves Matrix and clip, and allocates a SkBitmap for subsequent drawing.
     * Calling restore() discards changes to Matrix and clip, and draws the SkBitmap.
     * It returns the height of the stack.
     * See Canvas.h for more.
     * @param paint
     * @param bounds
     * @param backdrop
     * @param flags
     */
    saveLayer(paint?: Paint, bounds?: InputRect | null, backdrop?: ImageFilter | null,
              flags?: SaveLayerFlag): number;

    /**
     * Scales the current matrix by sx on the x-axis and sy on the y-axis.
     * @param sx
     * @param sy
     */
    scale(sx: number, sy: number): void;

    /**
     *  Skews Matrix by sx on the x-axis and sy on the y-axis. A positive value of sx
     *  skews the drawing right as y-axis values increase; a positive value of sy skews
     *  the drawing down as x-axis values increase.
     * @param sx
     * @param sy
     */
    skew(sx: number, sy: number): void;

    /**
     * Translates Matrix by dx along the x-axis and dy along the y-axis.
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
 * See SkColorFilter.h for more on this class. The objects are opaque.
 */
export type ColorFilter = EmbindObject<ColorFilter>;

export interface ContourMeasureIter extends EmbindObject<ContourMeasureIter> {
    /**
     *  Iterates through contours in path, returning a contour-measure object for each contour
     *  in the path. Returns null when it is done.
     *
     *  See SkContourMeasure.h for more details.
     */
    next(): ContourMeasure | null;
}

export interface ContourMeasure extends EmbindObject<ContourMeasure> {
    /**
     * Returns the given position and tangent line for the distance on the given contour.
     * The return value is 4 floats in this order: posX, posY, vecX, vecY.
     * @param distance - will be pinned between 0 and length().
     * @param output - if provided, the four floats of the PosTan will be copied into this array
     *                 instead of allocating a new one.
     */
    getPosTan(distance: number, output?: PosTan): PosTan;

    /**
     * Returns an Path representing the segement of this contour.
     * @param startD - will be pinned between 0 and length()
     * @param stopD - will be pinned between 0 and length()
     * @param startWithMoveTo
     */
    getSegment(startD: number, stopD: number, startWithMoveTo: boolean): Path;

    /**
     * Returns true if the contour is closed.
     */
    isClosed(): boolean;

    /**
     * Returns the length of this contour.
     */
    length(): number;
}

export interface FontMetrics {
    ascent: number;     // suggested space above the baseline. < 0
    descent: number;    // suggested space below the baseline. > 0
    leading: number;    // suggested spacing between descent of previous line and ascent of next line.
    bounds?: Rect;      // smallest rect containing all glyphs (relative to 0,0)
}

/**
 * See SkFont.h for more on this class.
 */
export interface Font extends EmbindObject<Font> {
    /**
     * Returns the FontMetrics for this font.
     */
    getMetrics(): FontMetrics;

    /**
     * Retrieves the bounds for each glyph in glyphs.
     * If paint is not null, its stroking, PathEffect, and MaskFilter fields are respected.
     * These are returned as flattened rectangles.  For each glyph, there will be 4 floats for
     * left, top, right, bottom (relative to 0, 0) for that glyph.
     * @param glyphs
     * @param paint
     * @param output - if provided, the results will be copied into this array.
     */
    getGlyphBounds(glyphs: InputGlyphIDArray, paint?: Paint | null,
                   output?: Float32Array): Float32Array;

    /**
     * Retrieves the glyph ids for each code point in the provided string. This call is passed to
     * the typeface of this font. Note that glyph IDs are typeface-dependent; different faces
     * may have different ids for the same code point.
     * @param str
     * @param numCodePoints - the number of code points in the string. Defaults to str.length.
     * @param output - if provided, the results will be copied into this array.
     */
    getGlyphIDs(str: string, numCodePoints?: number,
                output?: GlyphIDArray): GlyphIDArray;

    /**
     * Retrieves the advanceX measurements for each glyph.
     * If paint is not null, its stroking, PathEffect, and MaskFilter fields are respected.
     * One width per glyph is returned in the returned array.
     * @param glyphs
     * @param paint
     * @param output - if provided, the results will be copied into this array.
     */
    getGlyphWidths(glyphs: InputGlyphIDArray, paint?: Paint | null,
                   output?: Float32Array): Float32Array;

    /**
     * Computes any intersections of a thick "line" and a run of positionsed glyphs.
     * The thick line is represented as a top and bottom coordinate (positive for
     * below the baseline, negative for above). If there are no intersections
     * (e.g. if this is intended as an underline, and there are no "collisions")
     * then the returned array will be empty. If there are intersections, the array
     * will contain pairs of X coordinates [start, end] for each segment that
     * intersected with a glyph.
     *
     * @param glyphs        the glyphs to intersect with
     * @param positions     x,y coordinates (2 per glyph) for each glyph
     * @param top           top of the thick "line" to use for intersection testing
     * @param bottom        bottom of the thick "line" to use for intersection testing
     * @return              array of [start, end] x-coordinate pairs. Maybe be empty.
     */
    getGlyphIntercepts(glyphs: InputGlyphIDArray, positions: Float32Array | number[],
                       top: number, bottom: number): Float32Array;

    /**
     * Returns text scale on x-axis. Default value is 1.
     */
    getScaleX(): number;

    /**
     * Returns text size in points.
     */
    getSize(): number;

    /**
     * Returns text skew on x-axis. Default value is zero.
     */
    getSkewX(): number;

    /**
     * Returns embolden effect for this font. Default value is false.
     */
    isEmbolden(): boolean;

    /**
     * Returns the Typeface set for this font.
     */
    getTypeface(): Typeface | null;

    /**
     * Requests, but does not require, that edge pixels draw opaque or with partial transparency.
     * @param edging
     */
    setEdging(edging: FontEdging): void;

    /**
     * Requests, but does not require, to use bitmaps in fonts instead of outlines.
     * @param embeddedBitmaps
     */
    setEmbeddedBitmaps(embeddedBitmaps: boolean): void;

    /**
     * Sets level of glyph outline adjustment.
     * @param hinting
     */
    setHinting(hinting: FontHinting): void;

    /**
     * Requests, but does not require, linearly scalable font and glyph metrics.
     *
     * For outline fonts 'true' means font and glyph metrics should ignore hinting and rounding.
     * Note that some bitmap formats may not be able to scale linearly and will ignore this flag.
     * @param linearMetrics
     */
    setLinearMetrics(linearMetrics: boolean): void;

    /**
     * Sets the text scale on the x-axis.
     * @param sx
     */
    setScaleX(sx: number): void;

    /**
     * Sets the text size in points on this font.
     * @param points
     */
    setSize(points: number): void;

    /**
     * Sets the text-skew on the x axis for this font.
     * @param sx
     */
    setSkewX(sx: number): void;

    /**
     * Set embolden effect for this font.
     * @param embolden
     */
    setEmbolden(embolden: boolean): void;

    /**
     * Requests, but does not require, that glyphs respect sub-pixel positioning.
     * @param subpixel
     */
    setSubpixel(subpixel: boolean): void;

    /**
     * Sets the typeface to use with this font. null means to clear the typeface and use the
     * default one.
     * @param face
     */
    setTypeface(face: Typeface | null): void;
}

/**
 * See SkFontMgr.h for more details
 */
export interface FontMgr extends EmbindObject<FontMgr> {
    /**
     * Return the number of font families loaded in this manager. Useful for debugging.
     */
    countFamilies(): number;

    /**
     * Return the nth family name. Useful for debugging.
     * @param index
     */
    getFamilyName(index: number): string;
}

/**
 * See SkImage.h for more information on this class.
 */
export interface Image extends EmbindObject<Image> {
    /**
     * Encodes this image's pixels to the specified format and returns them. Must be built with
     * the specified codec. If the options are unspecified, sensible defaults will be
     * chosen.
     * @param fmt - PNG is the default value.
     * @param quality - a value from 0 to 100; 100 is the least lossy. May be ignored.
     */
    encodeToBytes(fmt?: EncodedImageFormat, quality?: number): Uint8Array | null;

    /**
     * Returns the color space associated with this object.
     * It is the user's responsibility to call delete() on this after it has been used.
     */
    getColorSpace(): ColorSpace;

    /**
     * Returns the width, height, colorType and alphaType associated with this image.
     * Colorspace is separate so as to not accidentally leak that memory.
     */
    getImageInfo(): PartialImageInfo;

    /**
     * Return the height in pixels of the image.
     */
    height(): number;

    /**
     * Returns an Image with the same "base" pixels as the this image, but with mipmap levels
     * automatically generated and attached.
     */
    makeCopyWithDefaultMipmaps(): Image;

    /**
     * Returns this image as a shader with the specified tiling. It will use cubic sampling.
     * @param tx - tile mode in the x direction.
     * @param ty - tile mode in the y direction.
     * @param B - See CubicResampler in SkSamplingOptions.h for more information
     * @param C - See CubicResampler in SkSamplingOptions.h for more information
     * @param localMatrix
     */
    makeShaderCubic(tx: TileMode, ty: TileMode, B: number, C: number,
                    localMatrix?: InputMatrix): Shader;

    /**
     * Returns this image as a shader with the specified tiling. It will use cubic sampling.
     * @param tx - tile mode in the x direction.
     * @param ty - tile mode in the y direction.
     * @param fm - The filter mode.
     * @param mm - The mipmap mode. Note: for settings other than None, the image must have mipmaps
     *             calculated with makeCopyWithDefaultMipmaps;
     * @param localMatrix
     */
    makeShaderOptions(tx: TileMode, ty: TileMode, fm: FilterMode, mm: MipmapMode,
                    localMatrix?: InputMatrix): Shader;

    /**
     * Returns a TypedArray containing the pixels reading starting at (srcX, srcY) and does not
     * exceed the size indicated by imageInfo. See SkImage.h for more on the caveats.
     *
     * If dest is not provided, we allocate memory equal to the provided height * the provided
     * bytesPerRow to fill the data with.
     *
     * @param srcX
     * @param srcY
     * @param imageInfo - describes the destination format of the pixels.
     * @param dest - If provided, the pixels will be copied into the allocated buffer allowing
     *        access to the pixels without allocating a new TypedArray.
     * @param bytesPerRow - number of bytes per row. Must be provided if dest is set. This
     *        depends on destination ColorType. For example, it must be at least 4 * width for
     *        the 8888 color type.
     * @returns a TypedArray appropriate for the specified ColorType. Note that 16 bit floats are
     *          not supported in JS, so that colorType corresponds to raw bytes Uint8Array.
     */
    readPixels(srcX: number, srcY: number, imageInfo: ImageInfo, dest?: MallocObj,
               bytesPerRow?: number): Uint8Array | Float32Array | null;

    /**
     * Return the width in pixels of the image.
     */
    width(): number;
}

/**
 * See ImageFilter.h for more on this class. The objects are opaque.
 */
export type ImageFilter = EmbindObject<ImageFilter>;

export interface ImageInfo {
    alphaType: AlphaType;
    colorSpace: ColorSpace;
    colorType: ColorType;
    height: number;
    width: number;
}

export interface PartialImageInfo {
    alphaType: AlphaType;
    colorType: ColorType;
    height: number;
    width: number;
}

/*
 *  Specifies sampling with bicubic coefficients
 */
export interface CubicResampler {
    B: number;  // 0..1
    C: number;  // 0..1
}

/**
 * Specifies sampling using filter and mipmap options
 */
export interface FilterOptions {
    filter: FilterMode;
    mipmap?: MipmapMode;    // defaults to None if not specified
}

/**
 * See SkMaskFilter.h for more on this class. The objects are opaque.
 */
export type MaskFilter = EmbindObject<MaskFilter>;

/**
 * See SkPaint.h for more information on this class.
 */
export interface Paint extends EmbindObject<Paint> {
    /**
     * Returns a copy of this paint.
     */
    copy(): Paint;

    /**
     * Retrieves the alpha and RGB unpremultiplied. RGB are extended sRGB values
     * (sRGB gamut, and encoded with the sRGB transfer function).
     */
    getColor(): Color;

    /**
     * Returns the geometry drawn at the beginning and end of strokes.
     */
    getStrokeCap(): StrokeCap;

    /**
     * Returns the geometry drawn at the corners of strokes.
     */
    getStrokeJoin(): StrokeJoin;

    /**
     *  Returns the limit at which a sharp corner is drawn beveled.
     */
    getStrokeMiter(): number;

    /**
     * Returns the thickness of the pen used to outline the shape.
     */
    getStrokeWidth(): number;

    /**
     * Replaces alpha, leaving RGBA unchanged. 0 means fully transparent, 1.0 means opaque.
     * @param alpha
     */
    setAlphaf(alpha: number): void;

    /**
     * Requests, but does not require, that edge pixels draw opaque or with
     * partial transparency.
     * @param aa
     */
    setAntiAlias(aa: boolean): void;

    /**
     * Sets the blend mode that is, the mode used to combine source color
     * with destination color.
     * @param mode
     */
    setBlendMode(mode: BlendMode): void;

    /**
     * Sets alpha and RGB used when stroking and filling. The color is four floating
     * point values, unpremultiplied. The color values are interpreted as being in
     * the provided colorSpace.
     * @param color
     * @param colorSpace - defaults to sRGB
     */
    setColor(color: InputColor, colorSpace?: ColorSpace): void;

    /**
     * Sets alpha and RGB used when stroking and filling. The color is four floating
     * point values, unpremultiplied. The color values are interpreted as being in
     * the provided colorSpace.
     * @param r
     * @param g
     * @param b
     * @param a
     * @param colorSpace - defaults to sRGB
     */
    setColorComponents(r: number, g: number, b: number, a: number, colorSpace?: ColorSpace): void;

    /**
     * Sets the current color filter, replacing the existing one if there was one.
     * @param filter
     */
    setColorFilter(filter: ColorFilter): void;

    /**
     * Sets the color used when stroking and filling. The color values are interpreted as being in
     * the provided colorSpace.
     * @param color
     * @param colorSpace - defaults to sRGB.
     */
    setColorInt(color: ColorInt, colorSpace?: ColorSpace): void;

    /**
     * Sets the current image filter, replacing the existing one if there was one.
     * @param filter
     */
    setImageFilter(filter: ImageFilter): void;

    /**
     * Sets the current mask filter, replacing the existing one if there was one.
     * @param filter
     */
    setMaskFilter(filter: MaskFilter): void;

    /**
     * Sets the current path effect, replacing the existing one if there was one.
     * @param effect
     */
    setPathEffect(effect: PathEffect): void;

    /**
     * Sets the current shader, replacing the existing one if there was one.
     * @param shader
     */
    setShader(shader: Shader): void;

    /**
     * Sets the geometry drawn at the beginning and end of strokes.
     * @param cap
     */
    setStrokeCap(cap: StrokeCap): void;

    /**
     * Sets the geometry drawn at the corners of strokes.
     * @param join
     */
    setStrokeJoin(join: StrokeJoin): void;

    /**
     * Sets the limit at which a sharp corner is drawn beveled.
     * @param limit
     */
    setStrokeMiter(limit: number): void;

    /**
     * Sets the thickness of the pen used to outline the shape.
     * @param width
     */
    setStrokeWidth(width: number): void;

    /**
     * Sets whether the geometry is filled or stroked.
     * @param style
     */
    setStyle(style: PaintStyle): void;
}

/**
 * See SkPath.h for more information on this class.
 */
export interface Path extends EmbindObject<Path> {
    /**
     * Appends arc to Path, as the start of new contour. Arc added is part of ellipse
     * bounded by oval, from startAngle through sweepAngle. Both startAngle and
     * sweepAngle are measured in degrees, where zero degrees is aligned with the
     * positive x-axis, and positive sweeps extends arc clockwise.
     * Returns the modified path for easier chaining.
     * @param oval
     * @param startAngle
     * @param sweepAngle
     */
    addArc(oval: InputRect, startAngle: AngleInDegrees, sweepAngle: AngleInDegrees): Path;

    /**
     * Adds oval to Path, appending kMove_Verb, four kConic_Verb, and kClose_Verb.
     * Oval is upright ellipse bounded by Rect oval with radii equal to half oval width
     * and half oval height. Oval begins at start and continues clockwise by default.
     * Returns the modified path for easier chaining.
     * @param oval
     * @param isCCW - if the path should be drawn counter-clockwise or not
     * @param startIndex - index of initial point of ellipse
     */
    addOval(oval: InputRect, isCCW?: boolean, startIndex?: number): Path;

    /**
     * Takes 1, 2, 7, or 10 required args, where the first arg is always the path.
     * The last arg is an optional boolean and chooses between add or extend mode.
     * The options for the remaining args are:
     *   - an array of 6 or 9 parameters (perspective is optional)
     *   - the 9 parameters of a full matrix or
     *     the 6 non-perspective params of a matrix.
     * Returns the modified path for easier chaining (or null if params were incorrect).
     * @param args
     */
    addPath(...args: any[]): Path | null;

    /**
     * Adds contour created from array of n points, adding (count - 1) line segments.
     * Contour added starts at pts[0], then adds a line for every additional point
     * in pts array. If close is true, appends kClose_Verb to Path, connecting
     * pts[count - 1] and pts[0].
     * Returns the modified path for easier chaining.
     * @param points
     * @param close - if true, will add a line connecting last point to the first point.
     */
    addPoly(points: InputFlattenedPointArray, close: boolean): Path;

    /**
     * Adds Rect to Path, appending kMove_Verb, three kLine_Verb, and kClose_Verb,
     * starting with top-left corner of Rect; followed by top-right, bottom-right,
     * and bottom-left if isCCW is false; or followed by bottom-left,
     * bottom-right, and top-right if isCCW is true.
     * Returns the modified path for easier chaining.
     * @param rect
     * @param isCCW
     */
    addRect(rect: InputRect, isCCW?: boolean): Path;

    /**
     * Adds rrect to Path, creating a new closed contour.
     * Returns the modified path for easier chaining.
     * @param rrect
     * @param isCCW
     */
    addRRect(rrect: InputRRect, isCCW?: boolean): Path;

    /**
     * Adds the given verbs and associated points/weights to the path. The process
     * reads the first verb from verbs and then the appropriate number of points from the
     * FlattenedPointArray (e.g. 2 points for moveTo, 4 points for quadTo, etc). If the verb is
     * a conic, a weight will be read from the WeightList.
     * Returns the modified path for easier chaining
     * @param verbs - the verbs that create this path, in the order of being drawn.
     * @param points - represents n points with 2n floats.
     * @param weights - used if any of the verbs are conics, can be omitted otherwise.
     */
    addVerbsPointsWeights(verbs: VerbList, points: InputFlattenedPointArray,
                          weights?: WeightList): Path;

    /**
     * Adds an arc to this path, emulating the Canvas2D behavior.
     * Returns the modified path for easier chaining.
     * @param x
     * @param y
     * @param radius
     * @param startAngle
     * @param endAngle
     * @param isCCW
     */
    arc(x: number, y: number, radius: number, startAngle: AngleInRadians, endAngle: AngleInRadians,
        isCCW?: boolean): Path;

    /**
     * Appends arc to Path. Arc added is part of ellipse
     * bounded by oval, from startAngle through sweepAngle. Both startAngle and
     * sweepAngle are measured in degrees, where zero degrees is aligned with the
     * positive x-axis, and positive sweeps extends arc clockwise.
     * Returns the modified path for easier chaining.
     * @param oval
     * @param startAngle
     * @param endAngle
     * @param forceMoveTo
     */
    arcToOval(oval: InputRect, startAngle: AngleInDegrees, endAngle: AngleInDegrees,
              forceMoveTo: boolean): Path;

    /**
     * Appends arc to Path. Arc is implemented by one or more conics weighted to
     * describe part of oval with radii (rx, ry) rotated by xAxisRotate degrees. Arc
     * curves from last Path Point to (x, y), choosing one of four possible routes:
     * clockwise or counterclockwise, and smaller or larger. See SkPath.h for more details.
     * Returns the modified path for easier chaining.
     * @param rx
     * @param ry
     * @param xAxisRotate
     * @param useSmallArc
     * @param isCCW
     * @param x
     * @param y
     */
    arcToRotated(rx: number, ry: number, xAxisRotate: AngleInDegrees, useSmallArc: boolean,
                 isCCW: boolean, x: number, y: number): Path;

    /**
     * Appends arc to Path, after appending line if needed. Arc is implemented by conic
     * weighted to describe part of circle. Arc is contained by tangent from
     * last Path point to (x1, y1), and tangent from (x1, y1) to (x2, y2). Arc
     * is part of circle sized to radius, positioned so it touches both tangent lines.
     * Returns the modified path for easier chaining.
     * @param x1
     * @param y1
     * @param x2
     * @param y2
     * @param radius
     */
    arcToTangent(x1: number, y1: number, x2: number, y2: number, radius: number): Path;

    /**
     * Appends CLOSE_VERB to Path. A closed contour connects the first and last point
     * with a line, forming a continuous loop.
     * Returns the modified path for easier chaining.
     */
    close(): Path;

    /**
     * Returns minimum and maximum axes values of the lines and curves in Path.
     * Returns (0, 0, 0, 0) if Path contains no points.
     * Returned bounds width and height may be larger or smaller than area affected
     * when Path is drawn.
     *
     * Behaves identically to getBounds() when Path contains
     * only lines. If Path contains curves, computed bounds includes
     * the maximum extent of the quad, conic, or cubic; is slower than getBounds();
     * and unlike getBounds(), does not cache the result.
     * @param outputArray - if provided, the bounding box will be copied into this array instead of
     *                      allocating a new one.
     */
    computeTightBounds(outputArray?: Rect): Rect;

    /**
     * Adds conic from last point towards (x1, y1), to (x2, y2), weighted by w.
     * If Path is empty, or path is closed, the last point is set to (0, 0)
     * before adding conic.
     * Returns the modified path for easier chaining.
     * @param x1
     * @param y1
     * @param x2
     * @param y2
     * @param w
     */
    conicTo(x1: number, y1: number, x2: number, y2: number, w: number): Path;

    /**
     * Returns true if the point (x, y) is contained by Path, taking into
     * account FillType.
     * @param x
     * @param y
     */
    contains(x: number, y: number): boolean;

    /**
     * Returns a copy of this Path.
     */
    copy(): Path;

    /**
     * Returns the number of points in this path. Initially zero.
     */
    countPoints(): number;

    /**
     *  Adds cubic from last point towards (x1, y1), then towards (x2, y2), ending at
     * (x3, y3). If Path is empty, or path is closed, the last point is set to
     * (0, 0) before adding cubic.
     * @param cpx1
     * @param cpy1
     * @param cpx2
     * @param cpy2
     * @param x
     * @param y
     */
    cubicTo(cpx1: number, cpy1: number, cpx2: number, cpy2: number, x: number, y: number): Path;

    /**
     * Changes this path to be the dashed version of itself. This is the same effect as creating
     * a DashPathEffect and calling filterPath on this path.
     * @param on
     * @param off
     * @param phase
     */
    dash(on: number, off: number, phase: number): boolean;

    /**
     * Returns true if other path is equal to this path.
     * @param other
     */
    equals(other: Path): boolean;

    /**
     * Returns minimum and maximum axes values of Point array.
     * Returns (0, 0, 0, 0) if Path contains no points. Returned bounds width and height may
     * be larger or smaller than area affected when Path is drawn.
     * @param outputArray - if provided, the bounding box will be copied into this array instead of
     *                      allocating a new one.
     */
    getBounds(outputArray?: Rect): Rect;

    /**
     * Return the FillType for this path.
     */
    getFillType(): FillType;

    /**
     * Returns the Point at index in Point array. Valid range for index is
     * 0 to countPoints() - 1.
     * @param index
     * @param outputArray - if provided, the point will be copied into this array instead of
     *                      allocating a new one.
     */
    getPoint(index: number, outputArray?: Point): Point;

    /**
     * Returns true if there are no verbs in the path.
     */
    isEmpty(): boolean;

    /**
     * Returns true if the path is volatile; it will not be altered or discarded
     * by the caller after it is drawn. Path by default have volatile set false, allowing
     * Surface to attach a cache of data which speeds repeated drawing. If true, Surface
     * may not speed repeated drawing.
     */
    isVolatile(): boolean;

    /**
     * Adds line from last point to (x, y). If Path is empty, or last path is closed,
     * last point is set to (0, 0) before adding line.
     * Returns the modified path for easier chaining.
     * @param x
     * @param y
     */
    lineTo(x: number, y: number): Path;

    /**
     * Returns a new path that covers the same area as the original path, but with the
     * Winding FillType. This may re-draw some contours in the path as counter-clockwise
     * instead of clockwise to achieve that effect. If such a transformation cannot
     * be done, null is returned.
     */
    makeAsWinding(): Path | null;

    /**
     * Adds beginning of contour at the given point.
     * Returns the modified path for easier chaining.
     * @param x
     * @param y
     */
    moveTo(x: number, y: number): Path;

    /**
     * Translates all the points in the path by dx, dy.
     * Returns the modified path for easier chaining.
     * @param dx
     * @param dy
     */
    offset(dx: number, dy: number): Path;

    /**
     * Combines this path with the other path using the given PathOp. Returns false if the operation
     * fails.
     * @param other
     * @param op
     */
    op(other: Path, op: PathOp): boolean;

    /**
     * Adds quad from last point towards (x1, y1), to (x2, y2).
     * If Path is empty, or path is closed, last point is set to (0, 0) before adding quad.
     * Returns the modified path for easier chaining.
     * @param x1
     * @param y1
     * @param x2
     * @param y2
     */
    quadTo(x1: number, y1: number, x2: number, y2: number): Path;

    /**
     * Relative version of arcToRotated.
     * @param rx
     * @param ry
     * @param xAxisRotate
     * @param useSmallArc
     * @param isCCW
     * @param dx
     * @param dy
     */
    rArcTo(rx: number, ry: number, xAxisRotate: AngleInDegrees, useSmallArc: boolean,
           isCCW: boolean, dx: number, dy: number): Path;

    /**
     * Relative version of conicTo.
     * @param dx1
     * @param dy1
     * @param dx2
     * @param dy2
     * @param w
     */
    rConicTo(dx1: number, dy1: number, dx2: number, dy2: number, w: number): Path;

    /**
     * Relative version of cubicTo.
     * @param cpx1
     * @param cpy1
     * @param cpx2
     * @param cpy2
     * @param x
     * @param y
     */
    rCubicTo(cpx1: number, cpy1: number, cpx2: number, cpy2: number, x: number, y: number): Path;

    /**
     * Sets Path to its initial state.
     * Removes verb array, point array, and weights, and sets FillType to Winding.
     * Internal storage associated with Path is released
     */
    reset(): void;

    /**
     * Sets Path to its initial state.
     * Removes verb array, point array, and weights, and sets FillType to Winding.
     * Internal storage associated with Path is *not* released.
     * Use rewind() instead of reset() if Path storage will be reused and performance
     * is critical.
     */
    rewind(): void;

    /**
     * Relative version of lineTo.
     * @param x
     * @param y
     */
    rLineTo(x: number, y: number): Path;

    /**
     * Relative version of moveTo.
     * @param x
     * @param y
     */
    rMoveTo(x: number, y: number): Path;

    /**
     * Relative version of quadTo.
     * @param x1
     * @param y1
     * @param x2
     * @param y2
     */
    rQuadTo(x1: number, y1: number, x2: number, y2: number): Path;

    /**
     * Sets FillType, the rule used to fill Path.
     * @param fill
     */
    setFillType(fill: FillType): void;

    /**
     * Specifies whether Path is volatile; whether it will be altered or discarded
     * by the caller after it is drawn. Path by default have volatile set false.
     *
     * Mark animating or temporary paths as volatile to improve performance.
     * Mark unchanging Path non-volatile to improve repeated rendering.
     * @param volatile
     */
    setIsVolatile(volatile: boolean): void;

    /**
     * Set this path to a set of non-overlapping contours that describe the
     * same area as the original path.
     * The curve order is reduced where possible so that cubics may
     * be turned into quadratics, and quadratics maybe turned into lines.
     *
     * Returns true if operation was able to produce a result.
     */
    simplify(): boolean;

    /**
     * Turns this path into the filled equivalent of the stroked path. Returns null if the operation
     * fails (e.g. the path is a hairline).
     * @param opts - describe how stroked path should look.
     */
    stroke(opts?: StrokeOpts): Path | null;

    /**
     * Serializes the contents of this path as a series of commands.
     * The first item will be a verb, followed by any number of arguments needed. Then it will
     * be followed by another verb, more arguments and so on.
     */
    toCmds(): Float32Array;

    /**
     * Returns this path as an SVG string.
     */
    toSVGString(): string;

    /**
     * Takes a 3x3 matrix as either an array or as 9 individual params.
     * @param args
     */
    transform(...args: any[]): Path;

    /**
     * Take start and stop "t" values (values between 0...1), and modify this path such that
     * it is a subset of the original path.
     * The trim values apply to the entire path, so if it contains several contours, all of them
     * are including in the calculation.
     * Null is returned if either input value is NaN.
     * @param startT - a value in the range [0.0, 1.0]. 0.0 is the beginning of the path.
     * @param stopT  - a value in the range [0.0, 1.0]. 1.0 is the end of the path.
     * @param isComplement
     */
    trim(startT: number, stopT: number, isComplement: boolean): Path | null;
}

/**
 * See SkPathEffect.h for more on this class. The objects are opaque.
 */
export type PathEffect = EmbindObject<PathEffect>;

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
    serialize(): Uint8Array | null;
}

export interface PictureRecorder extends EmbindObject<PictureRecorder> {
    /**
     * Returns a canvas on which to draw. When done drawing, call finishRecordingAsPicture()
     *
     * @param bounds - a rect to cull the results.
     */
    beginRecording(bounds: InputRect): Canvas;

    /**
     * Returns the captured draw commands as a picture and invalidates the canvas returned earlier.
     */
    finishRecordingAsPicture(): SkPicture;
}

/**
 * See SkRuntimeEffect.h for more details.
 */
export interface RuntimeEffect extends EmbindObject<RuntimeEffect> {
    /**
     * Returns a shader executed using the given uniform data.
     * @param uniforms
     * @param isOpaque
     * @param localMatrix
     */
    makeShader(uniforms: Float32Array | number[], isOpaque?: boolean,
               localMatrix?: InputMatrix): Shader;

    /**
     * Returns a shader executed using the given uniform data and the children as inputs.
     * @param uniforms
     * @param isOpaque
     * @param children
     * @param localMatrix
     */
    makeShaderWithChildren(uniforms: Float32Array | number[], isOpaque?: boolean,
                           children?: Shader[], localMatrix?: InputMatrix): Shader;

    /**
     * Returns the nth uniform from the effect.
     * @param index
     */
    getUniform(index: number): SkSLUniform;

    /**
     * Returns the number of uniforms on the effect.
     */
    getUniformCount(): number;

    /**
     * Returns the total number of floats across all uniforms on the effect. This is the length
     * of the uniforms array expected by makeShader. For example, an effect with a single float3
     * uniform, would return 1 from `getUniformCount()`, but 3 from `getUniformFloatCount()`.
     */
    getUniformFloatCount(): number;

    /**
     * Returns the name of the nth effect uniform.
     * @param index
     */
    getUniformName(index: number): string;
}

/**
 * See SkShader.h for more on this class. The objects are opaque.
 */
export type Shader = EmbindObject<Shader>;

export interface Surface extends EmbindObject<Surface> {
    /**
     * A convenient way to draw exactly once on the canvas associated with this surface.
     * This requires an environment where a global function called requestAnimationFrame is
     * available (e.g. on the web, not on Node). Users do not need to flush the surface,
     * or delete/dispose of it as that is taken care of automatically with this wrapper.
     *
     * Node users should call getCanvas() and work with that canvas directly.
     */
    drawOnce(drawFrame: (_: Canvas) => void): void;

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
    getCanvas(): Canvas;

    /**
     * Returns the height of this surface in pixels.
     */
    height(): number;

    /**
     * Returns the ImageInfo associated with this surface.
     */
    imageInfo(): ImageInfo;

    /**
     * Creates an Image from the provided texture and info. The Image will own the texture;
     * when the image is deleted, the texture will be cleaned up.
     * @param tex
     * @param info - describes the content of the texture.
     */
    makeImageFromTexture(tex: WebGLTexture, info: ImageInfo): Image | null;

    /**
     * Returns a texture-backed image based on the content in src. It uses RGBA_8888, unpremul
     * and SRGB - for more control, use makeImageFromTexture.
     *
     * The underlying texture for this image will be created immediately from src, so
     * it can be disposed of after this call. This image will *only* be usable for this
     * surface (because WebGL textures are not transferable to other WebGL contexts).
     * For an image that can be used across multiple surfaces, at the cost of being lazily
     * loaded, see MakeLazyImageFromTextureSource.
     *
     * Not available for software-backed surfaces.
     * @param src
     * @param info - If provided, will be used to determine the width/height/format of the
     *               source image. If not, sensible defaults will be used.
     */
    makeImageFromTextureSource(src: TextureSource, info?: ImageInfo | PartialImageInfo): Image | null;

    /**
     * Returns current contents of the surface as an Image. This image will be optimized to be
     * drawn to another surface of the same type. For example, if this surface is backed by the
     * GPU, the returned Image will be backed by a GPU texture.
     */
    makeImageSnapshot(bounds?: InputIRect): Image;

    /**
     * Returns a compatible Surface, haring the same raster or GPU properties of the original.
     * The pixels are not shared.
     * @param info - width, height, etc of the Surface.
     */
    makeSurface(info: ImageInfo): Surface;

    /**
     * Returns if this Surface is a GPU-backed surface or not.
     */
    reportBackendTypeIsGPU(): boolean;

    /**
     * A convenient way to draw multiple frames on the canvas associated with this surface.
     * This requires an environment where a global function called requestAnimationFrame is
     * available (e.g. on the web, not on Node). Users do not need to flush the surface,
     * as that is taken care of automatically with this wrapper.
     *
     * Users should probably call surface.requestAnimationFrame in the callback function to
     * draw multiple frames, e.g. of an animation.
     *
     * Node users should call getCanvas() and work with that canvas directly.
     */
    requestAnimationFrame(drawFrame: (_: Canvas) => void): void;

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
 * See SkTextBlob.h for more on this class. The objects are opaque.
 */
export type TextBlob = EmbindObject<TextBlob>;

/**
 * See SkTypeface.h for more on this class. The objects are opaque.
 */
export interface Typeface extends EmbindObject<Typeface> {
    /**
     * Retrieves the glyph ids for each code point in the provided string. Note that glyph IDs
     * are typeface-dependent; different faces may have different ids for the same code point.
     * @param str
     * @param numCodePoints - the number of code points in the string. Defaults to str.length.
     * @param output - if provided, the results will be copied into this array.
     */
    getGlyphIDs(str: string, numCodePoints?: number,
                output?: GlyphIDArray): GlyphIDArray;
}

/**
 * See SkVertices.h for more on this class.
 */
export interface Vertices extends EmbindObject<Vertices> {
    /**
     * Return the bounding area for the vertices.
     * @param outputArray - if provided, the bounding box will be copied into this array instead of
     *                      allocating a new one.
     */
    bounds(outputArray?: Rect): Rect;

    /**
     * Return a unique ID for this vertices object.
     */
    uniqueID(): number;
}

export interface SkottieAnimation extends EmbindObject<SkottieAnimation> {
    /**
     * Returns the animation duration in seconds.
     */
    duration(): number;
    /**
     * Returns the animation frame rate (frames / second).
     */
    fps(): number;

    /**
     * Draws current animation frame. Must call seek or seekFrame first.
     * @param canvas
     * @param dstRect
     */
    render(canvas: Canvas, dstRect?: InputRect): void;

    /**
     * [deprecated] - use seekFrame
     * @param t - value from [0.0, 1.0]; 0 is first frame, 1 is final frame.
     * @param damageRect - will copy damage frame into this if provided.
     */
    seek(t: number, damageRect?: Rect): Rect;

    /**
     * Update the animation state to match |t|, specified as a frame index
     * i.e. relative to duration() * fps().
     *
     * Returns the rectangle that was affected by this animation.
     *
     * @param frame - Fractional values are allowed and meaningful - e.g.
     *                0.0 -> first frame
     *                1.0 -> second frame
     *                0.5 -> halfway between first and second frame
     * @param damageRect - will copy damage frame into this if provided.
     */
    seekFrame(frame: number, damageRect?: Rect): Rect;

    /**
     * Return the size of this animation.
     * @param outputSize - If provided, the size will be copied into here as width, height.
     */
    size(outputSize?: Point): Point;
    version(): string;
}

/**
 * Options used for Path.stroke(). If an option is omitted, a sensible default will be used.
 */
export interface StrokeOpts {
    /** The width of the stroked lines. */
    width?: number;
    miter_limit?: number;
    /**
     * if > 1, increase precision, else if (0 < resScale < 1) reduce precision to
     * favor speed and size
     */
    precision?: number;
    join?: StrokeJoin;
    cap?: StrokeCap;
}

export interface StrutStyle {
    strutEnabled?: boolean;
    fontFamilies?: string[];
    fontStyle?: FontStyle;
    fontSize?: number;
    heightMultiplier?: number;
    halfLeading?: boolean;
    leading?: number;
    forceStrutHeight?: boolean;
}

export interface TextFontFeatures {
    name: string;
    value: number;
}

export interface TextShadow {
    color?: InputColor;
    /**
     * 2d array for x and y offset. Defaults to [0, 0]
     */
    offset?: number[];
    blurRadius?: number;
}

export interface TextStyle {
    backgroundColor?: InputColor;
    color?: InputColor;
    decoration?: number;
    decorationColor?: InputColor;
    decorationThickness?: number;
    decorationStyle?: DecorationStyle;
    fontFamilies?: string[];
    fontFeatures?: TextFontFeatures[];
    fontSize?: number;
    fontStyle?: FontStyle;
    foregroundColor?: InputColor;
    heightMultiplier?: number;
    halfLeading?: boolean;
    letterSpacing?: number;
    locale?: string;
    shadows?: TextShadow[];
    textBaseline?: TextBaseline;
    wordSpacing?: number;
}

export interface TonalColorsInput {
    ambient: InputColor;
    spot: InputColor;
}

export interface TonalColorsOutput {
    ambient: Color;
    spot: Color;
}

export interface TypefaceFontProvider extends EmbindObject<TypefaceFontProvider> {
    /**
     * Registers a given typeface with the given family name (ignoring whatever name the
     * typface has for itself).
     * @param bytes - the raw bytes for a typeface.
     * @param family
     */
    registerFont(bytes: ArrayBuffer | Uint8Array, family: string): void;
}

export interface URange {
    start: number;
    end: number;
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

export interface DefaultConstructor<T> {
    new (): T;
}

export interface ColorMatrixHelpers {
    /**
     * Returns a new ColorMatrix that is the result of multiplying outer*inner
     * @param outer
     * @param inner
     */
    concat(outer: ColorMatrix, inner: ColorMatrix): ColorMatrix;

    /**
     * Returns an identity ColorMatrix.
     */
    identity(): ColorMatrix;

    /**
     * Sets the 4 "special" params that will translate the colors after they are multiplied
     * by the 4x4 matrix.
     * @param m
     * @param dr - delta red
     * @param dg - delta green
     * @param db - delta blue
     * @param da - delta alpha
     */
    postTranslate(m: ColorMatrix, dr: number, dg: number, db: number, da: number): ColorMatrix;

    /**
     * Returns a new ColorMatrix that is rotated around a given axis.
     * @param axis - 0 for red, 1 for green, 2 for blue
     * @param sine - sin(angle)
     * @param cosine - cos(angle)
     */
    rotated(axis: number, sine: number, cosine: number): ColorMatrix;

    /**
     * Returns a new ColorMatrix that scales the colors as specified.
     * @param redScale
     * @param greenScale
     * @param blueScale
     * @param alphaScale
     */
    scaled(redScale: number, greenScale: number, blueScale: number,
           alphaScale: number): ColorMatrix;
}

/**
 * A constructor for making an ImageData that is compatible with the Canvas2D emulation code.
 */
export interface ImageDataConstructor {
    new (width: number, height: number): EmulatedImageData;
    new (pixels: Uint8ClampedArray, width: number, height: number): EmulatedImageData;
}

/**
 * TODO(kjlubick) Make this API return Float32Arrays
 */
export interface Matrix3x3Helpers {
    /**
     * Returns a new identity 3x3 matrix.
     */
    identity(): number[];

    /**
     * Returns the inverse of the given 3x3 matrix or null if it is not invertible.
     * @param m
     */
    invert(m: Matrix3x3 | number[]): number[] | null;

    /**
     * Maps the given 2d points according to the given 3x3 matrix.
     * @param m
     * @param points - the flattened points to map; the results are computed in place on this array.
     */
    mapPoints(m: Matrix3x3 | number[], points: number[]): number[];

    /**
     * Multiplies the provided 3x3 matrices together from left to right.
     * @param matrices
     */
    multiply(...matrices: Array<(Matrix3x3 | number[])>): number[];

    /**
     * Returns a new 3x3 matrix representing a rotation by n radians.
     * @param radians
     * @param px - the X value to rotate around, defaults to 0.
     * @param py - the Y value to rotate around, defaults to 0.
     */
    rotated(radians: AngleInRadians, px?: number, py?: number): number[];

    /**
     * Returns a new 3x3 matrix representing a scale in the x and y directions.
     * @param sx - the scale in the X direction.
     * @param sy - the scale in the Y direction.
     * @param px - the X value to scale from, defaults to 0.
     * @param py - the Y value to scale from, defaults to 0.
     */
    scaled(sx: number, sy: number, px?: number, py?: number): number[];

    /**
     * Returns a new 3x3 matrix representing a scale in the x and y directions.
     * @param kx - the kurtosis in the X direction.
     * @param ky - the kurtosis in the Y direction.
     * @param px - the X value to skew from, defaults to 0.
     * @param py - the Y value to skew from, defaults to 0.
     */
    skewed(kx: number, ky: number, px?: number, py?: number): number[];

    /**
     * Returns a new 3x3 matrix representing a translation in the x and y directions.
     * @param dx
     * @param dy
     */
    translated(dx: number, dy: number): number[];
}

/**
 * See SkM44.h for more details.
 */
export interface Matrix4x4Helpers {
    /**
     * Returns a new identity 4x4 matrix.
     */
    identity(): number[];

    /**
     * Returns the inverse of the given 4x4 matrix or null if it is not invertible.
     * @param matrix
     */
    invert(matrix: Matrix4x4 | number[]): number[] | null;

    /**
     * Return a new 4x4 matrix representing a camera at eyeVec, pointed at centerVec.
     * @param eyeVec
     * @param centerVec
     * @param upVec
     */
    lookat(eyeVec: Vector3, centerVec: Vector3, upVec: Vector3): number[];

    /**
     * Multiplies the provided 4x4 matrices together from left to right.
     * @param matrices
     */
    multiply(...matrices: Array<(Matrix4x4 | number[])>): number[];

    /**
     * Returns the inverse of the given 4x4 matrix or throws if it is not invertible.
     * @param matrix
     */
    mustInvert(matrix: Matrix4x4 | number[]): number[];

    /**
     * Returns a new 4x4 matrix representing a perspective.
     * @param near
     * @param far
     * @param radians
     */
    perspective(near: number, far: number, radians: AngleInRadians): number[];

    /**
     * Returns the value at the specified row and column of the given 4x4 matrix.
     * @param matrix
     * @param row
     * @param col
     */
    rc(matrix: Matrix4x4 | number[], row: number, col: number): number;

    /**
     * Returns a new 4x4 matrix representing a rotation around the provided vector.
     * @param axis
     * @param radians
     */
    rotated(axis: Vector3, radians: AngleInRadians): number[];

    /**
     * Returns a new 4x4 matrix representing a rotation around the provided vector.
     * Rotation is provided redundantly as both sin and cos values.
     * This rotate can be used when you already have the cosAngle and sinAngle values
     * so you don't have to atan(cos/sin) to call roatated() which expects an angle in radians.
     * This does no checking! Behavior for invalid sin or cos values or non-normalized axis vectors
     * is incorrect. Prefer rotated().
     * @param axis
     * @param sinAngle
     * @param cosAngle
     */
    rotatedUnitSinCos(axis: Vector3, sinAngle: number, cosAngle: number): number[];

    /**
     * Returns a new 4x4 matrix representing a scale by the provided vector.
     * @param vec
     */
    scaled(vec: Vector3): number[];

    /**
     * Returns a new 4x4 matrix that sets up a 3D perspective view from a given camera.
     * @param area - describes the viewport. (0, 0, canvas_width, canvas_height) suggested.
     * @param zScale - describes the scale of the z axis. min(width, height)/2 suggested
     * @param cam
     */
    setupCamera(area: InputRect, zScale: number, cam: Camera): number[];

    /**
     * Returns a new 4x4 matrix representing a translation by the provided vector.
     * @param vec
     */
    translated(vec: Vector3): number[];

    /**
     * Returns a new 4x4 matrix that is the transpose of this 4x4 matrix.
     * @param matrix
     */
    transpose(matrix: Matrix4x4 | number[]): number[];
}

export interface ParagraphBuilderFactory {
    /**
     * Creates a ParagraphBuilder using the fonts available from the given font manager.
     * @param style
     * @param fontManager
     */
    Make(style: ParagraphStyle, fontManager: FontMgr): ParagraphBuilder;

    /**
     * Creates a ParagraphBuilder using the fonts available from the given font provider.
     * @param style
     * @param fontSrc
     */
    MakeFromFontProvider(style: ParagraphStyle, fontSrc: TypefaceFontProvider): ParagraphBuilder;

    /**
     * Return a shaped array of lines
     */
    ShapeText(text: string, runs: FontBlock[], width?: number): ShapedLine[];
}

export interface ParagraphStyleConstructor {
    /**
     * Fills out all optional fields with defaults. The emscripten bindings complain if there
     * is a field undefined and it was expecting a float (for example).
     * @param ps
     */
    new(ps: ParagraphStyle): ParagraphStyle;
}

/**
 * See SkColorFilter.h for more.
 */
export interface ColorFilterFactory {
    /**
     * Makes a color filter with the given color and blend mode.
     * @param color
     * @param mode
     */
    MakeBlend(color: InputColor, mode: BlendMode): ColorFilter;

    /**
     * Makes a color filter composing two color filters.
     * @param outer
     * @param inner
     */
    MakeCompose(outer: ColorFilter, inner: ColorFilter): ColorFilter;

    /**
     * Makes a color filter that is linearly interpolated between two other color filters.
     * @param t - a float in the range of 0.0 to 1.0.
     * @param dst
     * @param src
     */
    MakeLerp(t: number, dst: ColorFilter, src: ColorFilter): ColorFilter;

    /**
     * Makes a color filter that converts between linear colors and sRGB colors.
     */
    MakeLinearToSRGBGamma(): ColorFilter;

    /**
     * Creates a color filter using the provided color matrix.
     * @param cMatrix
     */
    MakeMatrix(cMatrix: InputColorMatrix): ColorFilter;

    /**
     * Makes a color filter that converts between sRGB colors and linear colors.
     */
    MakeSRGBToLinearGamma(): ColorFilter;
}

export interface ContourMeasureIterConstructor {
    /**
     * Creates an ContourMeasureIter with the given path.
     * @param path
     * @param forceClosed - if path should be forced close before measuring it.
     * @param resScale - controls the precision of the measure. values > 1 increase the
     *                   precision (and possibly slow down the computation).
     */
    new (path: Path, forceClosed: boolean, resScale: number): ContourMeasureIter;
}

/**
 * See SkFont.h for more.
 */
export interface FontConstructor extends DefaultConstructor<Font> {
    /**
     * Constructs Font with default values with Typeface.
     * @param face
     * @param size - font size in points. If not specified, uses a default value.
     */
    new (face: Typeface | null, size?: number): Font;

    /**
     * Constructs Font with default values with Typeface and size in points,
     * horizontal scale, and horizontal skew. Horizontal scale emulates condensed
     * and expanded fonts. Horizontal skew emulates oblique fonts.
     * @param face
     * @param size
     * @param scaleX
     * @param skewX
     */
    new (face: Typeface | null, size: number, scaleX: number, skewX: number): Font;
}

export interface FontMgrFactory {
    /**
     * Create an FontMgr with the created font data. Returns null if buffers was empty.
     * @param buffers
     */
    FromData(...buffers: ArrayBuffer[]): FontMgr | null;
}

/**
 * See effects/ImageFilters.h for more.
 */
export interface ImageFilterFactory {
    /**
     * Create a filter that blurs its input by the separate X and Y sigmas. The provided tile mode
     * is used when the blur kernel goes outside the input image.
     *
     * @param sigmaX - The Gaussian sigma value for blurring along the X axis.
     * @param sigmaY - The Gaussian sigma value for blurring along the Y axis.
     * @param mode
     * @param input - if null, it will use the dynamic source image (e.g. a saved layer)
     */
    MakeBlur(sigmaX: number, sigmaY: number, mode: TileMode,
             input: ImageFilter | null): ImageFilter;

    /**
     * Create a filter that applies the color filter to the input filter results.
     * @param cf
     * @param input - if null, it will use the dynamic source image (e.g. a saved layer)
     */
    MakeColorFilter(cf: ColorFilter, input: ImageFilter | null): ImageFilter;

    /**
     * Create a filter that composes 'inner' with 'outer', such that the results of 'inner' are
     * treated as the source bitmap passed to 'outer'.
     * If either param is null, the other param will be returned.
     * @param outer
     * @param inner - if null, it will use the dynamic source image (e.g. a saved layer)
     */
    MakeCompose(outer: ImageFilter | null, inner: ImageFilter | null): ImageFilter;

    /**
     * Create a filter that transforms the input image by 'matrix'. This matrix transforms the
     * local space, which means it effectively happens prior to any transformation coming from the
     * Canvas initiating the filtering.
     * @param matr
     * @param sampling
     * @param input - if null, it will use the dynamic source image (e.g. a saved layer)
     */
    MakeMatrixTransform(matr: InputMatrix, sampling: FilterOptions | CubicResampler,
                        input: ImageFilter | null): ImageFilter;
}

/**
 * See SkMaskFilter.h for more details.
 */
export interface MaskFilterFactory {
    /**
     * Create a blur maskfilter
     * @param style
     * @param sigma - Standard deviation of the Gaussian blur to apply. Must be > 0.
     * @param respectCTM - if true the blur's sigma is modified by the CTM.
     */
    MakeBlur(style: BlurStyle, sigma: number, respectCTM: boolean): MaskFilter;
}

/**
 * Contains the ways to create an Path.
 */
export interface PathConstructorAndFactory extends DefaultConstructor<Path> {
    /**
     * Creates a new path from the given list of path commands. If this fails, null will be
     * returned instead.
     * @param cmds
     */
    MakeFromCmds(cmds: InputCommands): Path | null;

    /**
     * Creates a new path by combining the given paths according to op. If this fails, null will
     * be returned instead.
     * @param one
     * @param two
     * @param op
     */
    MakeFromOp(one: Path, two: Path, op: PathOp): Path | null;

    /**
     * Creates a new path from the provided SVG string. If this fails, null will be
     * returned instead.
     * @param str
     */
    MakeFromSVGString(str: string): Path | null;

    /**
     * Creates a new path using the provided verbs and associated points and weights. The process
     * reads the first verb from verbs and then the appropriate number of points from the
     * FlattenedPointArray (e.g. 2 points for moveTo, 4 points for quadTo, etc). If the verb is
     * a conic, a weight will be read from the WeightList.
     * If the data is malformed (e.g. not enough points), the resulting path will be incomplete.
     * @param verbs - the verbs that create this path, in the order of being drawn.
     * @param points - represents n points with 2n floats.
     * @param weights - used if any of the verbs are conics, can be omitted otherwise.
     */
    MakeFromVerbsPointsWeights(verbs: VerbList, points: InputFlattenedPointArray,
                               weights?: WeightList): Path;
}

/**
 * See SkPathEffect.h for more details.
 */
export interface PathEffectFactory {
    /**
     * Returns a PathEffect that can turn sharp corners into rounded corners.
     * @param radius - if <=0, returns null
     */
    MakeCorner(radius: number): PathEffect | null;

    /**
     * Returns a PathEffect that add dashes to the path.
     *
     * See SkDashPathEffect.h for more details.
     *
     * @param intervals - even number of entries with even indicies specifying the length of
     *                    the "on" intervals, and the odd indices specifying the length of "off".
     * @param phase - offset length into the intervals array. Defaults to 0.
     */
    MakeDash(intervals: number[], phase?: number): PathEffect;

    /**
     * Returns a PathEffect that breaks path into segments of segLength length, and randomly move
     * the endpoints away from the original path by a maximum of deviation.
     * @param segLength - length of the subsegments.
     * @param dev - limit of the movement of the endpoints.
     * @param seedAssist - modifies the randomness. See SkDiscretePathEffect.h for more.
     */
    MakeDiscrete(segLength: number, dev: number, seedAssist: number): PathEffect;
}

/**
 * See RuntimeEffect.h for more details.
 */
export interface DebugTrace extends EmbindObject<DebugTrace> {
    writeTrace(): string;
}

export interface TracedShader {
    shader: Shader;
    debugTrace: DebugTrace;
}

export interface RuntimeEffectFactory {
    /**
     * Compiles a RuntimeEffect from the given shader code.
     * @param sksl - Source code for a shader written in SkSL
     * @param callback - will be called with any compilation error. If not provided, errors will
     *                   be printed to console.log().
     */
    Make(sksl: string, callback?: (err: string) => void): RuntimeEffect | null;

    /**
     * Adds debug tracing to an existing RuntimeEffect.
     * @param shader - An already-assembled shader, created with RuntimeEffect.makeShader.
     * @param traceCoordX - the X coordinate of the device-space pixel to trace
     * @param traceCoordY - the Y coordinate of the device-space pixel to trace
     */
    MakeTraced(shader: Shader, traceCoordX: number, traceCoordY: number): TracedShader;
}

/**
 * For more information, see SkShaders.h.
 */
export interface ShaderFactory {
    /**
     * Returns a shader that combines the given shaders with a BlendMode.
     * @param mode
     * @param one
     * @param two
     */
    MakeBlend(mode: BlendMode, one: Shader, two: Shader): Shader;

    /**
     * Returns a shader with a given color and colorspace.
     * @param color
     * @param space
     */
    MakeColor(color: InputColor, space: ColorSpace): Shader;

    /**
     * Returns a shader with Perlin Fractal Noise.
     * See SkPerlinNoiseShader.h for more details
     * @param baseFreqX - base frequency in the X direction; range [0.0, 1.0]
     * @param baseFreqY - base frequency in the Y direction; range [0.0, 1.0]
     * @param octaves
     * @param seed
     * @param tileW - if this and tileH are non-zero, the frequencies will be modified so that the
     *                noise will be tileable for the given size.
     * @param tileH - if this and tileW are non-zero, the frequencies will be modified so that the
     *                noise will be tileable for the given size.
     */
    MakeFractalNoise(baseFreqX: number, baseFreqY: number, octaves: number, seed: number,
                     tileW: number, tileH: number): Shader;

    /**
     * Returns a shader that generates a linear gradient between the two specified points.
     * See SkGradientShader.h for more.
     * @param start
     * @param end
     * @param colors - colors to be distributed between start and end.
     * @param pos - May be null. The relative positions of colors. If supplied must be same length
     *              as colors.
     * @param mode
     * @param localMatrix
     * @param flags - By default gradients will interpolate their colors in unpremul space
     *                and then premultiply each of the results. By setting this to 1, the
     *                gradients will premultiply their colors first, and then interpolate
     *                between them.
     * @param colorSpace
     */
    MakeLinearGradient(start: InputPoint, end: InputPoint, colors: InputFlexibleColorArray,
                       pos: number[] | null, mode: TileMode, localMatrix?: InputMatrix,
                       flags?: number, colorSpace?: ColorSpace): Shader;

    /**
     * Returns a shader that generates a radial gradient given the center and radius.
     * See SkGradientShader.h for more.
     * @param center
     * @param radius
     * @param colors - colors to be distributed between the center and edge.
     * @param pos - May be null. The relative positions of colors. If supplied must be same length
     *              as colors. Range [0.0, 1.0]
     * @param mode
     * @param localMatrix
     * @param flags - 0 to interpolate colors in unpremul, 1 to interpolate colors in premul.
     * @param colorSpace
     */
    MakeRadialGradient(center: InputPoint, radius: number, colors: InputFlexibleColorArray,
                       pos: number[] | null, mode: TileMode, localMatrix?: InputMatrix,
                       flags?: number, colorSpace?: ColorSpace): Shader;

    /**
     * Returns a shader that generates a sweep gradient given a center.
     * See SkGradientShader.h for more.
     * @param cx
     * @param cy
     * @param colors - colors to be distributed around the center, within the provided angles.
     * @param pos - May be null. The relative positions of colors. If supplied must be same length
     *              as colors. Range [0.0, 1.0]
     * @param mode
     * @param localMatrix
     * @param flags - 0 to interpolate colors in unpremul, 1 to interpolate colors in premul.
     * @param startAngle - angle corresponding to 0.0. Defaults to 0 degrees.
     * @param endAngle - angle corresponding to 1.0. Defaults to 360 degrees.
     * @param colorSpace
     */
    MakeSweepGradient(cx: number, cy: number, colors: InputFlexibleColorArray,
                      pos: number[] | null, mode: TileMode, localMatrix?: InputMatrix | null,
                      flags?: number, startAngle?: AngleInDegrees, endAngle?: AngleInDegrees,
                      colorSpace?: ColorSpace): Shader;

    /**
     * Returns a shader with Perlin Turbulence.
     * See SkPerlinNoiseShader.h for more details
     * @param baseFreqX - base frequency in the X direction; range [0.0, 1.0]
     * @param baseFreqY - base frequency in the Y direction; range [0.0, 1.0]
     * @param octaves
     * @param seed
     * @param tileW - if this and tileH are non-zero, the frequencies will be modified so that the
     *                noise will be tileable for the given size.
     * @param tileH - if this and tileW are non-zero, the frequencies will be modified so that the
     *                noise will be tileable for the given size.
     */
    MakeTurbulence(baseFreqX: number, baseFreqY: number, octaves: number, seed: number,
                   tileW: number, tileH: number): Shader;

    /**
     * Returns a shader that generates a conical gradient given two circles.
     * See SkGradientShader.h for more.
     * @param start
     * @param startRadius
     * @param end
     * @param endRadius
     * @param colors
     * @param pos
     * @param mode
     * @param localMatrix
     * @param flags
     * @param colorSpace
     */
    MakeTwoPointConicalGradient(start: InputPoint, startRadius: number, end: InputPoint,
                                endRadius: number, colors: InputFlexibleColorArray,
                                pos: number[] | null, mode: TileMode, localMatrix?: InputMatrix,
                                flags?: number, colorSpace?: ColorSpace): Shader;
}

/**
 * See SkTextBlob.h for more details.
 */
export interface TextBlobFactory {
    /**
     * Return a TextBlob with a single run of text.
     *
     * It does not perform typeface fallback for characters not found in the Typeface.
     * It does not perform kerning or other complex shaping; glyphs are positioned based on their
     * default advances.
     * @param glyphs - if using Malloc'd array, be sure to use CanvasKit.MallocGlyphIDs().
     * @param font
     */
    MakeFromGlyphs(glyphs: InputGlyphIDArray, font: Font): TextBlob;

    /**
     * Returns a TextBlob built from a single run of text with rotation, scale, and translations.
     *
     * It uses the default character-to-glyph mapping from the typeface in the font.
     * @param str
     * @param rsxforms
     * @param font
     */
    MakeFromRSXform(str: string, rsxforms: InputFlattenedRSXFormArray, font: Font): TextBlob;

    /**
     * Returns a TextBlob built from a single run of text with rotation, scale, and translations.
     *
     * @param glyphs - if using Malloc'd array, be sure to use CanvasKit.MallocGlyphIDs().
     * @param rsxforms
     * @param font
     */
    MakeFromRSXformGlyphs(glyphs: InputGlyphIDArray, rsxforms: InputFlattenedRSXFormArray,
                          font: Font): TextBlob;

    /**
     * Return a TextBlob with a single run of text.
     *
     * It uses the default character-to-glyph mapping from the typeface in the font.
     * It does not perform typeface fallback for characters not found in the Typeface.
     * It does not perform kerning or other complex shaping; glyphs are positioned based on their
     * default advances.
     * @param str
     * @param font
     */
    MakeFromText(str: string, font: Font): TextBlob;

    /**
     * Returns a TextBlob that has the glyphs following the contours of the given path.
     *
     * It is a convenience wrapper around MakeFromRSXform and ContourMeasureIter.
     * @param str
     * @param path
     * @param font
     * @param initialOffset - the length in pixels to start along the path.
     */
    MakeOnPath(str: string, path: Path, font: Font, initialOffset?: number): TextBlob;
}

export interface TextStyleConstructor {
    /**
     * Fills out all optional fields with defaults. The emscripten bindings complain if there
     * is a field undefined and it was expecting a float (for example).
     * @param ts
     */
    new(ts: TextStyle): TextStyle;
}

export interface TypefaceFactory {
    /**
     * Create a typeface using Freetype from the specified bytes and return it. CanvasKit supports
     * .ttf, .woff and .woff2 fonts. It returns null if the bytes cannot be decoded.
     * @param fontData
     */
    MakeFreeTypeFaceFromData(fontData: ArrayBuffer): Typeface | null;
}

export interface TypefaceFontProviderFactory {
    /**
     * Return an empty TypefaceFontProvider
     */
    Make(): TypefaceFontProvider;
}

/**
 * Functions for manipulating vectors. It is Loosely based off of SkV3 in SkM44.h but Skia
 * also has SkVec2 and Skv4. This combines them and works on vectors of any length.
 */
export interface VectorHelpers {
    /**
     * Adds 2 vectors together, term by term, returning a new Vector.
     * @param a
     * @param b
     */
    add(a: VectorN, b: VectorN): VectorN;

    /**
     * Returns the cross product of the two vectors. Only works for length 3.
     * @param a
     * @param b
     */
    cross(a: Vector3, b: Vector3): Vector3;

    /**
     * Returns the length(sub(a, b))
     * @param a
     * @param b
     */
    dist(a: VectorN, b: VectorN): number;

    /**
     * Returns the dot product of the two vectors.
     * @param a
     * @param b
     */
    dot(a: VectorN, b: VectorN): number;

    /**
     * Returns the length of this vector, which is always positive.
     * @param v
     */
    length(v: VectorN): number;

    /**
     * Returns the length squared of this vector.
     * @param v
     */
    lengthSquared(v: VectorN): number;

    /**
     * Returns a new vector which is v multiplied by the scalar s.
     * @param v
     * @param s
     */
    mulScalar(v: VectorN, s: number): VectorN;

    /**
     * Returns a normalized vector.
     * @param v
     */
    normalize(v: VectorN): VectorN;

    /**
     * Subtracts vector b from vector a (termwise).
     * @param a
     * @param b
     */
    sub(a: VectorN, b: VectorN): VectorN;
}

/**
 * A PosTan is a Float32Array of length 4, representing a position and a tangent vector. In order,
 * the values are [px, py, tx, ty].
 */
export type PosTan = Float32Array;
/**
 * An Color is represented by 4 floats, typically with values between 0 and 1.0. In order,
 * the floats correspond to red, green, blue, alpha.
 */
export type Color = Float32Array;
export type ColorInt = number; // deprecated, prefer Color
/**
 * An ColorMatrix is a 4x4 color matrix that transforms the 4 color channels
 * with a 1x4 matrix that post-translates those 4 channels.
 * For example, the following is the layout with the scale (S) and post-transform
 * (PT) items indicated.
 * RS,  0,  0,  0 | RPT
 *  0, GS,  0,  0 | GPT
 *  0,  0, BS,  0 | BPT
 *  0,  0,  0, AS | APT
 */
export type ColorMatrix = Float32Array;
/**
 * An IRect is represented by 4 ints. In order, the ints correspond to left, top,
 * right, bottom. See Rect.h for more
 */
export type IRect = Int32Array;
/**
 * An Point is represented by 2 floats: (x, y).
 */
export type Point = Float32Array;
/**
 * An Rect is represented by 4 floats. In order, the floats correspond to left, top,
 * right, bottom. See Rect.h for more
 */
export type Rect = Float32Array;
/**
 * An RRect (rectangle with rounded corners) is represented by 12 floats. In order, the floats
 * correspond to left, top, right, bottom and then in pairs, the radiusX, radiusY for upper-left,
 * upper-right, lower-right, lower-left. See RRect.h for more.
 */
export type RRect = Float32Array;

export type WebGLContextHandle = number;
export type AngleInDegrees = number;
export type AngleInRadians = number;
export type SaveLayerFlag = number;

export type TypedArrayConstructor = Float32ArrayConstructor | Int32ArrayConstructor |
    Int16ArrayConstructor | Int8ArrayConstructor | Uint32ArrayConstructor |
    Uint16ArrayConstructor | Uint8ArrayConstructor;
export type TypedArray = Float32Array | Int32Array | Int16Array | Int8Array | Uint32Array |
    Uint16Array | Uint8Array;

export type ColorIntArray = MallocObj | Uint32Array | number[];
/**
 * FlattenedPointArray represents n points by 2*n float values. In order, the values should
 * be the x, y for each point.
 */
export type FlattenedPointArray = Float32Array;
/**
 * FlattenedRectangleArray represents n rectangles by 4*n float values. In order, the values should
 * be the top, left, right, bottom point for each rectangle.
 */
export type FlattenedRectangleArray = Float32Array;

export type GlyphIDArray = Uint16Array;
/**
 * A command is a verb and then any arguments needed to fulfill that path verb.
 * InputCommands is a flattened structure of one or more of these.
 * Examples:
 *   [CanvasKit.MOVE_VERB, 0, 10,
 *    CanvasKit.QUAD_VERB, 20, 50, 45, 60,
 *    CanvasKit.LINE_VERB, 30, 40]
 */
export type InputCommands = MallocObj | Float32Array | number[];
/**
 * VerbList holds verb constants like CanvasKit.MOVE_VERB, CanvasKit.CUBIC_VERB.
 */
export type VerbList = MallocObj | Uint8Array | number[];
/**
 * WeightList holds weights for conics when making paths.
 */
export type WeightList = MallocObj | Float32Array | number[];

export type Matrix4x4 = Float32Array;
export type Matrix3x3 = Float32Array;
export type Matrix3x2 = Float32Array;
/**
 * Vector3 represents an x, y, z coordinate or vector. It has length 3.
 */
export type Vector3 = number[];

/**
 * VectorN represents a vector of length n.
 */
export type VectorN = number[];

/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as colors.
 * Length 4.
 */
export type InputColor = MallocObj | Color | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as color matrices.
 * Length 20.
 */
export type InputColorMatrix = MallocObj | ColorMatrix | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as glyph IDs.
 * Length n for n glyph IDs.
 */
export type InputGlyphIDArray = MallocObj | GlyphIDArray | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as flattened points.
 * Length 2 * n for n points.
 */
export type InputFlattenedPointArray = MallocObj | FlattenedPointArray | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as flattened rectangles.
 * Length 4 * n for n rectangles.
 */
export type InputFlattenedRectangleArray = MallocObj | FlattenedRectangleArray | number[];
/**
 * Some APIs accept a flattened array of colors in one of two ways - groups of 4 float values for
 * r, g, b, a or just integers that have 8 bits for each these. CanvasKit will detect which one
 * it is and act accordingly. Additionally, this can be an array of Float32Arrays of length 4
 * (e.g. Color). This is convenient for things like gradients when matching up colors to stops.
 */
export type InputFlexibleColorArray = Float32Array | Uint32Array | Float32Array[];
/**
 * CanvasKit APIs accept a Float32Array or a normal array (of length 2) as a Point.
 */
export type InputPoint = Point | number[];
/**
 * CanvasKit APIs accept all of these matrix types. Under the hood, we generally use 4x4 matrices.
 */
export type InputMatrix = MallocObj | Matrix4x4 | Matrix3x3 | Matrix3x2 | DOMMatrix | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as rectangles.
 * Length 4.
 */
export type InputRect = MallocObj | Rect | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as (int) rectangles.
 * Length 4.
 */
export type InputIRect = MallocObj | IRect | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as rectangles with
 * rounded corners. Length 12.
 */
export type InputRRect = MallocObj | RRect | number[];
/**
 * This represents n RSXforms by 4*n float values. In order, the values should
 * be scos, ssin, tx, ty for each RSXForm. See RSXForm.h for more details.
 */
export type InputFlattenedRSXFormArray = MallocObj | Float32Array | number[];
/**
 * CanvasKit APIs accept normal arrays, typed arrays, or Malloc'd memory as a vector of 3 floats.
 * For example, this is the x, y, z coordinates.
 */
export type InputVector3 = MallocObj | Vector3 | Float32Array;
/**
 * These are the types that webGL's texImage2D supports as a way to get data from as a texture.
 * Not listed, but also supported are https://developer.mozilla.org/en-US/docs/Web/API/VideoFrame
 */
export type TextureSource = TypedArray | HTMLImageElement | HTMLVideoElement | ImageData | ImageBitmap;

export type AlphaType = EmbindEnumEntity;
export type BlendMode = EmbindEnumEntity;
export type BlurStyle = EmbindEnumEntity;
export type ClipOp = EmbindEnumEntity;
export type ColorSpace = EmbindObject<ColorSpace>;
export type ColorType = EmbindEnumEntity;
export type EncodedImageFormat = EmbindEnumEntity;
export type FillType = EmbindEnumEntity;
export type FilterMode = EmbindEnumEntity;
export type FontEdging = EmbindEnumEntity;
export type FontHinting = EmbindEnumEntity;
export type MipmapMode = EmbindEnumEntity;
export type PaintStyle = EmbindEnumEntity;
export type PathOp = EmbindEnumEntity;
export type PointMode = EmbindEnumEntity;
export type StrokeCap = EmbindEnumEntity;
export type StrokeJoin = EmbindEnumEntity;
export type TileMode = EmbindEnumEntity;
export type VertexMode = EmbindEnumEntity;

export type Affinity = EmbindEnumEntity;
export type DecorationStyle = EmbindEnumEntity;
export type FontSlant = EmbindEnumEntity;
export type FontWeight = EmbindEnumEntity;
export type FontWidth = EmbindEnumEntity;
export type PlaceholderAlignment = EmbindEnumEntity;
export type RectHeightStyle = EmbindEnumEntity;
export type RectWidthStyle = EmbindEnumEntity;
export type TextAlign = EmbindEnumEntity;
export type TextBaseline = EmbindEnumEntity;
export type TextDirection = EmbindEnumEntity;
export type TextHeightBehavior = EmbindEnumEntity;

export interface AffinityEnumValues extends EmbindEnum {
    Upstream: Affinity;
    Downstream: Affinity;
}

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

export interface BlurStyleEnumValues extends EmbindEnum {
    Normal: BlurStyle;
    Solid: BlurStyle;
    Outer: BlurStyle;
    Inner: BlurStyle;
}

export interface ClipOpEnumValues extends EmbindEnum {
    Difference: ClipOp;
    Intersect: ClipOp;
}

/**
 * The currently supported color spaces. These are all singleton values.
 */
export interface ColorSpaceEnumValues { // not a typical enum, but effectively like one.
    // These are all singleton values - don't call delete on them.
    readonly SRGB: ColorSpace;
    readonly DISPLAY_P3: ColorSpace;
    readonly ADOBE_RGB: ColorSpace;

    /**
     * Returns true if the two color spaces are equal.
     * @param a
     * @param b
     */
    Equals(a: ColorSpace, b: ColorSpace): boolean;
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

export interface DecorationStyleEnumValues extends EmbindEnum {
    Solid: DecorationStyle;
    Double: DecorationStyle;
    Dotted: DecorationStyle;
    Dashed: DecorationStyle;
    Wavy: DecorationStyle;
}

export interface FillTypeEnumValues extends EmbindEnum {
    Winding: FillType;
    EvenOdd: FillType;
}

export interface FilterModeEnumValues extends EmbindEnum {
    Linear: FilterMode;
    Nearest: FilterMode;
}

export interface FontEdgingEnumValues extends EmbindEnum {
    Alias: FontEdging;
    AntiAlias: FontEdging;
    SubpixelAntiAlias: FontEdging;
}

export interface FontHintingEnumValues extends EmbindEnum {
    None: FontHinting;
    Slight: FontHinting;
    Normal: FontHinting;
    Full: FontHinting;
}

export interface FontSlantEnumValues extends EmbindEnum {
    Upright: FontSlant;
    Italic: FontSlant;
    Oblique: FontSlant;
}

export interface FontWeightEnumValues extends EmbindEnum {
    Invisible: FontWeight;
    Thin: FontWeight;
    ExtraLight: FontWeight;
    Light: FontWeight;
    Normal: FontWeight;
    Medium: FontWeight;
    SemiBold: FontWeight;
    Bold: FontWeight;
    ExtraBold: FontWeight;
    Black: FontWeight;
    ExtraBlack: FontWeight;
}

export interface FontWidthEnumValues extends EmbindEnum {
    UltraCondensed: FontWidth;
    ExtraCondensed: FontWidth;
    Condensed: FontWidth;
    SemiCondensed: FontWidth;
    Normal: FontWidth;
    SemiExpanded: FontWidth;
    Expanded: FontWidth;
    ExtraExpanded: FontWidth;
    UltraExpanded: FontWidth;
}

/*
 *  These values can be OR'd together
 */
export interface GlyphRunFlagValues {
    IsWhiteSpace: number;
}

export interface ImageFormatEnumValues extends EmbindEnum {
    // TODO(kjlubick) When these are compiled in depending on the availability of the codecs,
    //   be sure to make these nullable.
    PNG: EncodedImageFormat;
    JPEG: EncodedImageFormat;
    WEBP: EncodedImageFormat;
}

export interface MipmapModeEnumValues extends EmbindEnum {
    None: MipmapMode;
    Nearest: MipmapMode;
    Linear: MipmapMode;
}

export interface PaintStyleEnumValues extends EmbindEnum {
    Fill: PaintStyle;
    Stroke: PaintStyle;
}

export interface PathOpEnumValues extends EmbindEnum {
    Difference: PathOp;
    Intersect: PathOp;
    Union: PathOp;
    XOR: PathOp;
    ReverseDifference: PathOp;
}

export interface PlaceholderAlignmentEnumValues extends EmbindEnum {
    Baseline: PlaceholderAlignment;
    AboveBaseline: PlaceholderAlignment;
    BelowBaseline: PlaceholderAlignment;
    Top: PlaceholderAlignment;
    Bottom: PlaceholderAlignment;
    Middle: PlaceholderAlignment;
}

export interface PointModeEnumValues extends EmbindEnum {
    Points: PointMode;
    Lines: PointMode;
    Polygon: PointMode;
}

export interface RectHeightStyleEnumValues extends EmbindEnum {
    Tight: RectHeightStyle;
    Max: RectHeightStyle;
    IncludeLineSpacingMiddle: RectHeightStyle;
    IncludeLineSpacingTop: RectHeightStyle;
    IncludeLineSpacingBottom: RectHeightStyle;
    Strut: RectHeightStyle;
}

export interface RectWidthStyleEnumValues extends EmbindEnum {
    Tight: RectWidthStyle;
    Max: RectWidthStyle;
}

export interface StrokeCapEnumValues extends EmbindEnum {
    Butt: StrokeCap;
    Round: StrokeCap;
    Square: StrokeCap;
}

export interface StrokeJoinEnumValues extends EmbindEnum {
    Bevel: StrokeJoin;
    Miter: StrokeJoin;
    Round: StrokeJoin;
}

export interface TextAlignEnumValues extends EmbindEnum {
    Left: TextAlign;
    Right: TextAlign;
    Center: TextAlign;
    Justify: TextAlign;
    Start: TextAlign;
    End: TextAlign;
}

export interface TextBaselineEnumValues extends EmbindEnum {
    Alphabetic: TextBaseline;
    Ideographic: TextBaseline;
}

export interface TextDirectionEnumValues extends EmbindEnum {
    LTR: TextDirection;
    RTL: TextDirection;
}

export interface TextHeightBehaviorEnumValues extends EmbindEnum {
    All: TextHeightBehavior;
    DisableFirstAscent: TextHeightBehavior;
    DisableLastDescent: TextHeightBehavior;
    DisableAll: TextHeightBehavior;
}

export interface TileModeEnumValues extends EmbindEnum {
    Clamp: TileMode;
    Decal: TileMode;
    Mirror: TileMode;
    Repeat: TileMode;
}

export interface VertexModeEnumValues extends EmbindEnum {
    Triangles: VertexMode;
    TrianglesStrip: VertexMode;
    TriangleFan: VertexMode;
}
