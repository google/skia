const DOM_RENDER_MODE = 0;
const CANVAS2D_RENDER_MODE = 1;
const CANVASKIT_RENDER_MODE = 2;

let current_render_mode = DOM_RENDER_MODE;

const ckLoaded = CanvasKitInit({locateFile: (file) => 'https://particles.skia.org/static/'+file});

window.onload = async () => {
    const mysvg = document.getElementById('test-svg').contentDocument.childNodes[0];

    // Canvas2d initialization
    const paths=[], pts=[];
    parsePaths(mysvg, paths, pts);
    const canvas2D = document.createElement('canvas');
    canvas2D.height = canvas2D.width = 500;
    const targetCanvas = document.getElementById('canvas2d-canvas');
    const targetCtx = targetCanvas.getContext('2d');
    const ctx = canvas2D.getContext("2d");

    // CanvasKit initialization
    const CanvasKit = await ckLoaded;
    const ckPaths=[], ckPts=[];
    parsePathsCanvasKit(CanvasKit, mysvg, ckPaths, ckPts);
    const canvas = document.getElementById('canvaskit-canvas');
    CanvasKit.GetWebGLContext(canvas, {antialias: true});
    const sksurface = CanvasKit.MakeWebGLCanvasSurface(canvas, null);
    if (!sksurface) {
      throw 'Could not make canvas surface';
    }
    if (sksurface.sampleCnt() <= 1) {
      throw 'MSAA not supported';
    }

    const domFpsEl = document.getElementById('dom-fpslabel');
    const canvas2dFpsEl = document.getElementById('canvas2d-fpslabel');
    const canvaskitFpsEl = document.getElementById('canvaskit-fpslabel');

    const canvas2dRenderGenerator = partialRenderGeneratorCanvas2D(pts, paths, ctx);
    const domRenderGenerator = partialRenderGeneratorDOM(pts, paths, mysvg);
    const canvasKitRenderGenerator = partialRenderGeneratorCanvasKit(ckPts, ckPaths, CanvasKit);

    window.requestAnimationFrame(function drawFrame(skcanvas) {
        if (current_render_mode === DOM_RENDER_MODE) {
            const totalFrameTimeMs = domRenderGenerator.next().value;
            if (totalFrameTimeMs !== null) {
                domFpsEl.innerText = (1000 / totalFrameTimeMs).toFixed(4) + ' fps';
            }
            window.requestAnimationFrame(drawFrame);
        }
        if (current_render_mode === CANVAS2D_RENDER_MODE) {
            const totalFrameTimeMs = canvas2dRenderGenerator.next().value;
            if (totalFrameTimeMs !== null) {
                targetCtx.drawImage(canvas2D, 0, 0);
                canvas2dFpsEl.innerText = (1000 / totalFrameTimeMs).toFixed(4) + ' fps';
            }
            window.requestAnimationFrame(drawFrame);
        }
        if (current_render_mode === CANVASKIT_RENDER_MODE) {
            if (skcanvas && skcanvas.drawPath) {
                const totalFrameTimeMs = canvasKitRenderGenerator.next(skcanvas).value;
                if (totalFrameTimeMs !== null) {
                    canvaskitFpsEl.innerText = (1000 / totalFrameTimeMs).toFixed(4) + ' fps';
                }
            }
            sksurface.requestAnimationFrame(drawFrame);
        }
    });

    document.getElementById('dom-button').addEventListener('click', () => {
        current_render_mode = DOM_RENDER_MODE;
        document.getElementById('dom-inprogress-indicator').hidden = false;
        document.getElementById('canvas2d-inprogress-indicator').hidden = true;
        document.getElementById('canvaskit-inprogress-indicator').hidden = true;
    });
    document.getElementById('canvas2d-button').addEventListener('click', () => {
        current_render_mode = CANVAS2D_RENDER_MODE;
        document.getElementById('canvas2d-inprogress-indicator').hidden = false;
        document.getElementById('dom-inprogress-indicator').hidden = true;
        document.getElementById('canvaskit-inprogress-indicator').hidden = true;
    });
    document.getElementById('canvaskit-button').addEventListener('click', () => {
        current_render_mode = CANVASKIT_RENDER_MODE;
        document.getElementById('canvaskit-inprogress-indicator').hidden = false;
        document.getElementById('dom-inprogress-indicator').hidden = true;
        document.getElementById('canvas2d-inprogress-indicator').hidden = true;
    });
    document.getElementById('button4').addEventListener('click', () => {
        CanvasKit.ToggleTess();
    });
}

