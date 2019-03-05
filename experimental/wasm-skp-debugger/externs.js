// TODO(nifong): Complete this before turning on clojure optimizations in compile.sh

var DebuggerView = {
	MakeSWCanvasSurface: function() {},

	_getRasterDirectSurface: function() {},
	_malloc: function() {},
	_free: function() {},
	onRuntimeInitialized: function() {},

	ColorType: {
		RGBA_8888: {},
	},

	AlphaType: {
		Unpremul: {},
	},

	SkSurface: {
		// public API (from C++ bindings)
		/** @return {DebuggerView.SkCanvas} */
		getCanvas: function() {},

		// private API
		_flush: function() {},
		delete: function() {},
	},
};

DebuggerView.SkSurface.prototype.flush = function() {};
DebuggerView.SkSurface.prototype.dispose = function() {};