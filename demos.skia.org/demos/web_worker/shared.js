function SkottieExample(CanvasKit, surface, jsonStr, bounds) {
    if (!CanvasKit || !jsonStr) {
        return;
    }
    const animation = CanvasKit.MakeAnimation(jsonStr);
    const duration = animation.duration() * 1000;
    bounds = {fLeft: 0, fTop: 0, fRight: 500, fBottom: 500};

    const firstFrame = performance.now();

    function drawFrame(skcanvas) {
        const now = performance.now();
        const seek = ((now - firstFrame) / duration) % 1.0;

        animation.seek(seek);
        animation.render(skcanvas, bounds);

        surface.requestAnimationFrame(drawFrame);
    }
    surface.requestAnimationFrame(drawFrame);
}