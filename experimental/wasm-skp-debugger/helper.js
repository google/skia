// Adds compile-time JS functions to augment the DebuggerView interface.
(function(DebuggerView){

    DebuggerView.SkpFilePlayer = function(file_arraybuf) {
    // Create the instance of SkpDebugPlayer
    var player = new this.SkpDebugPlayer();
    // Convert file (an ArrayBuffer) into a typedarray,
    // otherwise fileMem.set() below seems to have no effect.
    var fileContents = new Uint8Array(file_arraybuf);
    var size = fileContents.byteLength;
    // Allocate memory in wasm to hold the skp file selected by the user.
    var fileMemPtr = this._malloc(size);
    // Make a typed array view of that memory
    var fileMem = new Uint8Array(this.buffer, fileMemPtr, size);
    // Copy the file into it
    fileMem.set(fileContents);
    // Hand off pointer to wasm
    player.loadSkp(fileMemPtr, size);
    return player;
  }

}(Module)); // When this file is loaded in, the high level object is "Module";
