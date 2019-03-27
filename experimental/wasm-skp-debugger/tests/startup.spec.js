// The increased timeout is especially needed with larger binaries
// like in the debug/gpu build
jasmine.DEFAULT_TIMEOUT_INTERVAL = 20000;

describe('Debugger\'s Startup Behavior', function() {
    // Note, don't try to print the CanvasKit object - it can cause Karma/Jasmine to lock up.
    var Debugger = null;
    const LoadDebugger = new Promise(function(resolve, reject) {
        if (Debugger) {
            resolve();
        } else {
            DebuggerInit({
                locateFile: (file) => '/debugger/bin/'+file,
            }).ready().then((_Debugger) => {
                Debugger = _Debugger;
                resolve();
            });
        }
    });

    let container = document.createElement('div');
    document.body.appendChild(container);
    container.innerHTML = `<canvas id=debugger_view width=720 height=1280></canvas>`;

    it('can load and draw a skp file', function(done) {
        LoadDebugger.then(catchException(done, () => {
            const surface = Debugger.MakeSWCanvasSurface(document.getElementById('debugger_view'));
            const player = new Debugger.SkpDebugPlayer();

            fetch('/debugger/sample.skp').then(function(response) {
                // Load test file
                if (!response.ok) {
                  throw new Error("HTTP error, status = " + response.status);
                }
                response.arrayBuffer().then(function(buffer) {
                    let fileContents = new Uint8Array(buffer);
                    console.log('fetched /debugger/sample.skp');
                    const size = fileContents.byteLength;
                    expect(size).toEqual(662976);

                    // Allocate memory in wasm to hold the skp file selected by the user.
                    const fileMemPtr = Debugger._malloc(size);
                    // Make a typed array view of that memory
                    let fileMem = new Uint8Array(Debugger.buffer, fileMemPtr, size);
                    // Copy the file into it
                    fileMem.set(fileContents);
                    // Hand off pointer to wasm
                    player.loadSkp(fileMemPtr, size);
                    // Draw picture
                    player.drawTo(surface, 789); // number of commands in sample file
                    surface.flush();

                    console.log('drew picture to canvas element');
                    surface.dispose();
                    done();
                });
              });
        }));
    });
});
