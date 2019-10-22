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
	// public API (i.e. things we declare in the pre-js file)
	Color: function() {},
	/** @return {CanvasKit.SkRect} */
	LTRBRect: function() {},
	/** @return {CanvasKit.SkRect} */
	XYWHRect: function() {},
	/** @return {CanvasKit.SkRRect} */
	RRectXY: function() {},
	/** @return {ImageData} */
	ImageData: function() {},

	GetWebGLContext: function() {},
	MakeBlurMaskFilter: function() {},
	MakeCanvas: function() {},
	MakeCanvasSurface: function() {},
	MakeGrContext: function() {},
	/** @return {CanvasKit.SkAnimatedImage} */
	MakeAnimatedImageFromEncoded: function() {},
	/** @return {CanvasKit.SkImage} */
	MakeImageFromEncoded: function() {},
	/** @return {LinearCanvasGradient} */
	MakeLinearGradientShader: function() {},
	MakeOnScreenGLSurface: function() {},
	MakePathFromCmds: function() {},
	MakePathFromOp: function() {},
	MakePathFromSVGString: function() {},
	MakeRadialGradientShader: function() {},
	MakeRenderTarget: function() {},
	MakeSWCanvasSurface: function() {},
	MakeManagedAnimation: function() {},
	MakeSkDashPathEffect: function() {},
	MakeSkVertices: function() {},
	MakeSurface: function() {},
	/** @return {RadialCanvasGradient} */
	MakeTwoPointConicalGradientShader: function() {},
	MakeWebGLCanvasSurface: function() {},
	/** @return {TypedArray} */
	Malloc: function() {},
	/** @return {TonalColors} */
	computeTonalColors: function() {},
	currentContext: function() {},
	getColorComponents: function() {},
	getSkDataBytes: function() {},
	multiplyByAlpha: function() {},
	setCurrentContext: function() {},

	// private API (i.e. things declared in the bindings that we use
	// in the pre-js file)
	_MakeImage: function() {},
	_MakeLinearGradientShader: function() {},
	_MakePathFromCmds: function() {},
	_MakeRadialGradientShader: function() {},
	_MakeManagedAnimation: function() {},
	_MakeSkDashPathEffect: function() {},
	_MakeSkVertices: function() {},
	_MakeTwoPointConicalGradientShader: function() {},
	_decodeAnimatedImage: function() {},
	_decodeImage: function() {},
	_drawShapedText: function() {},
	_getRasterDirectSurface: function() {},
	_getRasterN32PremulSurface: function() {},

	// The testing object is meant to expose internal functions
	// for more fine-grained testing, e.g. parseColor
	_testing: {},

	// Objects and properties on CanvasKit

	GrContext: {
		// public API (from C++ bindings)
		getResourceCacheLimitBytes: function() {},
		getResourceCacheUsageBytes: function() {},
		setResourceCacheLimitBytes: function() {},
	},

	Paragraph: {
		// public API (from C++ bindings)
		getGlyphPositionAtCoordinate: function() {},
		layout: function() {},

		// private API
		/** @return {Float32Array} */
		_getRectsForRange: function() {},
	},

	ParagraphStyle: function() {},

	RSXFormBuilder: function() {},
	SkColorBuilder: function() {},
	SkRectBuilder: function() {},

	ShapedText: {
		// public API (from C++ bindings)
		getBounds: function() {},
	},

	SkAnimatedImage: {
		// public API (from C++ bindings)
		getRepetitionCount: function() {},
		decodeNextFrame: function() {},
	},

	SkCanvas: {
		// public API (from C++ bindings)
		clear: function() {},
		clipPath: function() {},
		clipRect: function() {},
		concat: function() {},
		drawArc: function() {},
		drawCircle: function() {},
		drawDRRect:  function() {},
		drawAnimatedImage: function() {},
		drawImage: function() {},
		drawImageRect: function() {},
		drawLine: function() {},
		drawOval: function() {},
		drawPaint: function() {},
		drawParagraph: function() {},
		drawPath: function() {},
		drawPicture: function() {},
		drawRRect:  function() {},
		drawRect: function() {},
		drawRoundRect: function() {},
		drawShadow: function() {},
		drawText: function() {},
		drawTextBlob: function() {},
		drawVertices: function() {},
		flush: function() {},
		getSaveCount: function() {},
		getTotalMatrix: function() {},
		makeSurface: function() {},
		restore: function() {},
		restoreToCount: function() {},
		rotate: function() {},
		save: function() {},
		saveLayer: function() {},
		scale: function() {},
		skew: function() {},
		translate: function() {},

		// private API
		_drawAtlas: function() {},
		_drawSimpleText: function() {},
		_readPixels: function() {},
		_writePixels: function() {},
		delete: function() {},
	},

	SkColorFilter: {
		// public API (from C++ bindings and JS interface)
		MakeBlend: function() {},
		MakeCompose: function() {},
		MakeLerp: function() {},
		MakeLinearToSRGBGamma: function() {},
		MakeMatrix: function() {},
		MakeSRGBToLinearGamma: function() {},
		// private API (from C++ bindings)
		_makeMatrix: function() {},
	},

	SkColorMatrix: {
		concat: function() {},
		identity: function() {},
		postTranslate: function() {},
		rotated: function() {},
		scaled: function() {},
	},

	SkFont: {
		// public API (from C++ bindings)
		getScaleX: function() {},
		getSize: function() {},
		getSkewX: function() {},
		getTypeface: function() {},
		measureText: function() {},
		setScaleX: function() {},
		setSize: function() {},
		setSkewX: function() {},
		setTypeface: function() {},
		// private API (from C++ bindings)
		_getWidths: function() {},
	},

	SkFontMgr: {
		// public API (from C++ and JS bindings)
		FromData: function() {},
		RefDefault: function() {},
		countFamilies: function() {},

		// private API
		_makeTypefaceFromData: function() {},
		_fromData: function() {},
	},

	SkImage: {
		// public API (from C++ bindings)
		height: function() {},
		width: function() {},
		// private API
		_encodeToData: function() {},
		_encodeToDataWithFormat: function() {},
		_makeShader: function() {},
	},

	SkMatrix: {
		identity: function() {},
		invert: function() {},
		mapPoints: function() {},
		multiply: function() {},
		rotated: function() {},
		scaled: function() {},
		skewed: function() {},
		translated: function() {},
	},

	SkPaint: {
		// public API (from C++ bindings)
		/** @return {CanvasKit.SkPaint} */
		copy: function() {},
		getBlendMode: function() {},
		getColor: function() {},
		getFilterQuality: function() {},
		getStrokeCap: function() {},
		getStrokeJoin: function() {},
		getStrokeMiter: function() {},
		getStrokeWidth: function() {},
		setAntiAlias: function() {},
		setBlendMode: function() {},
		setColor: function() {},
		setFilterQuality: function() {},
		setMaskFilter: function() {},
		setPathEffect: function() {},
		setShader: function() {},
		setStrokeCap: function() {},
		setStrokeJoin: function() {},
		setStrokeMiter: function() {},
		setStrokeWidth: function() {},
		setStyle: function() {},

		//private API
		delete: function() {},
	},

	SkPath: {
		// public API (from C++ bindings)
		computeTightBounds: function() {},
		contains: function() {},
		/** @return {CanvasKit.SkPath} */
		copy: function() {},
		countPoints: function() {},
		equals: function() {},
		getBounds: function() {},
		getFillType: function() {},
		getPoint: function() {},
		isEmpty: function() {},
		isVolatile: function() {},
		reset: function() {},
		rewind: function() {},
		setFillType: function() {},
		setIsVolatile: function() {},
		toSVGString: function() {},

		// private API
		_addArc: function() {},
		_addOval: function() {},
		_addPath: function() {},
		_addRect: function() {},
		_addRoundRect: function() {},
		_arc: function() {},
		_arcTo: function() {},
		_close: function() {},
		_conicTo: function() {},
		_cubicTo: function() {},
		_dash: function() {},
		_lineTo: function() {},
		_moveTo: function() {},
		_op: function() {},
		_quadTo: function() {},
		_rect: function() {},
		_simplify: function() {},
		_stroke: function() {},
		_transform: function() {},
		_trim: function() {},
		delete: function() {},
		dump: function() {},
		dumpHex: function() {},
	},

	SkPathMeasure: {
		getLength: function() {},
		getPosTan: function() {},
		isClosed: function() {},
		nextContour: function() {},
	},

	SkPicture: {
		DEBUGONLY_serialize: function() {},
	},

	SkPictureRecorder: {
		beginRecording: function() {},
		finishRecordingAsPicture: function() {},
	},

	SkRect: {
		fLeft: {},
		fTop: {},
		fRight: {},
		fBottom: {},
	},

	SkRRect: {
		rect: {},
		rx1: {},
		ry1: {},
		rx2: {},
		ry2: {},
		rx3: {},
		ry3: {},
		rx4: {},
		ry4: {},
	},

	SkSurface: {
		// public API (from C++ bindings)
		/** @return {CanvasKit.SkCanvas} */
		getCanvas: function() {},
		/** @return {CanvasKit.SkImage} */
		makeImageSnapshot: function() {},
		makeSurface: function() {},
		grContext: {},

		// private API
		_flush: function() {},
		_getRasterN32PremulSurface: function() {},
		delete: function() {},
	},

	SkTextBlob: {
		// public API (both C++ and JS bindings)
		MakeFromRSXform: function() {},
		MakeFromText: function() {},
		MakeOnPath: function() {},
		// private API (from C++ bindings)
		_MakeFromRSXform: function() {},
		_MakeFromText: function() {},
	},

	SkVertices: {
		// public API (from C++ bindings)
		bounds: function() {},
		mode: function() {},
		uniqueID: function() {},
		vertexCount: function() {},

		// private API
		/** @return {CanvasKit.SkVertices} */
		_applyBones: function() {},
	},

	_SkVerticesBuilder: {
		// public API (from C++ bindings)
		boneIndices: function() {},
		boneWeights: function() {},
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
	RED: {},
	BLUE: {},
	YELLOW: {},
	CYAN: {},
	BLACK: {},
	WHITE: {},

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
		InverseWinding: {},
		InverseEvenOdd: {},
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
		StrokeAndFill: {},
	},

	PathOp: {
		Difference: {},
		Intersect: {},
		Union: {},
		XOR: {},
		ReverseDifference: {},
	},

	RectHeightStyle: {
		Tight: {},
		Max: {},
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

	TextEncoding: {
		UTF8: {},
		UTF16: {},
		UTF32: {},
		GlyphID: {},
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

CanvasKit.SkPath.prototype.addArc = function() {};
CanvasKit.SkPath.prototype.addOval = function() {};
CanvasKit.SkPath.prototype.addPath = function() {};
CanvasKit.SkPath.prototype.addRect = function() {};
CanvasKit.SkPath.prototype.addRoundRect = function() {};
CanvasKit.SkPath.prototype.arc = function() {};
CanvasKit.SkPath.prototype.arcTo = function() {};
CanvasKit.SkPath.prototype.close = function() {};
CanvasKit.SkPath.prototype.conicTo = function() {};
CanvasKit.SkPath.prototype.cubicTo = function() {};
CanvasKit.SkPath.prototype.dash = function() {};
CanvasKit.SkPath.prototype.lineTo = function() {};
CanvasKit.SkPath.prototype.moveTo = function() {};
CanvasKit.SkPath.prototype.offset = function() {};
CanvasKit.SkPath.prototype.op = function() {};
CanvasKit.SkPath.prototype.quadTo = function() {};
CanvasKit.SkPath.prototype.rect = function() {};
CanvasKit.SkPath.prototype.simplify = function() {};
CanvasKit.SkPath.prototype.stroke = function() {};
CanvasKit.SkPath.prototype.transform = function() {};
CanvasKit.SkPath.prototype.trim = function() {};

CanvasKit.SkPicture.prototype.DEBUGONLY_saveAsFile = function() {};

CanvasKit.SkSurface.prototype.dispose = function() {};
CanvasKit.SkSurface.prototype.flush = function() {};
CanvasKit.SkSurface.prototype.requestAnimationFrame = function() {};
CanvasKit.SkSurface.prototype.captureFrameAsSkPicture = function() {};

/** @return {CanvasKit.SkVertices} */
CanvasKit.SkVertices.prototype.applyBones = function() {};

CanvasKit.SkImage.prototype.encodeToData = function() {};
CanvasKit.SkImage.prototype.makeShader = function() {};

CanvasKit.SkCanvas.prototype.drawAtlas = function() {};
CanvasKit.SkCanvas.prototype.drawText = function() {};
/** @return {Uint8Array} */
CanvasKit.SkCanvas.prototype.readPixels = function() {};
CanvasKit.SkCanvas.prototype.writePixels = function() {};

CanvasKit.SkFontMgr.prototype.MakeTypefaceFromData = function() {};

CanvasKit.SkFont.prototype.getWidths = function() {};

CanvasKit.RSXFormBuilder.prototype.build = function() {};
CanvasKit.RSXFormBuilder.prototype.delete = function() {};
CanvasKit.RSXFormBuilder.prototype.push = function() {};
CanvasKit.RSXFormBuilder.prototype.set = function() {};

CanvasKit.SkColorBuilder.prototype.build = function() {};
CanvasKit.SkColorBuilder.prototype.delete = function() {};
CanvasKit.SkColorBuilder.prototype.push = function() {};
CanvasKit.SkColorBuilder.prototype.set = function() {};

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
