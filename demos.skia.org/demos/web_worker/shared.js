function SkottieExample(CanvasKit, surface, jsonStr, bounds) {
    if (!CanvasKit || !jsonStr) {
        return;
    }
    const animation = CanvasKit.MakeAnimation(jsonStr);
    const duration = animation.duration() * 1000;
    bounds = {fLeft: 0, fTop: 0, fRight: 500, fBottom: 500};

    const skcanvas = surface.getCanvas();

    let firstFrame = performance.now();

    function drawFrame() {
        let now = performance.now();
        let seek = ((now - firstFrame) / duration) % 1.0;

        animation.seek(seek);
        animation.render(skcanvas, bounds);
        skcanvas.flush();
        requestAnimationFrame(drawFrame);
    }
    requestAnimationFrame(drawFrame);
}