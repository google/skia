const char2verb = {'M':0, 'L':1, 'Q':2, 'C':4, 'Z':5};

function parseColorInt(name) {
    if (!name || name == "black") {
        return 0xff000000;
    }
    if (name == "white") {
        return 0xffffffff;
    }
    if (name.startsWith('#')) {
        const val = parseInt(name.substring(1), 16);
        if (name.length < 7) {
            return 0xff000000 + ((val & 0xf00) << 12) + ((val & 0xf0) << 8) + ((val & 0xf) << 4);
        }
        return 0xff000000 + val;
    }
    return 0xffff0000;
}

let gradients = {};
function parseGradient(ck, node) {
    const id = node.getAttributeNS(null, 'id');
    let x1 = parseFloat(node.getAttributeNS(null, 'x1'));
    let y1 = parseFloat(node.getAttributeNS(null, 'y1'));
    let x2 = parseFloat(node.getAttributeNS(null, 'x2'));
    let y2 = parseFloat(node.getAttributeNS(null, 'y2'));
    const trans = node.getAttributeNS(null, 'gradientTransform');
    let matrix = null;
    if (trans) {
        let str = trans.substring(trans.indexOf('('));
        matrix = [1,0,0, 0,1,0, 0,0,1];
        for (let i = 0; i < 6; ++i) {
            matrix[i] = parseFloat(str);
            str = str.substring(str.indexOf(' '));
        }
    }
    let offsets=[], colors=[];
    for (child of node.childNodes) {
        if (child.nodeName == "stop") {
            offsets.push(parseInt(child.getAttributeNS(null, 'offset')));
            colors.push(parseColorInt(child.getAttributeNS(null, 'stop-color')));
        }
    }
    const skPaint = new ck.SkPaint();
    skPaint.setAntiAlias(true);
    // skPaint.setShader(ck.SkShader.MakeLinearGradient(
    //         [x1, y1], [x2, y2], new Uint32Array(colors), offsets, ck.TileMode.Mirror, matrix, 1));
    skPaint.setColorInt(colors[colors.length>>1]);
    gradients[id] = skPaint;
}

function parsePaths(ck, node, paths, allPts) {
    if (node.nodeName == "linearGradient") {
        parseGradient(ck, node);
        return;
    }
    if (node.nodeName == "path") {
        function StringParser(str) {
            this.str = str.trimStart();
            this.hasNext = function() {
                return this.str !== "";
            }
            this.nextVerb = function() {
                const verbChar = this.str[0];
                if (!(verbChar in char2verb)) {
                    throw "bad verb";
                }
                this.str = this.str.substring(1).trimStart();
                return char2verb[verbChar];
            }
            this.nextFloat = function() {
                const val = parseFloat(this.str);
                if (isNaN(val)) {
                    throw "NaN";
                }
                let endIdx = 0;
                while ("0123456789eE-+.".includes(this.str[endIdx])) {
                    ++endIdx;
                }
                this.str = this.str.substring(endIdx).trimStart();
                return val;
            }
            this.nextFloats = function(n) {
                let floats = [];
                for (let i = 0; i < n; ++i) {
                    floats.push(this.nextFloat());
                }
                return floats;
            }
        };
        const parser = new StringParser(node.getAttributeNS(null, 'd'));
        const skPath = new ck.SkPath();
        let ptsStartIdx = allPts.length;
        let verbs = [];
        while (parser.hasNext()) {
            const verb = parser.nextVerb();
            verbs.push(verb);
            let pts;
            switch (verb) {
                case ck.MOVE_VERB:
                    pts = parser.nextFloats(2);
                    skPath.moveTo(...pts);
                    allPts.push(...pts);
                    break;
                case ck.LINE_VERB:
                    pts = parser.nextFloats(2);
                    skPath.lineTo(...pts);
                    allPts.push(...pts);
                    break;
                case ck.QUAD_VERB:
                    pts = parser.nextFloats(4);
                    skPath.quadTo(...pts);
                    allPts.push(...pts);
                    break;
                case ck.CONIC_VERB:
                    throw "No conic support yet."
                case ck.CUBIC_VERB:
                    pts = parser.nextFloats(6);
                    skPath.cubicTo(...pts);
                    allPts.push(...pts);
                    break;
                case ck.CLOSE_VERB:
                    skPath.close();
                    break;
             }
        }
        let skPaint = new ck.SkPaint();
        skPaint.setAntiAlias(true);
        const fill = node.getAttributeNS(null, 'fill');
        if (fill == "none") {
            skPaint.setStyle(ck.PaintStyle.Stroke);
            skPaint.setColorInt(parseColorInt(node.getAttributeNS(null, 'stroke')));
            skPaint.setStrokeWidth(parseFloat(node.getAttributeNS(null, 'stroke-width')));
            skPaint.setStrokeMiter(parseFloat(node.getAttributeNS(null, 'stroke-miterlimit')));
        } else if (fill && fill.startsWith("url(#")) {
            const id = fill.substring(fill.indexOf("#") + 1, fill.indexOf(")"));
            skPaint = gradients[id];
            if (!skPaint) {
                throw "Bad gradient."
            }
        } else {
            skPaint.setColorInt(parseColorInt(fill));
            const opacity = node.getAttributeNS(null, 'fill-opacity');
            if (opacity) {
                skPaint.setAlphaf(parseFloat(opacity));
            }
        }
        paths.push({ckpath:skPath, ckVerbs:new Uint8Array(verbs), skPaint:skPaint, ptsIdx:ptsStartIdx,
                    ptsEndIdx:allPts.length});
    }
    for (child of node.childNodes) {
        parsePaths(ck, child, paths, allPts);
    }
}

