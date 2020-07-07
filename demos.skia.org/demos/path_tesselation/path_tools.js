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
function* updatePaths(node, paths, pathIdx, pts) {
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
            yield null;
        }
        node.setAttributeNS(null, 'd', str)
    }
    for (child of node.childNodes) {
        let yieldedResult = null;
        let updatePathsGenerator = updatePaths(child, paths, pathIdx, pts);
        while (yieldedResult === null) {
            yieldedResult = updatePathsGenerator.next().value;
            yield null;
        }
        [pathIdx, pts] = yieldedResult;
    }
    yield [pathIdx, pts];
}
const M = 'M'.charCodeAt(0);
const L = 'L'.charCodeAt(0);
const Q = 'Q'.charCodeAt(0);
const C = 'C'.charCodeAt(0);
const Z = 'Z'.charCodeAt(0);
function* definePath(path2d,v,p,i) {
    const n = v.length;
    for (let j = 0; j < n; ++j) {
        switch (v.charCodeAt(j)) {
            case M:
                path2d.moveTo(p[i], p[i+1]);
                yield i += 2;
                break;
            case L:
                path2d.lineTo(p[i], p[i+1]);
                yield i += 2;
                break;
            case Q:
                path2d.quadraticCurveTo(p[i], p[i+1], p[i+2], p[i+3]);
                yield i += 4;
                break;
            case C:
                path2d.bezierCurveTo(p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5]);
                yield i += 6;
                break;
            case Z:
                yield path2d.closePath();
                break;
        }
    }
    yield i;
    return;
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

function parsePathsCanvasKit(ck, node, paths, allPts) {
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
        parsePathsCanvasKit(ck, child, paths, allPts);
    }
}