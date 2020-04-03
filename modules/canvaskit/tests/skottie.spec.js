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

    const imgPromise = fetch('/assets/flightAnim.gif')
        .then((response) => response.arrayBuffer());
    const jsonPromise = fetch('/assets/animated_gif.json')
        .then((response) => response.text());

    gm('skottie_animgif', (canvas, promises) => {
        if (!CanvasKit.skottie || !CanvasKit.managed_skottie) {
            console.warn('Skipping test because not compiled with skottie');
            return;
        }
        const animation = CanvasKit.MakeManagedAnimation(promises[1], {
            'flightAnim.gif': promises[0],
        });
        expect(animation).toBeTruthy();
        const bounds = {fLeft: 0, fTop: 0, fRight: 500, fBottom: 500};

        canvas.clear(CanvasKit.WHITE);
        animation.render(canvas, bounds);

        // There was a bug, fixed in https://skia-review.googlesource.com/c/skia/+/241757
        // that seeking again and drawing again revealed.
        animation.seek(0.5);
        canvas.clear(CanvasKit.WHITE);
        animation.render(canvas, bounds);
        animation.delete();
    }, imgPromise, jsonPromise);
});
