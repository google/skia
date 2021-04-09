// This externs format needs to be a little different due to the fact we don't rename
// Module to CanvasKit in quite the same way.
Module.sayHello = function() {};
Module.publicFunction = function() {};
Module.publicExtension = function() {};
Module.withObject = function() {};
Module._privateFunction = function() {};
Module._privateExtension = function() {};
Module._withObject = function() {};

Module.Something =  {
  getName: function() {},
  prototype: {
    setName: function() {},
  },
  _setName: function() {},
};

Module.CompoundObj = {
  alpha: 0,
  beta: "",
  gamma: 0,
};

// Things provided by Emscripten that we don't want minified.
Module.onRuntimeInitialized = function() {};
Module._malloc = function() {};
Module._free = function() {};