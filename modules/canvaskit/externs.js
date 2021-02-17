/*
 * This externs file prevents the Closure JS compiler from minifying away
 * names of objects created by Emscripten.
 * Basically, by defining empty objects and functions here, Closure will
 * know not to rename them.  This is needed because of our pre-js files,
 * that is, the JS we hand-write to bundle into the output. That JS will be
 * hit by the closure compiler and thus needs to know about what functions
 * have special names and should not be minified.
 *
 * Emscripten does not support automatically generating an externs file, so we
 * do it by hand. The general process is to write some JS code, and then put any
 * calls to CanvasKit or related things in here. Running ./compile.sh and then
 * looking at the minified results or running the Release trybot should
 * verify nothing was missed. Optionally, looking directly at the minified
 * pathkit.js can be useful when developing locally.
 *
 * Docs:
 *   https://github.com/cljsjs/packages/wiki/Creating-Externs
 *   https://github.com/google/closure-compiler/wiki/Types-in-the-Closure-Type-System
 *
 * Example externs:
 *   https://github.com/google/closure-compiler/tree/master/externs
 */

var CanvasKit = {
  // public API (i.e. things we declare in the pre-js file or in the cpp bindings)
  Color: function() {},
  Color4f: function() {},
  ColorAsInt: function() {},
  LTRBRect: function() {},
  XYWHRect: function() {},
  LTRBiRect: function() {},
  XYWHiRect: function() {},
  RRectXY: function() {},
  /** @return {ImageData} */
  ImageData: function() {},

  GetWebGLContext: function() {},
  MakeCanvas: function() {},
  MakeCanvasSurface: function() {},
  MakeGrContext: function() {},
  /** @return {CanvasKit.AnimatedImage} */
  MakeAnimatedImageFromEncoded: function() {},
  /** @return {CanvasKit.Image} */
  MakeImage: function() {},
  /** @return {CanvasKit.Image} */
  MakeImageFromEncoded: function() {},
  MakeImageFromCanvasImageSource: function() {},
  MakeOnScreenGLSurface: function() {},
  MakeRenderTarget: function() {},
  MakePicture: function() {},
  MakeSWCanvasSurface: function() {},
  MakeManagedAnimation: function() {},
  MakeParticles: function() {},
  MakeVertices: function() {},
  MakeSurface: function() {},
  MakeRasterDirectSurface: function() {},
  MakeWebGLCanvasSurface: function() {},
  Malloc: function() {},
  MallocGlyphIDs: function() {},
  Free: function() {},
  computeTonalColors: function() {},
  currentContext: function() {},
  deleteContext: function() {},
  getColorComponents: function() {},
  getDecodeCacheLimitBytes: function() {},
  getDecodeCacheUsageBytes: function() {},
  multiplyByAlpha: function() {},
  parseColorString: function() {},
  setCurrentContext: function() {},
  setDecodeCacheLimitBytes: function() {},
  getShadowLocalBounds: function() {},
  // Defined by emscripten.
  createContext: function() {},

  // private API (i.e. things declared in the bindings that we use
  // in the pre-js file)
  _computeTonalColors: function() {},
  _MakeImage: function() {},
  _MakeManagedAnimation: function() {},
  _MakeParticles: function() {},
  _MakePicture: function() {},
  _decodeAnimatedImage: function() {},
  _decodeImage: function() {},
  _getShadowLocalBounds: function() {},

  // The testing object is meant to expose internal functions
  // for more fine-grained testing, e.g. parseColor
  _testing: {},

  // Objects and properties on CanvasKit

  Animation: {
    prototype: {
      render: function() {},
      size: function() {},
    },
    _render: function() {},
    _size: function() {},
  },

  GrContext: {
    // public API (from C++ bindings)
    getResourceCacheLimitBytes: function() {},
    getResourceCacheUsageBytes: function() {},
    releaseResourcesAndAbandonContext: function() {},
    setResourceCacheLimitBytes: function() {},
  },

  ManagedAnimation: {
    prototype: {
      render: function() {},
      seek: function() {},
      seekFrame: function() {},
      setColor: function() {},
      size: function() {},
    },
    _render: function() {},
    _seek: function() {},
    _seekFrame: function() {},
    _size: function() {},
  },

  Paragraph: {
    // public API (from C++ bindings)
    didExceedMaxLines: function() {},
    getAlphabeticBaseline: function() {},
    getGlyphPositionAtCoordinate: function() {},
    getHeight: function() {},
    getIdeographicBaseline: function() {},
    getLineMetrics: function() {},
    getLongestLine: function() {},
    getMaxIntrinsicWidth: function() {},
    getMaxWidth: function() {},
    getMinIntrinsicWidth: function() {},
    getWordBoundary: function() {},
    layout: function() {},

    // private API
    /** @return {Float32Array} */
    _getRectsForRange: function() {},
    _getRectsForPlaceholders: function() {},
  },

  ParagraphBuilder: {
    Make: function() {},
    MakeFromFontProvider: function() {},
    addText: function() {},
    build: function() {},
    pop: function() {},

    prototype: {
      pushStyle: function() {},
      pushPaintStyle: function() {},
      addPlaceholder: function() {},
    },

    // private API
    _Make: function() {},
    _MakeFromFontProvider: function() {},
    _pushStyle: function() {},
    _pushPaintStyle: function() {},
    _addPlaceholder: function() {},
  },

  RuntimeEffect: {
    // public API (from JS bindings)
    Make: function() {},
    getUniform: function() {},
    getUniformCount: function() {},
    getUniformFloatCount: function() {},
    getUniformName: function() {},
    prototype: {
      makeShader: function() {},
      makeShaderWithChildren: function() {},
    },
    // private API (from C++ bindings)
    _Make: function() {},
    _makeShader: function() {},
    _makeShaderWithChildren: function() {},
  },

  ParagraphStyle: function() {},
  RSXFormBuilder: function() {},
  ColorBuilder: function() {},
  RectBuilder: function() {},

  AnimatedImage: {
    // public API (from C++ bindings)
    decodeNextFrame: function() {},
    getFrameCount: function() {},
    getRepetitionCount: function() {},
    height: function() {},
    reset: function() {},
    width: function() {},
  },

  Canvas: {
    // public API (from C++ bindings)
    clipPath: function() {},
    drawCircle: function() {},
    drawColorInt: function() {},
    drawImage: function() {},
    drawImageCubic: function() {},
    drawImageOptions: function() {},
    drawImageAtCurrentFrame: function() {},
    drawLine: function() {},
    drawPaint: function() {},
    drawParagraph: function() {},
    drawPath: function() {},
    drawPicture: function() {},
    drawRect4f: function() {},
    drawText: function() {},
    drawTextBlob: function() {},
    drawVertices: function() {},
    flush: function() {},
    getSaveCount: function() {},
    makeSurface: function() {},
    markCTM: function() {},
    findMarkedCTM: function() {},
    restore: function() {},
    restoreToCount: function() {},
    rotate: function() {},
    save: function() {},
    saveLayerPaint: function() {},
    scale: function() {},
    skew: function() {},
    translate: function() {},

    prototype: {
      clear: function() {},
      clipRRect: function() {},
      clipRect: function() {},
      concat: function() {},
      drawArc: function() {},
      drawAtlas: function() {},
      drawColor: function() {},
      drawColorComponents: function() {},
      drawDRRect:  function() {},
      drawImageNine: function() {},
      drawImageRect: function() {},
      drawImageRectCubic: function() {},
      drawImageRectOptions: function() {},
      drawOval: function() {},
      drawPoints: function() {},
      drawRect: function() {},
      drawRRect:  function() {},
      drawShadow: function() {},
      drawText: function() {},
      findMarkedCTM: function() {},
      getLocalToDevice: function() {},
      getTotalMatrix: function() {},
      readPixels: function() {},
      saveLayer: function() {},
      writePixels : function() {},
    },

    // private API
    _clear: function() {},
    _clipRRect: function() {},
    _clipRect: function() {},
    _concat: function() {},
    _drawArc: function() {},
    _drawAtlas: function() {},
    _drawColor: function() {},
    _drawDRRect:  function() {},
    _drawImageNine: function() {},
    _drawImageRect: function() {},
    _drawImageRectCubic: function() {},
    _drawImageRectOptions: function() {},
    _drawOval: function() {},
    _drawPoints: function() {},
    _drawRect: function() {},
    _drawRRect:  function() {},
    _drawShadow: function() {},
    _drawSimpleText: function() {},
    _findMarkedCTM: function() {},
    _getLocalToDevice: function() {},
    _getTotalMatrix: function() {},
    _readPixels: function() {},
    _saveLayer: function() {},
    _writePixels: function() {},
    delete: function() {},
  },

  ColorFilter: {
    // public API (from C++ bindings and JS interface)
    MakeBlend: function() {},
    MakeCompose: function() {},
    MakeLerp: function() {},
    MakeLinearToSRGBGamma: function() {},
    MakeMatrix: function() {},
    MakeSRGBToLinearGamma: function() {},
    // private API (from C++ bindings)
    _MakeBlend: function() {},
    _makeMatrix: function() {},
  },

  ColorMatrix: {
    concat: function() {},
    identity: function() {},
    postTranslate: function() {},
    rotated: function() {},
    scaled: function() {},
  },

  ColorSpace: {
    Equals: function() {},
    SRGB: {},
    DISPLAY_P3: {},
    ADOBE_RGB: {},
    // private API (from C++ bindings)
    _MakeSRGB: function() {},
    _MakeDisplayP3: function() {},
    _MakeAdobeRGB: function() {},
  },

  ContourMeasureIter: {
    next: function() {},
  },

  ContourMeasure: {
    getSegment: function() {},
    isClosed: function() {},
    length: function() {},
    prototype: {
      getPosTan: function() {},
    },
    _getPosTan: function() {},
  },

  Font: {
    // public API (from C++ bindings)
    getScaleX: function() {},
    getSize: function() {},
    getSkewX: function() {},
    getTypeface: function() {},
    setHinting: function() {},
    setLinearMetrics: function() {},
    setScaleX: function() {},
    setSize: function() {},
    setSkewX: function() {},
    setSubpixel: function() {},
    setTypeface: function() {},

    prototype: {
      getGlyphBounds: function() {},
      getGlyphIDs: function() {},
      getGlyphWidths: function() {},
    },

    // private API (from C++ bindings)
    _getGlyphIDs: function() {},
    _getGlyphWidthBounds: function() {},
  },

  FontMgr: {
    // public API (from C++ and JS bindings)
    FromData: function() {},
    RefDefault: function() {},
    countFamilies: function() {},
    getFamilyName: function() {},

    // private API
    _makeTypefaceFromData: function() {},
    _fromData: function() {},
  },

  TypefaceFontProvider: {
    // public API (from C++ and JS bindings)
    Make: function() {},
    registerFont: function() {},

    // private API
    _registerFont: function() {},
  },

  Image: {
    // public API (from C++ bindings)
    encodeToBytes: function() {},
    getColorSpace: function() {},
    getImageInfo: function() {},
    makeCopyWithDefaultMipmaps: function() {},
    height: function() {},
    width: function() {},

    prototype: {
      makeShaderCubic: function() {},
      makeShaderOptions: function() {},
    },
    // private API
    _makeShaderCubic: function() {},
    _makeShaderOptions: function() {},
  },

  ImageFilter: {
    MakeBlur: function() {},
    MakeColorFilter: function() {},
    MakeCompose: function() {},
    MakeMatrixTransform: function() {},

    // private API
    _MakeMatrixTransform: function() {},
  },

  // These are defined in interface.js
  M44: {
    identity: function() {},
    invert: function() {},
    mustInvert: function() {},
    multiply: function() {},
    rotatedUnitSinCos: function() {},
    rotated: function() {},
    scaled: function() {},
    translated: function() {},
    lookat: function() {},
    perspective: function() {},
    rc: function() {},
    transpose: function() {},
    setupCamera: function() {},
  },

  Matrix: {
    identity: function() {},
    invert: function() {},
    mapPoints: function() {},
    multiply: function() {},
    rotated: function() {},
    scaled: function() {},
    skewed: function() {},
    translated: function() {},
  },

  MaskFilter: {
    MakeBlur: function() {},
  },

  Paint: {
    // public API (from C++ bindings)
    /** @return {CanvasKit.Paint} */
    copy: function() {},
    getBlendMode: function() {},
    getFilterQuality: function() {},
    getStrokeCap: function() {},
    getStrokeJoin: function() {},
    getStrokeMiter: function() {},
    getStrokeWidth: function() {},
    setAntiAlias: function() {},
    setBlendMode: function() {},
    setColorInt: function() {},
    setFilterQuality: function() {},
    setImageFilter: function() {},
    setMaskFilter: function() {},
    setPathEffect: function() {},
    setShader: function() {},
    setStrokeCap: function() {},
    setStrokeJoin: function() {},
    setStrokeMiter: function() {},
    setStrokeWidth: function() {},
    setStyle: function() {},

    prototype: {
      getColor: function() {},
      setColor: function() {},
      setColorComponents: function() {},
      setColorInt: function() {},
    },

    // Private API
    delete: function() {},
    _getColor: function() {},
    _setColor: function() {},
  },

  PathEffect: {
    MakeCorner: function() {},
    MakeDash: function() {},
    MakeDiscrete: function() {},

    // Private C++ API
    _MakeDash: function() {},
  },

  ParticleEffect: {
    // public API (from C++ bindings)
    draw: function() {},
    getUniform: function() {},
    getUniformCount: function() {},
    getUniformFloatCount: function() {},
    getUniformName: function() {},
    setRate: function() {},
    start: function() {},
    update: function() {},

    prototype: {
      setPosition: function() {},
      uniforms: function() {},
    },

    // private API (from C++ bindings)
    _uniformPtr: function() {},
    _setPosition: function() {},
  },

  Path: {
    // public API (from C++ and JS bindings)
    MakeFromCmds: function() {},
    MakeFromSVGString: function() {},
    MakeFromOp: function() {},
    MakeFromVerbsPointsWeights: function() {},
    contains: function() {},
    /** @return {CanvasKit.Path} */
    copy: function() {},
    countPoints: function() {},
    equals: function() {},
    getFillType: function() {},
    isEmpty: function() {},
    isVolatile: function() {},
    reset: function() {},
    rewind: function() {},
    setFillType: function() {},
    setIsVolatile: function() {},
    toCmds: function() {},
    toSVGString: function() {},

    prototype: {
      addArc: function() {},
      addOval: function() {},
      addPath: function() {},
      addPoly: function() {},
      addRect: function() {},
      addRRect: function() {},
      addVerbsPointsWeights: function() {},
      arc: function() {},
      arcToOval: function() {},
      arcToRotated: function() {},
      arcToTangent: function() {},
      close: function() {},
      conicTo: function() {},
      computeTightBounds: function() {},
      cubicTo: function() {},
      dash: function() {},
      getBounds: function() {},
      getPoint: function() {},
      lineTo: function() {},
      moveTo: function() {},
      offset: function() {},
      op: function() {},
      quadTo: function() {},
      rArcTo: function() {},
      rConicTo: function() {},
      rCubicTo: function() {},
      rLineTo: function() {},
      rMoveTo: function() {},
      rQuadTo: function() {},
      simplify: function() {},
      stroke: function() {},
      transform: function() {},
      trim: function() {},
    },

    // private API
    _MakeFromCmds: function() {},
    _MakeFromVerbsPointsWeights: function() {},
    _addArc: function() {},
    _addOval: function() {},
    _addPath: function() {},
    _addPoly: function() {},
    _addRect: function() {},
    _addRRect: function() {},
    _addVerbsPointsWeights: function() {},
    _arcToOval: function() {},
    _arcToRotated: function() {},
    _arcToTangent: function() {},
    _close: function() {},
    _conicTo: function() {},
    _computeTightBounds: function() {},
    _cubicTo: function() {},
    _dash: function() {},
    _getBounds: function() {},
    _getPoint: function() {},
    _lineTo: function() {},
    _moveTo: function() {},
    _op: function() {},
    _quadTo: function() {},
    _rArcTo: function() {},
    _rConicTo: function() {},
    _rCubicTo: function() {},
    _rect: function() {},
    _rLineTo: function() {},
    _rMoveTo: function() {},
    _rQuadTo: function() {},
    _simplify: function() {},
    _stroke: function() {},
    _transform: function() {},
    _trim: function() {},
    delete: function() {},
    dump: function() {},
    dumpHex: function() {},
  },

  Picture: {
    serialize: function() {},
  },

  PictureRecorder: {
    finishRecordingAsPicture: function() {},
    prototype: {
      beginRecording: function() {},
    },
    _beginRecording: function() {},
  },

  Shader: {
    // Deprecated names
    Blend: function() {},
    Color: function() {},
    Lerp: function() {},
    // public API (from JS / C++ bindings)
    MakeBlend: function() {},
    MakeColor: function() {},
    MakeFractalNoise: function() {},
    MakeLerp: function() {},
    MakeLinearGradient: function() {},
    MakeRadialGradient: function() {},
    MakeSweepGradient: function() {},
    MakeTurbulence: function() {},
    MakeTwoPointConicalGradient: function() {},

    // private API (from C++ bindings)
    _MakeColor: function() {},
    _MakeLinearGradient: function() {},
    _MakeRadialGradient: function() {},
    _MakeSweepGradient: function() {},
    _MakeTwoPointConicalGradient: function() {},
  },

  Surface: {
    // public API (from C++ bindings)
    /** @return {CanvasKit.Canvas} */
    getCanvas: function() {},
    imageInfo: function() {},

    makeSurface: function() {},
    sampleCnt: function() {},
    reportBackendTypeIsGPU: function() {},
    grContext: {},
    openGLversion: {},

    prototype: {
      /** @return {CanvasKit.Image} */
      makeImageSnapshot: function() {},
    },

    // private API
    _flush: function() {},
    _makeImageSnapshot: function() {},
    _makeRasterDirect: function() {},
    delete: function() {},
  },

  TextBlob: {
    // public API (both C++ and JS bindings)
    MakeFromGlyphs: function() {},
    MakeFromRSXform: function() {},
    MakeFromRSXformGlyphs: function() {},
    MakeFromText: function() {},
    MakeOnPath: function() {},
    // private API (from C++ bindings)
    _MakeFromGlyphs: function() {},
    _MakeFromRSXform: function() {},
    _MakeFromRSXformGlyphs: function() {},
    _MakeFromText: function() {},
  },

  // These are defined in interface.js
  Vector: {
    add: function() {},
    sub: function() {},
    dot: function() {},
    cross: function() {},
    normalize: function() {},
    mulScalar: function() {},
    length: function() {},
    lengthSquared: function() {},
    dist: function() {},
  },

  Vertices: {
    // public API (from C++ bindings)
    uniqueID: function() {},

    prototype: {
      bounds: function() {},
    },
    // private API (from C++ bindings)

    _bounds: function() {},
  },

  _VerticesBuilder: {
    colors: function() {},
    detach: function() {},
    indices: function() {},
    positions: function() {},
    texCoords: function() {},
  },

  TextStyle: function() {},

  // Constants and Enums
  gpu: {},
  skottie: {},

  TRANSPARENT: {},
  BLACK: {},
  WHITE: {},
  RED: {},
  GREEN: {},
  BLUE: {},
  YELLOW: {},
  CYAN: {},
  MAGENTA: {},

  MOVE_VERB: {},
  LINE_VERB: {},
  QUAD_VERB: {},
  CONIC_VERB: {},
  CUBIC_VERB: {},
  CLOSE_VERB: {},

  NoDecoration: {},
  UnderlineDecoration: {},
  OverlineDecoration: {},
  LineThroughDecoration: {},

  SaveLayerInitWithPrevious: {},
  SaveLayerF16ColorType: {},

  Affinity: {
    Upstream: {},
    Downstream: {},
  },

  AlphaType: {
    Opaque: {},
    Premul: {},
    Unpremul: {},
  },

  BlendMode: {
    Clear: {},
    Src: {},
    Dst: {},
    SrcOver: {},
    DstOver: {},
    SrcIn: {},
    DstIn: {},
    SrcOut: {},
    DstOut: {},
    SrcATop: {},
    DstATop: {},
    Xor: {},
    Plus: {},
    Modulate: {},
    Screen: {},
    Overlay: {},
    Darken: {},
    Lighten: {},
    ColorDodge: {},
    ColorBurn: {},
    HardLight: {},
    SoftLight: {},
    Difference: {},
    Exclusion: {},
    Multiply: {},
    Hue: {},
    Saturation: {},
    Color: {},
    Luminosity: {},
  },

  BlurStyle: {
    Normal: {},
    Solid: {},
    Outer: {},
    Inner: {},
  },

  ClipOp: {
    Difference: {},
    Intersect: {},
  },

  ColorType: {
    Alpha_8: {},
    RGB_565: {},
    ARGB_4444: {},
    RGBA_8888: {},
    RGB_888x: {},
    BGRA_8888: {},
    RGBA_1010102: {},
    RGB_101010x: {},
    Gray_8: {},
    RGBA_F16: {},
    RGBA_F32: {},
  },

  FillType: {
    Winding: {},
    EvenOdd: {},
  },

  FilterMode: {
    Linear: {},
    Nearest: {},
  },

  FilterQuality: {
    None: {},
    Low: {},
    Medium: {},
    High: {},
  },

  FontSlant: {
    Upright: {},
    Italic: {},
    Oblique: {},
  },

  FontHinting: {
    None: {},
    Slight: {},
    Normal: {},
    Full: {},
  },

  FontWeight: {
    Invisible: {},
    Thin: {},
    ExtraLight: {},
    Light: {},
    Normal: {},
    Medium: {},
    SemiBold: {},
    Bold: {},
    ExtraBold: {},
    Black: {},
    ExtraBlack: {},
  },

  FontWidth: {
    UltraCondensed: {},
    ExtraCondensed: {},
    Condensed: {},
    SemiCondensed: {},
    Normal: {},
    SemiExpanded: {},
    Expanded: {},
    ExtraExpanded: {},
    UltraExpanded: {},
  },

  ImageFormat: {
    PNG: {},
    JPEG: {},
  },

  PaintStyle: {
    Fill: {},
    Stroke: {},
  },

  PathOp: {
    Difference: {},
    Intersect: {},
    Union: {},
    XOR: {},
    ReverseDifference: {},
  },

  PointMode: {
    Points: {},
    Lines: {},
    Polygon: {},
  },

  RectHeightStyle: {
    Tight: {},
    Max: {},
    IncludeLineSpacingMiddle: {},
    IncludeLineSpacingTop: {},
    IncludeLineSpacingBottom: {},
  },

  RectWidthStyle: {
    Tight: {},
    Max: {},
  },

  StrokeCap: {
    Butt: {},
    Round: {},
    Square: {},
  },

  StrokeJoin: {
    Miter: {},
    Round: {},
    Bevel: {},
  },

  TextAlign: {
    Left: {},
    Right: {},
    Center: {},
    Justify: {},
    Start: {},
    End: {},
  },

  TextDirection: {
    LTR: {},
    RTL: {},
  },

  DecorationStyle: {
    Solid: {},
    Double: {},
    Dotted: {},
    Dashed: {},
    Wavy: {},
  },

  PlaceholderAlignment: {
    Baseline: {},
    AboveBaseline: {},
    BelowBaseline: {},
    Top: {},
    Bottom: {},
    Middle: {},
  },

  TextBaseline: {
    Alphabetic: {},
    Ideographic: {},
  },

  TileMode: {
    Clamp: {},
    Repeat: {},
    Mirror: {},
    Decal: {},
  },

  VertexMode: {
    Triangles: {},
    TrianglesStrip: {},
    TriangleFan: {},
  },

  // Things Enscriptem adds for us

  /**
   * @type {Float32Array}
   */
  HEAPF32: {},
  /**
   * @type {Float64Array}
   */
  HEAPF64: {},
  /**
   * @type {Uint8Array}
   */
  HEAPU8: {},
  /**
   * @type {Uint16Array}
   */
  HEAPU16: {},
  /**
   * @type {Uint32Array}
   */
  HEAPU32: {},
  /**
   * @type {Int8Array}
   */
  HEAP8: {},
  /**
   * @type {Int16Array}
   */
  HEAP16: {},
  /**
   * @type {Int32Array}
   */
  HEAP32: {},

  _malloc: function() {},
  _free: function() {},
  onRuntimeInitialized: function() {},
};

