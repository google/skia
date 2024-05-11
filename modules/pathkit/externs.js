/*
 * This externs file prevents the Closure JS compiler from minifying away
 * names of objects created by Emscripten.
 * Basically, by defining empty objects and functions here, Closure will
 * know not to rename them.  This is needed because of our pre-js files,
 * that is, the JS we hand-write to bundle into PathKit.
 *
 * Emscripten does not support automatically generating an externs file, so we
 * do it by hand. The general process is to write some JS code, and then put any
 * calls to PathKit or related things in here. Running ./compile.sh and then
 * looking at the minified results or running the Release-PathKit trybot should
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

var PathKit = {
	SkBits2FloatUnsigned: function(num) {},
	_malloc: function(size) {},
	_free: function(ptr) {},
	onRuntimeInitialized: function() {},
	_FromCmds: function(ptr, size) {},
	loadCmdsTypedArray: function(arr) {},
	FromCmds: function(arr) {},
	_SkCubicMap: function(cp1, cp2) {},
	cubicYFromX: function(cpx1, cpy1, cpx2, cpy2, X) {},
	cubicPtFromT: function(cpx1, cpy1, cpx2, cpy2, T) {},

	/**
	 * @type {Float32Array}
	 */
	HEAPF32: {},

	SkPath: {
		_addPath: function(path, scaleX, skewX, transX, skewY, scaleY, transY, pers0, pers1, pers2) {},
		_arc: function(x, y, radius, startAngle, endAngle, ccw) {},
		_arcTo: function(x1, y1, x2, y2, radius) {},
		_asWinding: function() {},
		_dash: function(on, off, phase) {},
		_close: function() {},
		_conicTo: function(x1, y1, x2, y2, w) {},
		copy: function() {},
		_cubicTo: function(cp1x, cp1y, cp2x, cp2y, x, y) {},
		_ellipse: function(x, y, radiusX, radiusY, rotation, startAngle, endAngle, ccw) {},
		_isEmpty: function() {},
		_lineTo: function(x1, y1) {},
		_moveTo: function(x1, y1) {},
		_op: function(otherPath, op) {},
		_quadTo: function(cpx, cpy, x, y) {},
		_rect: function(x, y, w, h) {},
		_reverseAddPath: function(path) {},
		_simplify: function() {},
		_stroke: function(opts) {},
		_trim: function(startT, stopT, isComplement) {},
		_transform: function() {}, // takes 1 or 9 params
	},

	StrokeCap: {
		BUTT: {},
		ROUND: {},
		SQUARE: {},
	},
	StrokeJoin: {
		MITER: {},
		ROUND: {},
		BEVEL: {},
	}
};

// Define StrokeOpts object
var StrokeOpts = {};
StrokeOpts.prototype.width;
StrokeOpts.prototype.miter_limit;
StrokeOpts.prototype.cap;
StrokeOpts.prototype.join;

// Define CubicMap object
var CubicMap = {};
CubicMap.prototype.computeYFromX = function(x) {};
CubicMap.prototype.computePtFromT = function(t) {};


// For whatever reason, the closure compiler thinks it can rename some of our
// prototype methods.  Not entirely sure why.
// Listing them here prevents that.
PathKit.SkPath.prototype.addPath = function() {};
PathKit.SkPath.prototype.arc = function(x, y, radius, startAngle, endAngle, ccw) {};
PathKit.SkPath.prototype.arcTo = function(x1, y1, x2, y2, radius) {};
PathKit.SkPath.prototype.asWinding = function() {};
PathKit.SkPath.prototype.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {};
PathKit.SkPath.prototype.close = function() {};
PathKit.SkPath.prototype.closePath = function() {};
PathKit.SkPath.prototype.conicTo = function(x1, y1, x2, y2, w) {};
PathKit.SkPath.prototype.cubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {};
PathKit.SkPath.prototype.dash = function(on, off, phase) {};
PathKit.SkPath.prototype.ellipse = function(x, y, radiusX, radiusY, rotation, startAngle, endAngle, ccw) {};
PathKit.SkPath.prototype.isEmpty = function() {};
PathKit.SkPath.prototype.lineTo = function(x, y) {};
PathKit.SkPath.prototype.moveTo = function(x, y) {};
PathKit.SkPath.prototype.op = function(otherPath, op) {};
PathKit.SkPath.prototype.quadTo = function(x1, y1, x2, y2) {};
PathKit.SkPath.prototype.quadraticCurveTo = function(x1, y1, x2, y2) {};
PathKit.SkPath.prototype.rect = function(x, y, w, h) {};
PathKit.SkPath.prototype.reverseAddPath = function() {};
PathKit.SkPath.prototype.simplify = function() {};
PathKit.SkPath.prototype.stroke = function(opts) {};
PathKit.SkPath.prototype.transform = function() {};
PathKit.SkPath.prototype.trim = function(startT, stopT, isComplement) {};
