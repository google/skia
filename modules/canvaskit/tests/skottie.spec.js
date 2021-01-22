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
        precision = precision || 14 // digits of precision in base 10
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
});