// Public API things that are newly declared in the JS should go here.
// It's not enough to declare them above, because closure can still erase them
// unless they go on the prototype.
CanvasKit.Paragraph.prototype.getRectsForRange = function() {};
CanvasKit.Paragraph.prototype.getRectsForPlaceholders = function() {};

CanvasKit.Picture.prototype.saveAsFile = function() {};

CanvasKit.Surface.prototype.dispose = function() {};
CanvasKit.Surface.prototype.flush = function() {};
CanvasKit.Surface.prototype.requestAnimationFrame = function() {};
CanvasKit.Surface.prototype.drawOnce = function() {};

CanvasKit.FontMgr.prototype.MakeTypefaceFromData = function() {};

CanvasKit.RSXFormBuilder.prototype.build = function() {};
CanvasKit.RSXFormBuilder.prototype.delete = function() {};
CanvasKit.RSXFormBuilder.prototype.push = function() {};
CanvasKit.RSXFormBuilder.prototype.set = function() {};

CanvasKit.ColorBuilder.prototype.build = function() {};
CanvasKit.ColorBuilder.prototype.delete = function() {};
CanvasKit.ColorBuilder.prototype.push = function() {};
CanvasKit.ColorBuilder.prototype.set = function() {};

CanvasKit.RuntimeEffect.prototype.makeShader = function() {};
CanvasKit.RuntimeEffect.prototype.makeShaderWithChildren = function() {};