function launchDemo(ck) {
    window.onkeypress = function(event) {
        if (event.keyCode == 't'.charCodeAt(0)) {
            ck.ToggleTess();
            return true;
        }
    };
    const mysvg = document.getElementById('mysvg');
    const paths=[], allPts=[];
    parsePaths(ck, mysvg, paths, allPts);

    const canvas = document.getElementById('mycanvas');
    ck.GetWebGLContext(canvas, {antialias: true});
    const sksurface = ck.MakeWebGLCanvasSurface(canvas, null);
    if (!sksurface) {
      throw 'Could not make canvas surface';
    }
    if (sksurface.sampleCnt() <= 1) {
      throw 'MSAA not supported';
    }

    const waves = new Waves(new Float32Array(allPts), ck);
    const fps = new FPSMeter();
    const drawFrame = (skcanvas) => {
        const wavyPts = waves.animate();
        for (const path of paths) {
            const pts = wavyPts.subarray(path.ptsIdx, path.ptsEndIdx);
            // FIXME: parse fill type from svg.
            const skPath = ck.SkPath.MakeFromVerbsPointsWeights(path.ckVerbs, pts, null,
                                                                ck.FillType.Winding);
            skcanvas.drawPath(skPath, path.skPaint);
            skPath.delete();
        }
        fps.markFrameComplete();
        sksurface.requestAnimationFrame(drawFrame);
    };
    sksurface.requestAnimationFrame(drawFrame);
}

function Waves(pts, ck) {
    const numWaves = 6;
    const fAmplitudes = new Float32Array(numWaves);
    const fFrequencies = new Float32Array(numWaves);
    const fDirsX = new Float32Array(numWaves);
    const fDirsY = new Float32Array(numWaves);
    const fSpeeds = new Float32Array(numWaves);
    const fOffsets = new Float32Array(numWaves);

    const kAverageAngle = 3*Math.PI / 8.0;
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
    const batchHeight = Math.ceil(pts.length/2 / framebufferWidth);
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

    mWavyPts = ck.Malloc(Float32Array, framebufferWidth * framebufferHeight * 4);
    mWavyPts.ste

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
        fTsec += numBatchesPerFramebuffer/60;
    }

    let readIdx=0, writeIdx=3;
    gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffers[readIdx]);
    let n = 0;
    this.animate = function() {
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
            fTsec += numBatchesPerFramebuffer/60;
        }
        n = (n + 1) % numBatchesPerFramebuffer;

        return mWavyPts.subarray(n * framebufferWidth * batchHeight * 4);
    }
}

function FPSMeter() {
    this.frames = 0;
    this.startMs = window.performance.now();
    this.label = document.getElementById('fpslabel');
    this.markFrameComplete = () => {
        ++this.frames;
        const ms = window.performance.now();
        const sec = (ms - this.startMs) / 1000;
        if (sec > 1) {
            this.label.innerHTML = Math.round(this.frames / sec) + ' fps';
            this.frames = 0;
            this.startMs = ms;
        }
        return ms;
    };
}
