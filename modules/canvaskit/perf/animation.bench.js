describe('CanvasKit\'s Animation', function() {

    const LOTTIE_ANIMATIONS = ['lego_loader', 'drinks', 'confetti', 'onboarding'];

    let container = document.createElement('div');
    document.body.appendChild(container);


    beforeEach(function() {
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>`;
    });

    afterEach(function() {
        container.innerHTML = '';
    });

    function fetchAndText(url) {
        return new Promise(function(resolve, reject) {
            fetch(url).then((resp) => {
                    resp.text().then((str) => {
                        expect(str).toBeTruthy();
                        resolve(str);
                    });
                }).catch(reject);
        });
    }

    LOTTIE_ANIMATIONS.forEach((animStr) => {
        let promises = [fetchAndText(`/assets/${animStr}.json`), LoadCanvasKit];

        it(`animation loading for ${animStr}`, function(done) {
            let jsonStr = '';
            function setup(ctx) {
                expect(jsonStr).toBeTruthy();
            }

            function test(ctx) {
                const animation = CanvasKit.MakeAnimation(jsonStr);
                animation.delete();
            }

            function teardown(ctx) {}

            Promise.all(promises).then((responses) => {
                // The result from the first promise, that is, the JSON string
                // fetched by fetchAndText
                jsonStr = responses[0];
                benchmarkAndReport(`${animStr}_animation_load`, setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

        it(`animation frames in order for ${animStr}`, function(done) {
            let jsonStr = '';
            function setup(ctx) {
                expect(jsonStr).toBeTruthy();
                ctx.animation = CanvasKit.MakeAnimation(jsonStr);
                expect(ctx.animation).toBeTruthy();
                ctx.timer = 0;
            }

            function test(ctx) {
                ctx.animation.seek(ctx.timer);
                ctx.timer += 0.01;
                if (ctx.timer > 1.0) {
                    ctx.timer = 0;
                }
            }

            function teardown(ctx) {
                ctx.animation.delete();
            }

            Promise.all(promises).then((responses) => {
                // The result from the first promise, that is, the JSON string
                // fetched by fetchAndText
                jsonStr = responses[0];
                benchmarkAndReport(`${animStr}_animation_in_order`, setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

        it(`animation frames in random order for ${animStr}`, function(done) {
            let jsonStr = '';
            function setup(ctx) {
                expect(jsonStr).toBeTruthy();
                ctx.animation = CanvasKit.MakeAnimation(jsonStr);
                expect(ctx.animation).toBeTruthy();
            }

            function test(ctx) {
                ctx.animation.seek(Math.random());
            }

            function teardown(ctx) {
                ctx.animation.delete();
            }

            Promise.all(promises).then((responses) => {
                // The result from the first promise, that is, the JSON string
                // fetched by fetchAndText
                jsonStr = responses[0];
                benchmarkAndReport(`${animStr}_animation_random_order`, setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

        it(`renders to an HTML canvas ${animStr}`, function(done) {
            let jsonStr = '';
            function setup(ctx) {
                expect(jsonStr).toBeTruthy();
                ctx.animation = CanvasKit.MakeAnimation(jsonStr);
                expect(ctx.animation).toBeTruthy();
                ctx.animation.seek(0.5);
                ctx.surface = CanvasKit.MakeCanvasSurface('test');
                ctx.canvas = ctx.surface.getCanvas();
                ctx.clear = CanvasKit.Color(255, 255, 255, 0.0); // transparent
            }

            function test(ctx) {
                // This emulates what would need to be done do accurately
                // draw one frame.
                ctx.canvas.clear(ctx.clear);
                ctx.animation.render(ctx.canvas);
                ctx.surface.flush();
            }

            function teardown(ctx) {
                ctx.animation.delete();
                ctx.surface.dispose(); // ctx.canvas will also be cleaned up
            }

            Promise.all(promises).then((responses) => {
                // The result from the first promise, that is, the JSON string
                // fetched by fetchAndText
                jsonStr = responses[0];
                benchmarkAndReport(`${animStr}_animation_render_flush`, setup, test, teardown).then(() => {
                    done();
                }).catch(reportError(done));
            });
        });

    });

});
