// The increased timeout is especially needed with larger binaries
// like in the debug/gpu build
jasmine.DEFAULT_TIMEOUT_INTERVAL = 20000;

describe('Debugger\'s Startup Behavior', function() {
    let container = document.createElement('div');
    document.body.appendChild(container);

    beforeEach(function() {
        container.innerHTML = `<canvas id=debugger_view width=720 height=1280></canvas>`;
    });

    afterEach(function() {
        container.innerHTML = '';
    });

    it('can load and draw a skp file on an Canvas2D', function(done) {
        LoadDebugger.then(catchException(done, () => {
            const surface = Debugger.MakeSWCanvasSurface(document.getElementById('debugger_view'));

            fetch('/debugger/sample.skp').then(function(response) {
                // Load test file
                if (!response.ok) {
                  throw new Error("HTTP error, status = " + response.status);
                }
                response.arrayBuffer().then(function(buffer) {
                    const fileContents = new Uint8Array(buffer);
                    console.log('fetched /debugger/sample.skp');
                    const player = Debugger.SkpFilePlayer(fileContents);
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

    it('can load and draw a skp file on a Web GL canvas', function(done) {
        LoadDebugger.then(catchException(done, () => {
            const surface = Debugger.MakeWebGLCanvasSurface(
                document.getElementById('debugger_view'));

            fetch('/debugger/sample.skp').then(function(response) {
                // Load test file
                if (!response.ok) {
                  throw new Error("HTTP error, status = " + response.status);
                }
                response.arrayBuffer().then(function(buffer) {
                    const fileContents = new Uint8Array(buffer);
                    console.log('fetched /debugger/sample.skp');
                    const player = Debugger.SkpFilePlayer(fileContents);
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
