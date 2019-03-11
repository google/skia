describe('PathKit\'s Path2D API', function() {
    it('can do everything in the Path2D API w/o crashing', function(done) {
        LoadPathKit.then(catchException(done, () => {
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

            let canvas = document.createElement('canvas');
            let canvasCtx = canvas.getContext('2d');
            // Set canvas size and make it a bit bigger to zoom in on the lines
            standardizedCanvasSize(canvasCtx);
            canvasCtx.scale(3.0, 3.0);
            canvasCtx.fillStyle = 'blue';
            canvasCtx.stroke(path.toPath2D());

            let svgPath = document.createElementNS('http://www.w3.org/2000/svg', 'path');
            svgPath.setAttribute('stroke', 'black');
            svgPath.setAttribute('fill', 'rgba(255,255,255,0.0)');
            svgPath.setAttribute('transform', 'scale(3.0, 3.0)');
            svgPath.setAttribute('d', path.toSVGString());

            let newSVG = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
            newSVG.appendChild(svgPath);
            newSVG.setAttribute('xmlns', 'http://www.w3.org/2000/svg');
            newSVG.setAttribute('width', 600);
            newSVG.setAttribute('height', 600);

            path.delete();
            secondPath.delete();

            reportCanvas(canvas, 'path2D_api_example').then(() => {
                reportSVG(newSVG, 'path2D_api_example').then(() => {
                    done();
                }).catch(reportError(done));
            }).catch(reportError(done));
        }));
    });

    it('can chain by returning the same object', function(done) {
        LoadPathKit.then(catchException(done, () => {
            let path = PathKit.NewPath();

            let p1 = path.moveTo(20, 5)
                .lineTo(30, 20)
                .quadTo(66, 188, 120, 136)
                .close();

            // these should be the same object
            expect(path === p1).toBe(true);
            p1.delete();
            try {
                // This should throw an exception because
                // the underlying path was already deleted.
                path.delete();
                expect('should not have gotten here').toBe(false);
            } catch (e) {
                // all is well
            }
            done();
        }));
    });

    it('does not leak path objects when chaining', function(done) {
        LoadPathKit.then(catchException(done, () => {
            // By default, we have 16 MB of memory assigned to our PathKit
            // library. This can be configured by -S TOTAL_MEMORY=NN
            // and defaults to 16MB (we likely don't need to touch this).
            // If there's a leak in here, we should OOM pretty quick.
            // Testing showed around 50k is enough to see one if we leak a path,
            // so run 250k times just to be safe.
            for(let i = 0; i < 250000; i++) {
                let path = PathKit.NewPath()
                                  .moveTo(20, 5)
                                  .lineTo(30, 20)
                                  .quadTo(66, 188, 120, 136)
                                  .close();
                path.delete();
            }
            done();
        }));
    });

    function drawTriangle() {
        let path = PathKit.NewPath();
        path.moveTo(0, 0);
        path.lineTo(10, 0);
        path.lineTo(10, 10);
        path.close();
        return path;
    }

    it('has multiple overloads of addPath', function(done) {
        LoadPathKit.then(catchException(done, () => {
            let basePath = PathKit.NewPath();
            let otherPath = drawTriangle();
            // These add path call can be chained.
            // add it unchanged
            basePath.addPath(otherPath)
            // providing the 6 params of an SVG matrix to make it appear 20.5 px down
                    .addPath(otherPath, 1, 0, 0, 1, 0, 20.5)
            // provide the full 9 matrix params to make it appear 30 px to the right
            // and be 3 times as big.
                    .addPath(otherPath, 3, 0, 30,
                                        0, 3, 0,
                                        0, 0, 1);

            reportPath(basePath, 'add_path_3x', done);
            basePath.delete();
            otherPath.delete();
        }));
    });

    it('approximates arcs (conics) with quads', function(done) {
        LoadPathKit.then(catchException(done, () => {
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
        }));
    });

});
