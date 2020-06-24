const numPtsInVerb = {'M':1, 'L':1, 'Q':2, 'C':3, 'Z':0};

function parsePaths(node, paths, pts) {
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
        while (parser.hasNext()) {
            const verb = parser.nextVerb();
            verbs += verb;
            for (let i = 0; i < numPtsInVerb[verb]; ++i) {
                pts.push(parser.nextFloat());  // x
                pts.push(parser.nextFloat());  // y
            }
        }
        paths.push({v:verbs, f:node.getAttributeNS(null, 'fill')});
    }
    for (child of node.childNodes) {
        parsePaths(child, paths, pts);
    }
}

function updatePaths(node, paths, pathIdx, pts) {
    if (node.nodeName == "path") {
        const verbs = paths[pathIdx++].v;
        let str = "";
        for (v of verbs) {
            str += v;
            let n = numPtsInVerb[v] * 2;
            if (n > 0) {
                str += pts[0];
                for (let i = 1; i < n; ++i) {
                    str += ' ';
                    str += pts[i];
                }
            }
            pts = pts.subarray(n);
        }
        node.setAttributeNS(null, 'd', str)
    }
    for (child of node.childNodes) {
        [pathIdx, pts] = updatePaths(child, paths, pathIdx, pts);
    }
    return [pathIdx, pts];
}

const M = 'M'.charCodeAt(0);
const L = 'L'.charCodeAt(0);
const Q = 'Q'.charCodeAt(0);
const C = 'C'.charCodeAt(0);
const Z = 'Z'.charCodeAt(0);

function definePath(path2d,v,p,i) {
    const n = v.length;
    for (let j = 0; j < n; ++j) {
        switch (v.charCodeAt(j)) {
            case M:
                path2d.moveTo(p[i], p[i+1]);
                i += 2;
                break;
            case L:
                path2d.lineTo(p[i], p[i+1]);
                i += 2;
                break;
            case Q:
                path2d.quadraticCurveTo(p[i], p[i+1], p[i+2], p[i+3]);
                i += 4;
                break;
            case C:
                path2d.bezierCurveTo(p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5]);
                i += 6;
                break;
            case Z:
                path2d.closePath();
                break;
        }
    }
    return i;
}

function getPathStr(v,p,i) {
    let s = '';
    for (ch of v) {
        s += ch;
        const n = numPtsInVerb[ch]*2;
        if (n) {
            s += p[i];
            s += ',';
            s += p[i+1];
            for (let j = 2; j < n; j += 2) {
                s += ',';
                s += p[i+j];
                s += ',';
                s += p[i+j+1];
            }
            i += n;
        }
    }
    return [s,i];
}

window.onload = function() {
    const mysvg = document.getElementById('mysvg');
    const paths=[], pts=[];
    parsePaths(mysvg, paths, pts);
    const pts32 = new Float32Array(pts);

    const canvas = document.getElementById('mycanvas');
    const ctx = canvas.getContext("2d");
    ctx.fillStyle = "white";
    ctx.fillRect(0,0,10000,10000);

    const animator = new Animator(pts);
    const fps = new FPSMeter();
    window.requestAnimationFrame(function() {
        let ptsIdx = 0;
        let wavyPts = animator.animate();
        for (path of paths) {
            // let pathStr;
            // [pathStr, ptsIdx] = getPathStr(path.v, pts32, ptsIdx);
            // let path2d = new Path2D(pathStr);

            let path2d = new Path2D();
            ptsIdx = definePath(path2d, path.v, wavyPts, ptsIdx);

            ctx.fillStyle = path.f;
            ctx.fill(path2d);
        }
        fps.markFrameComplete();
        window.requestAnimationFrame(arguments.callee);
    });
}

function Animator(pts) {
    this.pts = new Float32Array(pts);
    this.animatedPts = new Float32Array(pts.length);
    let fTsec = 0;

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

    this.animate = function() {
        const pts = this.pts;
        const out = this.animatedPts;
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
        fTsec += 1;
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
            this.label.innerHTML = (this.frames / sec).toFixed(2) + ' fps';
            this.frames = 0;
            this.startMs = ms;
        }
        return ms;
    };
}
