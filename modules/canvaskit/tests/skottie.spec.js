describe('Skottie behavior', function() {
    let container = document.createElement('div');
    document.body.appendChild(container);
    const CANVAS_WIDTH = 600;
    const CANVAS_HEIGHT = 600;

    beforeEach(function() {
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>
            <canvas width=600 height=600 id=report></canvas>`;
    });

    afterEach(function() {
        container.innerHTML = '';
    });

    it('can draw one with an animated gif', function(done) {
        const imgPromise = fetch('/assets/flightAnim.gif')
            .then((response) => response.arrayBuffer());
        const jsonPromise = fetch('/assets/animated_gif.json')
            .then((response) => response.text());

        Promise.all([imgPromise, jsonPromise, LoadCanvasKit]).then((values) => {
            if (!CanvasKit.managed_skottie) {
                console.warn('Skipping test because not compiled with skottie')
                done();
                return;
            }
            catchException(done, () => {
                const imgBuffer = values[0];
                expect(imgBuffer).toBeTruthy();
                expect(imgBuffer.byteLength).toBeTruthy();
                const jsonStr = values[1];
                expect(jsonStr).toBeTruthy();

                const c = document.getElementById('test');
                expect(c).toBeTruthy();
                let surface = CanvasKit.MakeCanvasSurface(c);
                expect(surface).toBeTruthy('Could not make surface');
                if (CanvasKit.gpu) {
                    // If we are on GPU, make sure we didn't fallback
                    expect(c).not.toHaveClass('ck-replaced');
                }
                const animation = CanvasKit.MakeManagedAnimation(jsonStr, {
                    'flightAnim.gif': imgBuffer,
                });
                expect(animation).toBeTruthy();
                const bounds = {fLeft: 0, fTop: 0, fRight: 500, fBottom: 500};

                const canvas = surface.getCanvas();
                canvas.clear(CanvasKit.WHITE);
                animation.render(canvas, bounds);
                surface.flush();

                // There was a bug, fixed in https://skia-review.googlesource.com/c/skia/+/241757
                // that seeking again and drawing again revealed.
                animation.seek(0.5);
                canvas.clear(CanvasKit.WHITE);
                animation.render(canvas, bounds);
                surface.flush();

                reportSurface(surface, 'skottie_animgif', done);
            })();
        });
    });

});
