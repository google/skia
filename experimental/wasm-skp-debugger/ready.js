// See https://github.com/kripken/emscripten/issues/5820#issuecomment-385722568
// for context on why the .then() that comes with Module breaks things (e.g. infinite loops)
// and why the below fixes it.
Module['ready'] = function() {
  return new Promise(function (resolve, reject) {
    delete Module['then'];
    Module['onAbort'] = reject;
    if (runtimeInitialized) {
      resolve(Module);
    } else {
      addOnPostRun(function() {
        resolve(Module);
      });
    }
  });
}
// TODO(kjlubick): Shut .then() entirely off in 0.4.0 by uncommenting below.
// delete Module['then'];