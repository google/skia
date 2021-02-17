describe('Skottie behavior', () => {
    let container;

    beforeEach(async () => {
        await LoadCanvasKit;
        container = document.createElement('div');
        container.innerHTML = `
            <canvas width=600 height=600 id=test></canvas>
            <canvas width=600 height=600 id=report></canvas>`;
        document.body.appendChild(container);
    });

    afterEach(() => {
        document.body.removeChild(container);
    });

    const expectArrayCloseTo = (a, b, precision) => {
        precision = precision || 14; // digits of precision in base 10
        expect(a.length).toEqual(b.length);
        for (let i=0; i<a.length; i++) {
          expect(a[i]).toBeCloseTo(b[i], precision);
        }
    };

    const imgPromise = fetch('/assets/flightAnim.gif')
        .then((response) => response.arrayBuffer());
    const jsonPromise = fetch('/assets/animated_gif.json')
        .then((response) => response.text());
    const washPromise = fetch('/assets/map-shield.json')
        .then((response) => response.text());

    gm('skottie_animgif', (canvas, promises) => {
        if (!CanvasKit.skottie || !CanvasKit.managed_skottie) {
            console.warn('Skipping test because not compiled with skottie');
            return;
        }
        expect(promises[1]).not.toBe('NOT FOUND');
        const animation = CanvasKit.MakeManagedAnimation(promises[1], {
            'flightAnim.gif': promises[0],
        });
        expect(animation).toBeTruthy();
        const bounds = CanvasKit.LTRBRect(0, 0, 500, 500);

        const size = animation.size();
        expectArrayCloseTo(size, Float32Array.of(800, 600), 4);

        canvas.clear(CanvasKit.WHITE);
        animation.render(canvas, bounds);

        // We intentionally make the length of this array 5 and add a sentinel value
        // of 999 so we can make sure the bounds are copied into this rect and a new
        // one is not allocated.
        const damageRect = Float32Array.of(0, 0, 0, 0, 999);

        // There was a bug, fixed in https://skia-review.googlesource.com/c/skia/+/241757
        // that seeking again and drawing again revealed.
        animation.seek(0.5, damageRect);
        expectArrayCloseTo(damageRect, Float32Array.of(0, 0, 800, 600, 999), 4);

        canvas.clear(CanvasKit.WHITE);
        animation.render(canvas, bounds);
        animation.delete();
    }, imgPromise, jsonPromise);

    gm('skottie_setcolor', (canvas, promises) => {
        if (!CanvasKit.skottie || !CanvasKit.managed_skottie) {
            console.warn('Skipping test because not compiled with skottie');
            return;
        }
        expect(promises[0]).not.toBe('NOT FOUND');
        const bounds = CanvasKit.LTRBRect(0, 0, 500, 500);
        canvas.clear(CanvasKit.WHITE);

        const animation = CanvasKit.MakeManagedAnimation(promises[0]);
        expect(animation).toBeTruthy();
        animation.setColor('$Icon Fill', CanvasKit.RED);
        animation.seek(0.5);
        animation.render(canvas, bounds);
        animation.delete();
    }, washPromise);

    it('can load audio assets', (done) => {
        if (!CanvasKit.skottie || !CanvasKit.managed_skottie) {
            console.warn('Skipping test because not compiled with skottie');
            return;
        }
        const mockSoundMap = {
            map : new Map(),
            getPlayer : function(name) {return this.map.get(name)},
            setPlayer : function(name, player) {this.map.set(name, player)},
        };
        function mockPlayer(name) {
            this.name = name;
            this.wasPlayed = false,
            this.seek = function(t) {
                this.wasPlayed = true;
            }
        }
        for (let i = 0; i < 20; i++) {
            var name = 'audio_' + i;
            mockSoundMap.setPlayer(name, new mockPlayer(name));
        }
        fetch('/assets/audio_external.json')
        .then((response) => response.text())
        .then((lottie) => {
            const animation = CanvasKit.MakeManagedAnimation(lottie, null, null, mockSoundMap);
            expect(animation).toBeTruthy();
            // 190 frames in sample lottie
            for (let t = 0; t < 190; t++) {
                animation.seekFrame(t);
            }
            animation.delete();
            for(const player of mockSoundMap.map.values()) {
                expect(player.wasPlayed).toBeTrue(player.name + " was not played");
            }
            done();
        });
    });

    it('can get logs', (done) => {
        if (!CanvasKit.skottie || !CanvasKit.managed_skottie) {
            console.warn('Skipping test because not compiled with skottie');
            return;
        }

        const logger = {
           errors:   [],
           warnings: [],

           reset: function() { this.errors = []; this.warnings = []; },

           // Logger API
           onError:   function(err) { this.errors.push(err)   },
           onWarning: function(wrn) { this.warnings.push(wrn) }
        };

        {
            const json = `{
                "v": "5.2.1",
                "w": 100,
                "h": 100,
                "fr": 10,
                "ip": 0,
                "op": 100,
                "layers": [{
                    "ty": 3,
                    "nm": "null",
                    "ind": 0,
                    "ip": 0
                }]
            }`;
            const animation = CanvasKit.MakeManagedAnimation(json, null, null, null, logger);
            expect(animation).toBeTruthy();
            expect(logger.errors.length).toEqual(0);
            expect(logger.warnings.length).toEqual(0);
        }

        {
            const json = `{
                "v": "5.2.1",
                "w": 100,
                "h": 100,
                "fr": 10,
                "ip": 0,
                "op": 100,
                "layers": [{
                    "ty": 2,
                    "nm": "image",
                    "ind": 0,
                    "ip": 0
                }]
            }`;
            const animation = CanvasKit.MakeManagedAnimation(json, null, null, null, logger);
            expect(animation).toBeTruthy();
            expect(logger.errors.length).toEqual(1);
            expect(logger.warnings.length).toEqual(0);

            // Image layer missing refID
            expect(logger.errors[0].includes('missing ref'));
            logger.reset();
        }

        {
            const json = `{
                "v": "5.2.1",
                "w": 100,
                "h": 100,
                "fr": 10,
                "ip": 0,
                "op": 100,
                "layers": [{
                    "ty": 1,
                    "nm": "solid",
                    "sw": 100,
                    "sh": 100,
                    "sc": "#aabbcc",
                    "ind": 0,
                    "ip": 0,
                    "ef": [{
                      "mn": "FOO"
                    }]
                }]
            }`;
            const animation = CanvasKit.MakeManagedAnimation(json, null, null, null, logger);
            expect(animation).toBeTruthy();
            expect(logger.errors.length).toEqual(0);
            expect(logger.warnings.length).toEqual(1);

            // Unsupported effect FOO
            expect(logger.warnings[0].includes('FOO'));
            logger.reset();
        }

        done();
    });
});
