const numPtsInVerb = {'M':1, 'L':1, 'Q':2, 'C':3, 'Z':0};
const ckVerbIds = {'M':0, 'L':1, 'Q':2, 'C':4, 'Z':5};

function parseColorInt(name) {
    if (name == "white") {
        return 0xffffffff;
    }
    if (name.startsWith('#')) {
        return 0xff000000 + parseInt(name.substring(1), 16);
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
    const skpaint = new ck.SkPaint();
    skpaint.setAntiAlias(true);
    // skpaint.setShader(ck.SkShader.MakeLinearGradient(
    //         [x1, y1], [x2, y2], new Uint32Array(colors), offsets, ck.TileMode.Mirror, matrix, 1));
    skpaint.setColorInt(colors[colors.length>>1]);
    gradients[id] = skpaint;
}

function parsePaths(ck, node, paths, pts) {
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
                const verb = this.str[0];
                if (!(verb in numPtsInVerb)) {
                    throw "bad verb";
                }
                this.str = this.str.substring(1).trimStart();
                return verb;
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
        };
        const parser = new StringParser(node.getAttributeNS(null, 'd'));
        let verbs = "";
        let ckVerbs = [];
        let numPts = 0;
        while (parser.hasNext()) {
            const verb = parser.nextVerb();
            verbs += verb;
            ckVerbs.push(ckVerbIds[verb]);
            let n = numPtsInVerb[verb];
            for (let i = 0; i < n; ++i) {
                pts.push(parser.nextFloat());  // x
                pts.push(parser.nextFloat());  // y
            }
            numPts += n;
        }
        const fill = node.getAttributeNS(null, 'fill');
        let skpaint;
        if (fill.startsWith("url(")) {
            const id = fill.substring(5, fill.indexOf(")"));
            skpaint = gradients[id];
        } else {
            skpaint = new ck.SkPaint();
            skpaint.setAntiAlias(true);
            skpaint.setColorInt(parseColorInt(fill));
        }
        const mVerbs = ck.Malloc(Uint8Array, ckVerbs.length);
        mVerbs.toTypedArray().set(ckVerbs);
        paths.push({
            verbs:verbs, ckVerbs:mVerbs, numPts:numPts, paint:skpaint
        });
    }
    for (child of node.childNodes) {
        parsePaths(ck, child, paths, pts);
    }
}

const M = 'M'.charCodeAt(0);
const L = 'L'.charCodeAt(0);
const Q = 'Q'.charCodeAt(0);
const C = 'C'.charCodeAt(0);
const Z = 'Z'.charCodeAt(0);

function definePath(skpath,v,p,i) {
    const n = v.length;
    for (let j = 0; j < n; ++j) {
        switch (v.charCodeAt(j)) {
            case M:
                skpath.moveTo(p[i], p[i+1]);
                i += 2;
                break;
            case L:
                skpath.lineTo(p[i], p[i+1]);
                i += 2;
                break;
            case Q:
                skpath.quadTo(p[i], p[i+1], p[i+2], p[i+3]);
                i += 4;
                break;
            case C:
                skpath.cubicTo(p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5]);
                i += 6;
                break;
            case Z:
                skpath.close();
                break;
        }
    }
    return i;
}


function makePathFromCmds2(ck, verbs, pts) {
    let cmds = []
    let i=0;
    for (v of verbs) {
        switch (v) {
            case ck.MOVE_VERB:
                cmds.push([ck.MOVE_VERB, pts[i], pts[i+1]]);
                i += 2;
                break;
            case ck.LINE_VERB:
                cmds.push([ck.LINE_VERB, pts[i], pts[i+1]]);
                i += 2;
                break;
            case ck.QUAD_VERB:
                cmds.push([ck.QUAD_VERB, pts[i], pts[i+1], pts[i+2], pts[i+3]]);
                i += 4;
                break;
            case ck.CUBIC_VERB:
                cmds.push([ck.CUBIC_VERB, pts[i], pts[i+1], pts[i+2], pts[i+3], pts[i+4], pts[i+5]]);
                i += 6;
                break;
            case ck.CLOSE_VERB:
                cmds.push([ck.CLOSE_VERB]);
                break;
        }
    }
    return ck.MakePathFromCmds(cmds);
}

function launchDemo(ck) {
    const mysvg = document.getElementById('mysvg');
    const paths=[], pts=[];
    parsePaths(ck, mysvg, paths, pts);
    const pts32 = new Float32Array(pts);
    const mWavyPts = ck.Malloc(Float32Array, pts.length);
    mWavyPts.toTypedArray().set(pts);

    const canvas = document.getElementById('mycanvas');
    ck.GetWebGLContext(canvas, {antialias: true});
    const sksurface = ck.MakeWebGLCanvasSurface(canvas, null);
    if (!sksurface) {
      throw 'Could not make canvas surface';
    }
    if (sksurface.sampleCnt() <= 1) {
      throw 'MSAA not supported';
    }

    const waves = new Waves();
    const fps = new FPSMeter();
    const drawFrame = (skcanvas) => {
        waves.animate(pts32, mWavyPts.toTypedArray());
        let ptsIdx = 0;
        for (const path of paths) {
            // const skpath = new ck.SkPath();
            // definePath(skpath, path.verbs, wavyPts, ptsIdx);

            // const skpath = makePathFromCmds2(ck, path.ckVerbs,
            //                                  wavyPts.subarray(ptsIdx, ptsIdx + path.numPts*2));

            const skpath = ck.MakePathFromCmds2(path.ckVerbs,
              mWavyPts.subarray(ptsIdx, ptsIdx + path.numPts*2));

            ptsIdx += path.numPts*2;
            skcanvas.drawPath(skpath, path.paint);
            skpath.delete();
        }
        fps.markFrameComplete();
        sksurface.requestAnimationFrame(drawFrame);
    };
    sksurface.requestAnimationFrame(drawFrame);
}

function Waves() {
    const fAmplitudes = new Float32Array(4);
    const fFrequencies = new Float32Array(4);
    const fDirsX = new Float32Array(4);
    const fDirsY = new Float32Array(4);
    const fSpeeds = new Float32Array(4);
    const fOffsets = new Float32Array(4);

    const kAverageAngle = 3*Math.PI / 8.0;
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

    let fTsec = 0;

    this.animate = function(pts, out) {
        for (let i = 0; i < pts.length; i += 2) {
            let x=pts[i], y=pts[i+1];
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
            out[i+1] = y;
        }
        fTsec += 1/60;
        return out;
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
