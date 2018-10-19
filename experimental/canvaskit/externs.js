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
	MakeCanvas: function() {},
	MakeCanvasSurface: function() {},
	MakeSkDashPathEffect: function() {},
	MakeSurface: function() {},
	currentContext: function() {},
	initFonts: function() {},
	setCurrentContext: function() {},
	getSkDataBytes: function() {},

	// private API (i.e. things declared in the bindings that we use
	// in the pre-js file)
	_getWebGLSurface: function() {},
	_getRasterN32PremulSurface: function() {},
	_MakeSkDashPathEffect: function() {},

	// Objects and properties on CanvasKit

	SkCanvas: {
		// public API (from C++ bindings)
		clear: function() {},
		drawPaint: function() {},
		drawPath: function() {},
		drawText: function() {},
		flush: function() {},
		rotate: function() {},
		save: function() {},
		scale: function() {},
		setMatrix: function() {},
		skew: function() {},
		translate: function() {},

		// private API
		delete: function() {},
	},

	SkImage: {
		encodeToData: function() {},
	},

	SkPath: {
		// public API (from C++ bindings)

		// private API
		_addPath: function() {},
		_arcTo: function() {},
		_close: function() {},
		_conicTo: function() {},
		_cubicTo: function() {},
		_lineTo: function() {},
		_moveTo: function() {},
		_op: function() {},
		_quadTo: function() {},
		_rect: function() {},
		_simplify: function() {},
		_transform: function() {},
		delete: function() {},
	},

	SkPaint: {
		// public API (from C++ bindings)
		/** @return {CanvasKit.SkPaint} */
		copy: function() {},
		measureText: function() {},
		setAntiAlias: function() {},
		setColor: function() {},
		setPathEffect: function() {},
		setShader: function() {},
		setStrokeWidth: function() {},
		setStyle: function() {},
		setTextSize: function() {},

		//private API
		delete: function() {},
	},

	SkRect: {
		fLeft: {},
		fTop: {},
		fRight: {},
		fBottom: {},
	},

	SkSurface: {
		// public API (from C++ bindings)
		/** @return {CanvasKit.SkCanvas} */
		getCanvas: function() {},
		/** @return {CanvasKit.SkImage} */
		makeImageSnapshot: function() {},

		// private API
		_flush: function() {},
		_getRasterN32PremulSurface: function() {},
		_readPixels: function() {},
		delete: function() {},
	},

	// Constants and Enums
	gpu: {},
	skottie: {},
	PaintStyle: {
		FILL: {},
		STROKE: {},
		STROKE_AND_FILL: {},
	},

	FillType: {
		WINDING: {},
		EVENODD: {},
		INVERSE_WINDING: {},
		INVERSE_EVENODD: {},
	},

	// Things Enscriptem adds for us

	/** Represents the heap of the WASM code
	 * @type {ArrayBuffer}
	 */
	buffer: {},
	/**
	 * @type {Float32Array}
	 */
	HEAPF32: {}, // only needed for TypedArray mallocs
	/**
	 * @type {Uint8Array}
	 */
	HEAPU8: {},

	_malloc: function() {},
	_free: function() {},
	onRuntimeInitialized: function() {},
};

// Public API things that are newly declared in the JS should go here.
// It's not enough to declare them above, because closure can still erase them
// unless they go on the prototype.
CanvasKit.SkPath.prototype.addPath = function() {};
CanvasKit.SkPath.prototype.arcTo = function() {};
CanvasKit.SkPath.prototype.close = function() {};
CanvasKit.SkPath.prototype.conicTo = function() {};
CanvasKit.SkPath.prototype.cubicTo = function() {};
CanvasKit.SkPath.prototype.lineTo = function() {};
CanvasKit.SkPath.prototype.moveTo = function() {};
CanvasKit.SkPath.prototype.op = function() {};
CanvasKit.SkPath.prototype.quadTo = function() {};
CanvasKit.SkPath.prototype.rect = function() {};
CanvasKit.SkPath.prototype.simplify = function() {};
CanvasKit.SkPath.prototype.transform = function() {};

CanvasKit.SkSurface.prototype.flush = function() {};
CanvasKit.SkSurface.prototype.dispose = function() {};

// Not sure why this is needed - might be a bug in emsdk that this isn't properly declared.
function loadWebAssemblyModule() {}