const FRAME_TIME_BUDGET_MS = 33.333; // Equivalent to ~30fps
function* partialRenderGeneratorCanvas2D(pts, paths, ctx) {
    const animator = new PointsAnimator(pts);
    while (true) {
        let totalFrameTimeMs = 0;
        let partialframeStartTimeMs = performance.now();
        let ptsIdx = 0;
        let wavyPts = animator.animate();
        let path = paths[0];
        for (path of paths) {
            let path2d = new Path2D();
            const definePathGenerator = definePath(path2d, path.v, wavyPts, ptsIdx);
            for (const yieldedPtsIdx of definePathGenerator) {
                ptsIdx = yieldedPtsIdx;

                const partialFrameTimeMs = performance.now() - partialframeStartTimeMs;
                if (partialFrameTimeMs > FRAME_TIME_BUDGET_MS) {
                    totalFrameTimeMs += partialFrameTimeMs;
                    yield null;
                    partialframeStartTimeMs = performance.now();
                }
            }
            ctx.fillStyle = path.f;
            ctx.fill(path2d);

            const partialFrameTimeMs = performance.now() - partialframeStartTimeMs;
            if (partialFrameTimeMs > FRAME_TIME_BUDGET_MS) {
                totalFrameTimeMs += partialFrameTimeMs;
                yield null;
                partialframeStartTimeMs = performance.now();
            }
        }
        const partialFrameTimeMs = performance.now() - partialframeStartTimeMs;
        totalFrameTimeMs += partialFrameTimeMs;
        yield totalFrameTimeMs;
    }
}
function* partialRenderGeneratorDOM(pts, paths, mysvg) {
    const animator = new PointsAnimator(pts);
    while (true) {
        let totalFrameTimeMs = 0;
        let partialframeStartTimeMs = performance.now();

        let wavyPts = animator.animate();
        let path = paths[0];
        for (path of paths) {
            let updatePathsGenerator = updatePaths(mysvg, paths, 0, wavyPts);
            while (updatePathsGenerator.next().value === null) {
                const partialFrameTimeMs = performance.now() - partialframeStartTimeMs;
                if (partialFrameTimeMs > FRAME_TIME_BUDGET_MS) {
                    totalFrameTimeMs += partialFrameTimeMs;
                    yield null;
                    partialframeStartTimeMs = performance.now();
                }
            }
        }
        const partialFrameTimeMs = performance.now() - partialframeStartTimeMs;
        totalFrameTimeMs += partialFrameTimeMs;
        yield totalFrameTimeMs;
    }
}
function* partialRenderGeneratorCanvasKit(pts, paths, ck) {
    const waves = new CanvasKitAnimator(new Float32Array(pts), ck);
    let skcanvas = yield;
    while (true) {
        let totalFrameTimeMs = 0;
        let partialframeStartTimeMs = performance.now();

        const wavyPts = waves.animate();
        for (const path of paths) {
            const pts = wavyPts.subarray(path.ptsIdx, path.ptsEndIdx);
            // FIXME: parse fill type from svg.
            const skPath = ck.SkPath.MakeFromVerbsPointsWeights(path.ckVerbs, pts, null,
                                                                ck.FillType.Winding);
            skcanvas.drawPath(skPath, path.skPaint);
            skPath.delete();

            const partialFrameTimeMs = performance.now() - partialframeStartTimeMs;
            if (partialFrameTimeMs > FRAME_TIME_BUDGET_MS) {
                totalFrameTimeMs += partialFrameTimeMs;
                skcanvas = yield null;
                partialframeStartTimeMs = performance.now();
            }
        }
        const partialFrameTimeMs = performance.now() - partialframeStartTimeMs;
        totalFrameTimeMs += partialFrameTimeMs;
        skcanvas = yield totalFrameTimeMs;
    }
}

