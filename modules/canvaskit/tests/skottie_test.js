describe('Skottie behavior', () => {
    let container;

    beforeEach(async () => {
        await EverythingLoaded;
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
    const slotPromise = fetch('/assets/skottie_basic_slots.json')
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

        const animation = CanvasKit.MakeManagedAnimation(promises[0]);
        expect(animation).toBeTruthy();
        animation.setColor('$Icon Fill', CanvasKit.RED);
        animation.seek(0.5);
        animation.render(canvas, bounds);
        animation.delete();
    }, washPromise);

    gm('skottie_slots', (canvas, promises) => {
        if (!CanvasKit.skottie || !CanvasKit.managed_skottie) {
            console.warn('Skipping test because not compiled with skottie');
            return;
        }
        expect(promises[0]).not.toBe('NOT FOUND');
        const bounds = CanvasKit.LTRBRect(0, 0, 500, 500);

        const animation = CanvasKit.MakeManagedAnimation(promises[0],
                                                         {'flightAnim.gif': promises[1]});
        expect(animation).toBeTruthy();

        expect(animation.getScalarSlot('Opacity')).toBe(100);

        expect(animation.setColorSlot('FillsGroup', CanvasKit.RED)).toBeTruthy();
        expect(animation.setScalarSlot('Opacity', 25)).toBeTruthy();
        expect(animation.setVec2Slot('ScaleGroup', [25, 50])).toBeTruthy();
        expect(animation.setImageSlot('ImageSource', 'flighAnim.gif')).toBeTruthy();

        expectArrayCloseTo(animation.getColorSlot('FillsGroup'), CanvasKit.RED, 4);
        expect(animation.getScalarSlot('Opacity')).toBe(25);
        expectArrayCloseTo(animation.getVec2Slot('ScaleGroup'), [25, 50], 4);


        expect(animation.getColorSlot('Bad ID')).toBeFalsy();
        expect(animation.getScalarSlot('Bad ID')).toBeFalsy();
        expect(animation.getVec2Slot('Bad ID')).toBeFalsy();

        animation.seek(0.5);
        animation.render(canvas, bounds);
        animation.delete();
    }, slotPromise, imgPromise);

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

    it('can access dynamic props', () => {
        if (!CanvasKit.skottie || !CanvasKit.managed_skottie) {
            console.warn('Skipping test because not compiled with skottie');
            return;
        }

        const json = `{
            "v": "5.2.1",
            "w": 100,
            "h": 100,
            "fr": 10,
            "ip": 0,
            "op": 100,
            "fonts": {
              "list": [{
                "fName": "test_font",
                "fFamily": "test-family",
                "fStyle": "TestFontStyle"
              }]
            },
            "layers": [
              {
                "ty": 4,
                "nm": "__shape_layer",
                "ind": 0,
                "ip": 0,
                "shapes": [
                  {
                    "ty": "el",
                    "p": { "a": 0, "k": [ 50, 50 ] },
                    "s": { "a": 0, "k": [ 50, 50 ] }
                  },{
                    "ty": "fl",
                    "nm": "__shape_fill",
                    "c": { "a": 0, "k": [ 1, 0, 0] }
                  },{
                    "ty": "tr",
                    "nm": "__shape_opacity",
                    "o": { "a": 0, "k": 50 }
                  }
                ]
              },{
                "ty": 5,
                "nm": "__text_layer",
                "ip": 0,
                "t": {
                  "d": {
                    "k": [{
                      "t": 0,
                      "s": {
                        "f": "test_font",
                        "s": 100,
                        "t": "Foo Bar Baz",
                        "lh": 120,
                        "ls": 12
                      }
                    }]
                  }
                }
              }
            ]
        }`;

        const animation = CanvasKit.MakeManagedAnimation(json, null, '__');
        expect(animation).toBeTruthy();

        {
            const colors = animation.getColorProps();
            expect(colors.length).toEqual(1);
            expect(colors[0].key).toEqual('__shape_fill');
            expect(colors[0].value).toEqual(CanvasKit.ColorAsInt(255,0,0,255));

            const opacities = animation.getOpacityProps();
            expect(opacities.length).toEqual(1);
            expect(opacities[0].key).toEqual('__shape_opacity');
            expect(opacities[0].value).toEqual(50);

            const texts = animation.getTextProps();
            expect(texts.length).toEqual(1);
            expect(texts[0].key).toEqual('__text_layer');
            expect(texts[0].value.text).toEqual('Foo Bar Baz');
            expect(texts[0].value.size).toEqual(100);
        }

        expect(animation.setColor('__shape_fill', [0,1,0,1])).toEqual(true);
        expect(animation.setOpacity('__shape_opacity', 100)).toEqual(true);
        expect(animation.setText('__text_layer', 'baz bar foo', 10)).toEqual(true);

        {
            const colors = animation.getColorProps();
            expect(colors.length).toEqual(1);
            expect(colors[0].key).toEqual('__shape_fill');
            expect(colors[0].value).toEqual(CanvasKit.ColorAsInt(0,255,0,255));

            const opacities = animation.getOpacityProps();
            expect(opacities.length).toEqual(1);
            expect(opacities[0].key).toEqual('__shape_opacity');
            expect(opacities[0].value).toEqual(100);

            const texts = animation.getTextProps();
            expect(texts.length).toEqual(1);
            expect(texts[0].key).toEqual('__text_layer');
            expect(texts[0].value.text).toEqual('baz bar foo');
            expect(texts[0].value.size).toEqual(10);
        }

        expect(animation.setColor('INVALID_KEY', [0,1,0,1])).toEqual(false);
        expect(animation.setOpacity('INVALID_KEY', 100)).toEqual(false);
        expect(animation.setText('INVALID KEY', '', 10)).toEqual(false);
    });
});
