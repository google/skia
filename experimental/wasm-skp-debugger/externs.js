// TODO(nifong): Complete this before turning on clojure optimizations in compile.sh

var DebuggerView = {
	MakeSWCanvasSurface: function() {},
	_getRasterDirectSurface: function() {},
	_malloc: function() {},
	_free: function() {},
	onRuntimeInitialized: function() {},
	SkpFilePlayer: function() {},
	MakeWebGLCanvasSurface: function() {},
	MakeGrContext: function() {},
	MakeOnScreenGLSurface: function() {},

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

	SkpDebugPlayer: {
		SkpDebugPlayer: function() {},
		loadSkp: function() {},
		drawTo: function() {},
		getBounds: function() {},
		setOverdrawVis: function() {},
		setGpuOpBounds: function() {},
		setClipVizColor: function() {},
		getSize: function() {},
		deleteCommand: function() {},
		setCommandVisibility: function() {},
		jsonCommandList: function() {},
		lastCommandInfo: function() {},
	},
};

// Public API things that are newly declared in the JS should go here.
// It's not enough to declare them above, because closure can still erase them
// unless they go on the prototype.

DebuggerView.SkSurface.prototype.flush = function() {};
DebuggerView.SkSurface.prototype.dispose = function() {};