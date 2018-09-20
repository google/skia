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
	Color: function(r, g, b, a) {},
	currentContext: function() {},
	getWebGLSurface: function(htmlID) {},
	MakeSkDashPathEffect: function(intervals, phase) {},
	setCurrentContext: function() {},
	LTRBRect: function(l, t, r, b) {},

	// private API (i.e. things declared in the bindings that we use
	// in the pre-js file)
	_getWebGLSurface: function(htmlID, w, h) {},
	_malloc: function(size) {},
	onRuntimeInitialized: function() {},
	_MakeSkDashPathEffect: function(ptr, len, phase) {},

	// Objects and properties on CanvasKit

	HEAPF32: {}, // only needed for TypedArray mallocs

	SkPath: {
		// public API should go below because closure still will
		// remove things declared here and not on the prototype.

		// private API
		_addPath: function(path, scaleX, skewX, transX, skewY, scaleY, transY, pers0, pers1, pers2) {},
		_arcTo: function(x1, y1, x2, y2, radius) {},
		_close: function() {},
		_conicTo: function(x1, y1, x2, y2, w) {},
		_cubicTo: function(cp1x, cp1y, cp2x, cp2y, x, y) {},
		_lineTo: function(x1, y1) {},
		_moveTo: function(x1, y1) {},
		_op: function(otherPath, op) {},
		_quadTo: function(cpx, cpy, x, y) {},
		_rect: function(x, y, w, h) {},
		_simplify: function() {},
		_transform: function(scaleX, skewX, transX, skewY, scaleY, transY, pers0, pers1, pers2) {},
	}
}

// Path public API
CanvasKit.SkPath.prototype.addPath = function() {};
CanvasKit.SkPath.prototype.arcTo = function(x1, y1, x2, y2, radius) {};
CanvasKit.SkPath.prototype.close = function() {};
CanvasKit.SkPath.prototype.conicTo = function(x1, y1, x2, y2, w) {};
CanvasKit.SkPath.prototype.cubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {};
CanvasKit.SkPath.prototype.lineTo = function(x, y) {};
CanvasKit.SkPath.prototype.moveTo = function(x, y) {};
CanvasKit.SkPath.prototype.op = function(otherPath, op) {};
CanvasKit.SkPath.prototype.quadTo = function(x1, y1, x2, y2) {};
CanvasKit.SkPath.prototype.rect = function(x, y, w, h) {};
CanvasKit.SkPath.prototype.simplify = function() {};
CanvasKit.SkPath.prototype.transform = function() {};

// Not sure why this is needed - might be a bug in emsdk that this isn't properly declared.
function loadWebAssemblyModule() {}