// Define StrokeOpts object
var StrokeOpts = {};
StrokeOpts.prototype.width;
StrokeOpts.prototype.miter_limit;
StrokeOpts.prototype.cap;
StrokeOpts.prototype.join;
StrokeOpts.prototype.precision;

// Define everything created in the canvas2d spec here
var HTMLCanvas = {};
HTMLCanvas.prototype.decodeImage = function() {};
HTMLCanvas.prototype.dispose = function() {};
HTMLCanvas.prototype.getContext = function() {};
HTMLCanvas.prototype.loadFont = function() {};
HTMLCanvas.prototype.makePath2D = function() {};
HTMLCanvas.prototype.toDataURL = function() {};

var ImageBitmapRenderingContext = {};
ImageBitmapRenderingContext.prototype.transferFromImageBitmap = function() {};

var CanvasRenderingContext2D = {};
CanvasRenderingContext2D.prototype.addHitRegion = function() {};
CanvasRenderingContext2D.prototype.arc = function() {};
CanvasRenderingContext2D.prototype.arcTo = function() {};
CanvasRenderingContext2D.prototype.beginPath = function() {};
CanvasRenderingContext2D.prototype.bezierCurveTo = function() {};
CanvasRenderingContext2D.prototype.clearHitRegions = function() {};
CanvasRenderingContext2D.prototype.clearRect = function() {};
CanvasRenderingContext2D.prototype.clip = function() {};
CanvasRenderingContext2D.prototype.closePath = function() {};
CanvasRenderingContext2D.prototype.createImageData = function() {};
CanvasRenderingContext2D.prototype.createLinearGradient = function() {};
CanvasRenderingContext2D.prototype.createPattern = function() {};
CanvasRenderingContext2D.prototype.createRadialGradient = function() {};
CanvasRenderingContext2D.prototype.drawFocusIfNeeded = function() {};
CanvasRenderingContext2D.prototype.drawImage = function() {};
CanvasRenderingContext2D.prototype.ellipse = function() {};
CanvasRenderingContext2D.prototype.fill = function() {};
CanvasRenderingContext2D.prototype.fillRect = function() {};
CanvasRenderingContext2D.prototype.fillText = function() {};
CanvasRenderingContext2D.prototype.getImageData = function() {};
CanvasRenderingContext2D.prototype.getLineDash = function() {};
CanvasRenderingContext2D.prototype.isPointInPath = function() {};
CanvasRenderingContext2D.prototype.isPointInStroke = function() {};
CanvasRenderingContext2D.prototype.lineTo = function() {};
CanvasRenderingContext2D.prototype.measureText = function() {};
CanvasRenderingContext2D.prototype.moveTo = function() {};
CanvasRenderingContext2D.prototype.putImageData = function() {};
CanvasRenderingContext2D.prototype.quadraticCurveTo = function() {};
CanvasRenderingContext2D.prototype.rect = function() {};
CanvasRenderingContext2D.prototype.removeHitRegion = function() {};
CanvasRenderingContext2D.prototype.resetTransform = function() {};
CanvasRenderingContext2D.prototype.restore = function() {};
CanvasRenderingContext2D.prototype.rotate = function() {};
CanvasRenderingContext2D.prototype.save = function() {};
CanvasRenderingContext2D.prototype.scale = function() {};
CanvasRenderingContext2D.prototype.scrollPathIntoView = function() {};
CanvasRenderingContext2D.prototype.setLineDash = function() {};
CanvasRenderingContext2D.prototype.setTransform = function() {};
CanvasRenderingContext2D.prototype.stroke = function() {};
CanvasRenderingContext2D.prototype.strokeRect = function() {};
CanvasRenderingContext2D.prototype.strokeText = function() {};
CanvasRenderingContext2D.prototype.transform = function() {};
CanvasRenderingContext2D.prototype.translate = function() {};

