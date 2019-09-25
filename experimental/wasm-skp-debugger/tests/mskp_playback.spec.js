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

    it('can switch to the second frame of an animated skp', function(done) {
        LoadDebugger.then(catchException(done, () => {
            const canvasElement = document.getElementById('debugger_view');

            fetch('/debugger/anim.mskp').then(function(response) {
                // Load test file
                if (!response.ok) {
                  throw new Error("HTTP error, status = " + response.status);
                }
                response.arrayBuffer().then(function(buffer) {
                    const fileContents = new Uint8Array(buffer);
                    console.log('fetched /debugger/anim.mskp');
                    const player = Debugger.SkpFilePlayer(fileContents);
                    const bounds = player.getBounds();
                    canvasElement.width = bounds.fRight - bounds.fLeft;
                    canvasElement.height = bounds.fBottom - bounds.fTop;
                    expect(canvasElement.width).toBe(1080);
                    expect(canvasElement.height).toBe(1920);
                    const surface = Debugger.MakeWebGLCanvasSurface(canvasElement);
                    const numFrames = player.getFrameCount();
                    expect(numFrames).toBe(10);

                    let cmd = JSON.parse(player.jsonCommandList(surface));

                    // Move to last command in first frame
                    player.drawTo(surface, cmd.commands.length);
                    surface.flush();

                    // Move to frame two
                    player.changeFrame(1);
                    cmd = JSON.parse(player.jsonCommandList(surface));
                    // move to command 100 in frame 2
                    player.drawTo(surface, 100);
                    surface.flush();

                    console.log('drew picture to canvas element');
                    surface.dispose();
                    done();
                });
              });
        }));
    });
});
