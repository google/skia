function circleCoordinates(origin, radius, radians) {
    return [
        origin[0] + Math.cos(radians) * radius,
        origin[1] + Math.sin(radians) * radius
    ];
}

// Animator handles calling and stopping requestAnimationFrame and keeping track of framerate.
class Animator {
    framesCount = 0;
    totalFramesMs = 0;
    animating = false;
    renderer = null;

    start() {
        if (this.animating === false) {
            this.animating = true;
            this.framesCount = 0;
            const frameStartMs = performance.now();

            const drawFrame = () => {
                if (this.animating && this.renderer) {
                    requestAnimationFrame(drawFrame);
                    this.framesCount++;

                    const [x, y] = circleCoordinates([100, 100], 100, this.framesCount/100);
                    this.renderer.render(x, y);

                    const frameTimeMs = performance.now() - frameStartMs;
                    this.totalFramesMs = frameTimeMs;
                }
            };
            requestAnimationFrame(drawFrame);
        }
    }

    stop() {
        this.animating = false;
    }
}

class SVGRenderer {
    constructor(svgObjectElement) {
        this.svgObjectElement = svgObjectElement;
        this.svgElArray = [];
        for (let xo = 0; xo < 400; xo += 40) {
            for (let yo = 0; yo < 400; yo += 40) {
                const clonedSVG = svgObjectElement.cloneNode(true);
                this.svgElArray.push(clonedSVG);
                svgObjectElement.parentElement.appendChild(clonedSVG);
            }
        }
    }

    render(x, y) {
        let i = 0;
        for (let xo = 0; xo < 400; xo += 40) {
            for (let yo = 0; yo < 400; yo += 40) {
                this.svgElArray[i].style.transform = `translate(${x + xo}px, ${y + yo}px)`;
                i++;
            }
        }
    }
}

class Path2dRenderer {
    constructor(svgData, offscreenCanvas) {
        this.data = svgData.map(([pathString, fillColor]) => [new Path2D(pathString), fillColor]);

        this.ctx = offscreenCanvas.getContext('2d');
    }

    render(x, y) {
        const ctx = this.ctx;

        ctx.clearRect(0, 0, 500, 500);

        for (let xo = 0; xo < 400; xo += 40) {
            for (let yo = 0; yo < 400; yo += 40) {
                ctx.save();
                ctx.translate(x + xo, y + yo);

                for (const [path, fillColor] of this.data) {
                    ctx.fillStyle = fillColor;
                    ctx.fill(path);
                }
                ctx.restore();
            }
        }
    }
}

class CanvasKitRenderer {
    constructor(svgData, offscreenCanvas, CanvasKit) {
        this.CanvasKit = CanvasKit;
        this.data = svgData.map(([pathString, fillColor]) => [
            CanvasKit.MakePathFromSVGString(pathString),
            CanvasKit.parseColorString(fillColor)
        ]);

        this.surface = CanvasKit.MakeWebGLCanvasSurface(offscreenCanvas, null);
        if (!this.surface) {
            throw 'Could not make canvas surface';
        }
        this.canvas = this.surface.getCanvas();

        this.paint = new CanvasKit.SkPaint();
        this.paint.setAntiAlias(true);
        this.paint.setStyle(CanvasKit.PaintStyle.Fill);
    }

    render(x, y) {
        const canvas = this.canvas;

        canvas.clear(this.CanvasKit.WHITE);

        for (let xo = 0; xo < 400; xo += 40) {
            for (let yo = 0; yo < 400; yo += 40) {
                canvas.save();
                canvas.translate(x + xo, y + yo);

                for (const [path, color] of this.data) {
                    this.paint.setColor(color);
                    canvas.drawPath(path, this.paint);
                }
                canvas.restore();
            }
        }
        this.surface.flush();
    }
}
