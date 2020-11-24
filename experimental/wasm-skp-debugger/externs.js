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
	MakeCanvasSurface: function() {},
  MinVersion: function() {},

	ColorType: {
		RGBA_8888: {},
	},

	AlphaType: {
		Unpremul: {},
	},

	TRANSPARENT: {},

	SkSurface: {
		// public API (from C++ bindings)
		/** @return {DebuggerView.SkCanvas} */
		getCanvas: function() {},
		clear: function() {},

		// private API
		_flush: function() {},
		delete: function() {},
	},

	SkpDebugPlayer: {
		SkpDebugPlayer: function() {},
		changeFrame: function() {},
		deleteCommand: function() {},
		draw: function() {},
		drawTo: function() {},
		findCommandByPixel: function() {},
		getBounds: function() {},
		getFrameCount: function() {},
		getImageResource: function() {},
		getImageCount: function() {},
		getImageInfo: function() {},
		getLayerSummariesJs: function() {},
		getSize: function() {},
		imageUseInfoForFrameJs: function() {},
		jsonCommandList: function() {},
		lastCommandInfo: function() {},
		loadSkp: function() {},
		setClipVizColor: function() {},
		setCommandVisibility: function() {},
		setGpuOpBounds: function() {},
		setInspectedLayer: function() {},
		setOriginVisible: function() {},
		setOverdrawVis: function() {},
		setAndroidClipViz: function() {},
	},

	/**
	 * @type {Uint8Array}
	 */
	HEAPU8: {},
};

// Public API things that are newly declared in the JS should go here.
// It's not enough to declare them above, because closure can still erase them
// unless they go on the prototype.

DebuggerView.SkSurface.prototype.flush = function() {};
DebuggerView.SkSurface.prototype.dispose = function() {};