




var PathKit = {
	SkBits2FloatUnsigned: function(num) {},
	_malloc: function(size) {},
	onRuntimeInitialized: function() {},

	HEAPF32: {},

	SkPath: {
		_addPath: function(path, scaleX, skewX, transX, skewY, scaleY, transY, pers0, pers1, pers2) {},
		_arc: function(x, y, radius, startAngle, endAngle, ccw) {},
		_arcTo: function(x1, y1, x2, y2, radius) {},
		_dash: function(on, off, phase) {},
		_close: function() {},
		_conicTo: function(x1, y1, x2, y2, w) {},
		copy: function() {},
		_cubicTo: function(cp1x, cp1y, cp2x, cp2y, x, y) {},
		_ellipse: function(x, y, radiusX, radiusY, rotation, startAngle, endAngle, ccw) {},
		_lineTo: function(x1, y1) {},
		_moveTo: function(x1, y1) {},
		_quadTo: function(x1, y1, x2, y2) {},
		_rect: function(x, y, w, h) {},
		_trim: function(startT, stopT, isComplement) {},
		_transform: function() {}, // takes 1 or 9 params
	},
};

// For whatever reason, the closure compiler thinks it can rename some of our
// prototype methods.  Not entirely sure why.
// Listing them here prevents that.
PathKit.SkPath.prototype.addPath = function() {};
PathKit.SkPath.prototype.arc = function(x, y, radius, startAngle, endAngle, ccw) {};
PathKit.SkPath.prototype.arcTo = function(x1, y1, x2, y2, radius) {};
PathKit.SkPath.prototype.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {};
PathKit.SkPath.prototype.close = function() {};
PathKit.SkPath.prototype.closePath = function() {};
PathKit.SkPath.prototype.conicTo = function(x1, y1, x2, y2, w) {};
PathKit.SkPath.prototype.cubicTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {};
PathKit.SkPath.prototype.dash = function(on, off, phase) {};
PathKit.SkPath.prototype.ellipse = function(x, y, radiusX, radiusY, rotation, startAngle, endAngle, ccw) {};
PathKit.SkPath.prototype.lineTo = function(x, y) {};
PathKit.SkPath.prototype.moveTo = function(x, y) {};
PathKit.SkPath.prototype.quadTo = function(x1, y1, x2, y2) {};
PathKit.SkPath.prototype.quadraticCurveTo = function(x1, y1, x2, y2) {};
PathKit.SkPath.prototype.rect = function(x, y, w, h) {};
PathKit.SkPath.prototype.transform = function() {};
PathKit.SkPath.prototype.trim = function(startT, stopT, isComplement) {};
// The following was taken from https://github.com/google/closure-compiler/blob/master/contrib/externs/svg.js

/**
 * @constructor
 */
function SVGMatrix(){}


/**
 * @type {number}
 */
SVGMatrix.prototype.a;


/**
 * @type {number}
 */
SVGMatrix.prototype.b;


/**
 * @type {number}
 */
SVGMatrix.prototype.c;


/**
 * @type {number}
 */
SVGMatrix.prototype.d;


/**
 * @type {number}
 */
SVGMatrix.prototype.e;


/**
 * @type {number}
 */
SVGMatrix.prototype.f;