class PointsAnimator {
    constructor(pts) {
        this.pts = new Float32Array(pts);
        this.animatedPts = new Float32Array(pts.length);
        let fTsec = 0;
        const fAmplitudes = new Float32Array(4);
        const fFrequencies = new Float32Array(4);
        const fDirsX = new Float32Array(4);
        const fDirsY = new Float32Array(4);
        const fSpeeds = new Float32Array(4);
        const fOffsets = new Float32Array(4);
        const kAverageAngle = 3 * Math.PI / 8.0;
        const kMaxOffsetAngle = Math.PI / 3.0;
        const pixelsPerMeter = 0.06 * 1500;
        const medianWavelength = 8 * pixelsPerMeter;
        const medianWaveAmplitude = 0.01 * 4 * pixelsPerMeter;
        const gravity = 9.8 * pixelsPerMeter;
        for (let i = 0; i < 4; ++i) {
            const offsetAngle = (Math.random() * 2 - 1) * kMaxOffsetAngle;
            const intensity = Math.pow(2, Math.random() * 2 - 1);
            const wavelength = intensity * medianWavelength;
            fAmplitudes[i] = intensity * medianWaveAmplitude;
            fFrequencies[i] = 2 * Math.PI / wavelength;
            fDirsX[i] = Math.cos(kAverageAngle + offsetAngle);
            fDirsY[i] = Math.sin(kAverageAngle + offsetAngle);
            fSpeeds[i] = -Math.sqrt(gravity * 2 * Math.PI / wavelength);
            fOffsets[i] = Math.random() * 2 * Math.PI;
        }

        this.animate = function () {
            const pts = this.pts;
            const out = this.animatedPts;
            for (let i = 0; i < pts.length; i += 2) {
                let x = pts[i], y = pts[i + 1];
                for (let i = 0; i < 4; ++i) {
                    const t = fFrequencies[i] * (fDirsX[i] * x + fDirsY[i] * y) +
                        fSpeeds[i] * fTsec +
                        fOffsets[i];
                    const sinT = Math.sin(t);
                    const height = fAmplitudes[i] * sinT * sinT;
                    x += height * fDirsX[i];
                    y += height * fDirsY[i];
                }
                out[i] = x;
                out[i + 1] = y;
            }
            fTsec += 1;
            return out;
        };
    }
}

class CanvasKitAnimator {
    constructor(pts, ck) {
        const numWaves = 6;
        const fAmplitudes = new Float32Array(numWaves);
        const fFrequencies = new Float32Array(numWaves);
        const fDirsX = new Float32Array(numWaves);
        const fDirsY = new Float32Array(numWaves);
        const fSpeeds = new Float32Array(numWaves);
        const fOffsets = new Float32Array(numWaves);

        const kAverageAngle = 3 * Math.PI / 8.0;
        const kMaxOffsetAngle = Math.PI / 3.0;
        const pixelsPerMeter = 0.06 * 1500;
        const medianWavelength = 8 * pixelsPerMeter;
        const medianWaveAmplitude = 0.01 * 4 * pixelsPerMeter;
        const gravity = 9.8 * pixelsPerMeter;

        for (let i = 0; i < numWaves; ++i) {
            const offsetAngle = (Math.random() * 2 - 1) * kMaxOffsetAngle;
            const intensity = Math.pow(2, Math.random() * 2 - 1);
            const wavelength = intensity * medianWavelength;

            fAmplitudes[i] = intensity * medianWaveAmplitude;
            fFrequencies[i] = 2 * Math.PI / wavelength;
            fDirsX[i] = Math.cos(kAverageAngle + offsetAngle);
            fDirsY[i] = Math.sin(kAverageAngle + offsetAngle);
            fSpeeds[i] = -Math.sqrt(gravity * 2 * Math.PI / wavelength);
            fOffsets[i] = Math.random() * 2 * Math.PI;
        }

        const ringBufferSize = 3;
        const framebufferWidth = 1024;
        const numBatchesPerFramebuffer = 3;
        const batchHeight = Math.ceil(pts.length / 2 / framebufferWidth);
        const framebufferHeight = batchHeight * numBatchesPerFramebuffer;
        const offscreen = new OffscreenCanvas(framebufferWidth, framebufferHeight);
        const gl = offscreen.getContext('webgl2');
        if (!gl.getExtension("EXT_color_buffer_float")) {
            throw "EXT_color_buffer_float not supported";
        }

        let vertexShader = gl.createShader(gl.VERTEX_SHADER);
        gl.shaderSource(vertexShader, `#version 300 es
      out highp vec2 texcoord;
      void main() {
        texcoord.x = (gl_VertexID < 2) ? 0.0 : 1.0;
        texcoord.y = ((gl_VertexID & 1) == 0) ? 0.0 : ${numBatchesPerFramebuffer}.0;
        float x, y;
        x = (gl_VertexID < 2) ? -1.0 : +1.0;
        y = ((gl_VertexID & 1) == 0) ? -1.0 : +1.0;
        gl_Position = vec4(x, y, 0.0, 1.0);
      }
    `);
        gl.compileShader(vertexShader);
        if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
            console.log('Error compiling vertex shader:');
            console.log(gl.getShaderInfoLog(vertexShader));
            throw "bad shader";
        }

