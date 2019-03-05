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
                locateFile: (file) => '/debugger/'+file,
            }).ready().then((_Debugger) => {
                Debugger = _Debugger;
                resolve();
            });
        }
    });

    let container = document.createElement('div');
    document.body.appendChild(container);
    container.innerHTML = `<canvas id=debugger_view width=720 height=1280></canvas>`;

    it('can load the built in skp sample', function(done) {
        LoadDebugger.then(catchException(done, () => {
            const surface = Debugger.MakeSWCanvasSurface('debugger_view');
            const player = new Debugger.SkpDebugPlayer();
            player.loadSkp();
            player.drawTo(surface, 789); // number of commands in sample file
            surface.flush();
            done();
        }));
    });
});
