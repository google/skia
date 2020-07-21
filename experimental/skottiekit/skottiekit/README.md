A WASM version of Skottie (Lottie with Skia).

# Getting Started

To use the library, run `npm install skottiekit-wasm` and then simply include it:

    <script src="/node_modules/skottiekit-wasm/bin/skottiekit.js"></script>
    const loadKit = SkottieKitInit({
        locateFile: (file) => '/node_modules/skottiekit-wasm/bin/'+file,
    });
    // Load the animation Lottie JSON file.
    const loadLottie = fetch('/path/to/lottie.json').then((resp) => resp.text());

    Promise.all([loadKit, loadLottie]).then((values) => {
        const [SkottieKit, lottieJSON] = values;
        const animation = SkottieKit.MakeManagedAnimation(lottieJSON);
        const duration = animation.duration() * 1000;
        // Assumes there's a <canvas id="my_canvas"> somewhere
        const surface = SkottieKit.MakeCanvasSurface("my_canvas");

        const firstFrame = Date.now();
        const clearColor = SkottieKit.WHITE;

        function drawFrame(canvas) {
            // seek takes a float from 0.0 to 1.0
            const seek = ((Date.now() - firstFrame) / duration) % 1.0;
            animation.seek(seek);

            canvas.clear(clearColor);
            animation.render(canvas, bounds);
            surface.requestAnimationFrame(drawFrame);
        }
        surface.requestAnimationFrame(drawFrame);
    })

As with all npm packages, there's a freely available CDN via unpkg.com:

    <script src="https://unpkg.com/skottiekit-wasm@0.1.0/bin/skottiekit.js"></script>
    const loadKit SkottieKitInit({
         locateFile: (file) => 'https://unpkg.com/skottiekit-wasm@0.1.0/bin/'+file,
    })
