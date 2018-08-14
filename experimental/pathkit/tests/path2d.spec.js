

describe('PathKit\'s Path2D API', function() {
    // Note, don't try to print the PathKit object - it can cause Karma/Jasmine to lock up.
    var PathKit = null;
    const LoadPathKit = new Promise(function(resolve, reject) {
        if (PathKit) {
            resolve();
        } else {
            PathKitInit({
                locateFile: (file) => '/pathkit/'+file,
            }).then((_PathKit) => {
                PathKit = _PathKit;
                resolve();
            });
        }
    });

    it('can do everything in the Path2D API w/o crashing', function(done) {
        LoadPathKit.then(() => {
            // This is taken from example.html
            let path = PathKit.NewPath();

            path.moveTo(20, 5);
            path.lineTo(30, 20);
            path.lineTo(40, 10);
            path.lineTo(50, 20);
            path.lineTo(60, 0);
            path.lineTo(20, 5);

            path.moveTo(20, 80);
            path.bezierCurveTo(90, 10, 160, 150, 190, 10);

            path.moveTo(36, 148);
            path.quadraticCurveTo(66, 188, 120, 136);
            path.lineTo(36, 148);

            path.rect(5, 170, 20, 20);

            path.moveTo(150, 180);
            path.arcTo(150, 100, 50, 200, 20);
            path.lineTo(160, 160);

            path.moveTo(20, 120);
            path.arc(20, 120, 18, 0, 1.75 * Math.PI);
            path.lineTo(20, 120);

            let secondPath = PathKit.NewPath();
            secondPath.ellipse(130, 25, 30, 10, -1*Math.PI/8, Math.PI/6, 1.5*Math.PI, false);

            path.addPath(secondPath);

            let m = document.createElementNS('http://www.w3.org/2000/svg', 'svg').createSVGMatrix();
            m.a = 1; m.b = 0;
            m.c = 0; m.d = 1;
            m.e = 0; m.f = 20.5;

            path.addPath(secondPath, m);
            // An alternate API that avoids having to make the SVG matrix
            path.addPath(secondPath, 1, 0, 0, 1, 0, 20.5);

            let canvas = document.createElement('canvas');
            let canvasCtx = canvas.getContext('2d');
            // Set canvas size and make it a bit bigger to zoom in on the lines
            standardizedCanvasSize(canvasCtx);
            canvasCtx.scale(3.0, 3.0);
            canvasCtx.fillStyle = 'blue';
            canvasCtx.stroke(path.toPath2D());

            path.delete();
            secondPath.delete();

            reportCanvas(canvas, 'path2D_api_example').then(() => {
                done();
            }).catch(reportError(done));
        });
    });

    it('approximates arcs (conics) with quads', function(done) {
        LoadPathKit.then(() => {
            let path = PathKit.NewPath();
            path.moveTo(50, 120);
            path.arc(50, 120, 45, 0, 1.75 * Math.PI);
            path.lineTo(50, 120);

            let canvas = document.createElement('canvas');
            let canvasCtx = canvas.getContext('2d');
            standardizedCanvasSize(canvasCtx);
            // The and.callThrough is important to make it actually
            // draw the quadratics
            spyOn(canvasCtx, 'quadraticCurveTo').and.callThrough();

            canvasCtx.beginPath();
            path.toCanvas(canvasCtx);
            canvasCtx.stroke();
            // No need to check the whole path, as that's more what the
            // gold correctness tests are for (can account for changes we make
            // to the approximation algorithms).
            expect(canvasCtx.quadraticCurveTo).toHaveBeenCalled();
            path.delete();
            reportCanvas(canvas, 'conics_quads_approx').then(() => {
                done();
            }).catch(reportError(done));
        });
    });

});