        let fragShader = gl.createShader(gl.FRAGMENT_SHADER);
        gl.shaderSource(fragShader, `#version 300 es
      precision highp float;
      uniform highp float amplitudes[${numWaves}];
      uniform highp float frequencies[${numWaves}];
      uniform highp float dirsX[${numWaves}];
      uniform highp float dirsY[${numWaves}];
      uniform highp float speeds[${numWaves}];
      uniform highp float offsets[${numWaves}];
      uniform highp float tsec;
      uniform highp sampler2D tex;
      in highp vec2 texcoord;
      out highp vec4 outPts;
      void main() {
        float localTsec = tsec + floor(texcoord.y)/60.0;
        vec4 pts = texture(tex, texcoord);
        vec2 xx = pts.xz;
        vec2 yy = pts.yw;
        for (int i = 0; i < ${numWaves}; ++i) {
            vec2 tt = frequencies[i] * (dirsX[i] * xx + dirsY[i] * yy) + speeds[i] * localTsec +
                      offsets[i];
            vec2 sinTT = sin(tt);
            vec2 heights = amplitudes[i] * sinTT * sinTT;
            xx += heights * dirsX[i];
            yy += heights * dirsY[i];
        }
        outPts = vec4(xx.x, yy.x, xx.y, yy.y);
      }
    `);
        gl.compileShader(fragShader);
        if (!gl.getShaderParameter(fragShader, gl.COMPILE_STATUS)) {
            console.log('Error compiling frag shader:');
            console.log(gl.getShaderInfoLog(fragShader));
            throw "bad shader";
        }

        let program = gl.createProgram();
        gl.attachShader(program, vertexShader);
        gl.attachShader(program, fragShader);
        gl.linkProgram(program);
        if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
            console.log('Error linking shader:');
            console.log(gl.getProgramInfoLog(program));
            throw "bad shader";
        }
        gl.useProgram(program);
        gl.uniform1fv(gl.getUniformLocation(program, "amplitudes"), fAmplitudes);
        gl.uniform1fv(gl.getUniformLocation(program, "frequencies"), fFrequencies);
        gl.uniform1fv(gl.getUniformLocation(program, "dirsX"), fDirsX);
        gl.uniform1fv(gl.getUniformLocation(program, "dirsY"), fDirsY);
        gl.uniform1fv(gl.getUniformLocation(program, "speeds"), fSpeeds);
        gl.uniform1fv(gl.getUniformLocation(program, "offsets"), fOffsets);
        gl.uniform1i(gl.getUniformLocation(program, "tex"), 0);
        const tsecLocation = gl.getUniformLocation(program, "tsec");

        let mWavyPts = ck.Malloc(Float32Array, framebufferWidth * framebufferHeight * 4);
        mWavyPts.ste;

        let ptsArray = new Float32Array(framebufferWidth * batchHeight * 4);
        for (let j = 0; j < ptsArray.length; ++j) {
            ptsArray[j] = pts[j];
        }
        const tex = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, tex);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA32F, framebufferWidth, batchHeight, 0, gl.RGBA,
            gl.FLOAT, ptsArray);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);

        let fTsec = 0;

        const framebuffers = [];
        for (let i = 0; i < ringBufferSize; ++i) {
            let renderbuffer = gl.createRenderbuffer();
            gl.bindRenderbuffer(gl.RENDERBUFFER, renderbuffer);
            gl.renderbufferStorage(gl.RENDERBUFFER, gl.RGBA32F, framebufferWidth, framebufferHeight);
            let framebuffer = gl.createFramebuffer();
            gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
            gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.RENDERBUFFER,
                renderbuffer);
            framebuffers.push(framebuffer);
            gl.uniform1f(tsecLocation, fTsec);
            gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
            fTsec += numBatchesPerFramebuffer / 60;
        }

        let readIdx = 0, writeIdx = 3;
        gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[readIdx]);
        let n = 0;
        this.animate = function () {
            if (n == 0) {
                gl.readPixels(0, 0, framebufferWidth, framebufferHeight, gl.RGBA, gl.FLOAT,
                    mWavyPts.toTypedArray());

                gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[writeIdx]);
                gl.uniform1f(tsecLocation, fTsec);
                gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

                gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[readIdx]);
                gl.flush();

                readIdx = (readIdx + 1) % ringBufferSize;
                writeIdx = (writeIdx + 1) % ringBufferSize;
                fTsec += numBatchesPerFramebuffer / 60;
            }
            n = (n + 1) % numBatchesPerFramebuffer;

            return mWavyPts.subarray(n * framebufferWidth * batchHeight * 4);
        };
    }
}