var Path2D = {};
Path2D.prototype.addPath = function() {};
Path2D.prototype.arc = function() {};
Path2D.prototype.arcTo = function() {};
Path2D.prototype.bezierCurveTo = function() {};
Path2D.prototype.closePath = function() {};
Path2D.prototype.ellipse = function() {};
Path2D.prototype.lineTo = function() {};
Path2D.prototype.moveTo = function() {};
Path2D.prototype.quadraticCurveTo = function() {};
Path2D.prototype.rect = function() {};

var LinearCanvasGradient = {};
LinearCanvasGradient.prototype.addColorStop = function() {};
var RadialCanvasGradient = {};
RadialCanvasGradient.prototype.addColorStop = function() {};
var CanvasPattern = {};
CanvasPattern.prototype.setTransform = function() {};

var ImageData = {
  /**
   * @type {Uint8ClampedArray}
   */
  data: {},
  height: {},
  width: {},
};

var DOMMatrix = {
  a: {},
  b: {},
  c: {},
  d: {},
  e: {},
  f: {},
};

// Not sure why this is needed - might be a bug in emsdk that this isn't properly declared.
function loadWebAssemblyModule() {};

// This is a part of emscripten's webgl glue code. Preserving this attribute is necessary
// to override it in the puppeteer tests
var LibraryEGL = {
  contextAttributes: {
    majorVersion: {}
  }
}
