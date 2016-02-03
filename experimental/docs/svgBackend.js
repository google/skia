var svgCache;
var svgDefs;
var svgGradients;
var svgNS = "http://www.w3.org/2000/svg";
var svgRoot;

function displaySvg(displayList) {
    for (var index = 0; index < displayList.length; ++index) {
        drawToSvg(displayList[index]);
    }
}

function drawToSvg(display) {
    assert('string' == typeof(display.ref));
    var cache;
    if (display.ref in svgCache) {
        cache = svgCache[display.ref];
        if (display.drawDirty) {
            switch (cache.spec) {
                case "paths":
                    svgSetPathData(cache.element, display.draw);
                    break;
                case "pictures":
                    svgSetPictureData(cache.element, display.draw);
                    break;
                case "text":
                    svgCreateText(cache.element, display.draw);
                    break;
                default:
                    assert(0);
            }
        }
    } else {
        cache = {};
        cache.action = display;
        cache.spec = display.drawSpec;
        var dot = cache.spec.indexOf(".");
        if (dot > 0) {
            cache.spec = cache.spec.substring(0, dot);
        }
        switch (cache.spec) {
            case "paths":
                cache.element = svgCreatePath(display.ref, display.draw);
                break;
            case "pictures":
                cache.element = svgCreatePicture(display.ref, display.draw);
                break;
            case "text":
                cache.element = svgCreateText(display.ref, display.draw);
                break;
            default:
                assert(0);
        }
    }
    display.drawDirty = false;
    if (display.paintDirty) {
        svgSetPaintData(cache.element, display.paint);
        var opacity = svg_opacity(display.paint.color);
        cache.element.setAttribute("fill-opacity", opacity);
        cache.element.setAttribute("stroke-opacity", opacity);
        display.paintDirty = false;
    }
    assert('object' == typeof(cache));
    if (!(display.ref in svgCache)) {
        svgRoot.appendChild(cache.element);
        svgCache[display.ref] = cache;
    }
}

function setupSvg() {
    svgCache = { "paths":{}, "pictures":{}, "text":{} };
    svgDefs = document.createElementNS(svgNS, "defs");
    svgGradients = {};
    svgRoot = document.getElementById("svg");
    while (svgRoot.lastChild) {
        svgRoot.removeChild(svgRoot.lastChild);
    }
    svgRoot.appendChild(svgDefs);
}

function svg_rbg(color) {
    return "rgb(" + ((color >> 16) & 0xFF)
            + "," + ((color >>  8) & 0xFF)
            + "," + ((color >>  0) & 0xFF) + ")";
}

function svg_opacity(color) {
    return ((color >> 24) & 0xFF) / 255.0;
}

function svgCreatePath(key, path) {
    var svgPath = document.createElementNS(svgNS, "path");
    svgPath.setAttribute("id", key);
    svgSetPathData(svgPath, path);
    return svgPath;
}

function svgCreatePicture(key, picture) {
    var svgPicture = document.createElementNS(svgNS, "g");
    svgPicture.setAttribute("id", key);
    svgSetPictureData(svgPicture, picture);
    return svgPicture;
}

function svgCreateRadialGradient(key) {
    var g = gradients[key];
    var e = document.createElementNS(svgNS, "radialGradient");
    e.setAttribute("id", key);
    e.setAttribute("cx", g.cx);
    e.setAttribute("cy", g.cy);
    e.setAttribute("r", g.r);
    e.setAttribute("gradientUnits", "userSpaceOnUse");
    var stopLen = g.stops.length;
    for (var index = 0; index < stopLen; ++index) {
        var stop = g.stops[index];
        var color = svg_rbg(stop.color);
        var s = document.createElementNS(svgNS, 'stop');
        s.setAttribute("offset", stop.offset);
        var style = "stop-color:" + svg_rbg(stop.color) + "; stop-opacity:"
                + svg_opacity(stop.color);
        s.setAttribute("style", style);
        e.appendChild(s);
    }
    svgGradients[key] = e;
    svgDefs.appendChild(e);
}

function svgCreateText(key, text) {
    var svgText = document.createElementNS(svgNS, "text");
    svgText.setAttribute("id", key);
    var textNode = document.createTextNode(text.string);
    svgText.appendChild(textNode);
    svgSetTextData(svgText, text);
    return svgText;
}

function svgSetPathData(svgPath, path) {
    var dString = "";
    for (var cIndex = 0; cIndex < path.length; ++cIndex) {
        var curveKey = Object.keys(path[cIndex])[0];
        var v = path[cIndex][curveKey];
        switch (curveKey) {
            case 'arcTo':
                var clockwise = 1; // to do; work in general case
                dString += " A" + v[4] + "," + v[4] + " 0 0," + clockwise + " "
                        + v[2] + "," + v[3];
                break;
            case 'close':
                dString += " z";
                break;
            case 'cubic':
                dString += " M" + v[0] + "," + v[1];
                dString += " C" + v[2] + "," + v[3]
                          + " " + v[4] + "," + v[5]
                          + " " + v[6] + "," + v[7];
                break;
            case 'line':
                dString += " M" + v[0] + "," + v[1];
                dString += " L" + v[2] + "," + v[3];
                break;
            case 'quad':
                dString += " M" + v[0] + "," + v[1];
                dString += " Q" + v[2] + "," + v[3]
                          + " " + v[4] + "," + v[5];
                break;
            default:
                assert(0);
        }
    }
    svgPath.setAttribute("d", dString);
}

function svgSetPaintData(svgElement, paint) {
    var color;
    var inPicture = 'string' == typeof(paint);
    if (inPicture) {
        paint = (new Function("return " + paint))();
        assert('object' == typeof(paint) && !isArray(paint));
    }
    if ('gradient' in paint) {
        var gradient = paint.gradient.split('.');
        var gradName = gradient[1];
        if (!svgGradients[gradName]) {
            svgCreateRadialGradient(gradName);
        }
        color = "url(#" + gradName + ")";
    } else {
        color = svg_rbg(paint.color);
    }
    svgElement.setAttribute("fill", 'fill' == paint.style ? color : "none");
    if ('stroke' == paint.style) {
        svgElement.setAttribute("stroke", color);
    }
    if ('strokeWidth' in paint) {
        svgElement.setAttribute("stroke-width", paint.strokeWidth);
    }
    if ('typeface' in paint) {
        var typeface = typefaces[paint.typeface];
        var font = typeface.style;
        if ('textSize' in paint) {
            svgElement.setAttribute("font-size", paint.textSize);
        }
        if ('family' in typeface) {
            svgElement.setAttribute("font-family", typeface.family);
        }
        if ('textAlign' in paint) {
            svgElement.setAttribute("text-anchor", paint.textAlign == "right" ? "end" : assert(0));
        }
        if ('textBaseline' in paint) {
            svgElement.setAttribute("alignment-baseline", paint.textBaseline);
        }
    }
}

function svgSetPictureData(svgPicture, picture) {
    while (svgPicture.lastChild) {
        svgPicture.removeChild(svgPicture.lastChild);
    }
    for (var index = 0; index < picture.length; ++index) {
        var entry = picture[index];
        var drawObj = (new Function("return " + entry.draw))();
        var drawSpec = entry.draw.split('.');
        var svgElement;
        switch (drawSpec[0]) {
            case 'paths':
                svgElement = svgCreatePath(drawSpec[1], drawObj);
                break;
            case 'pictures':
                svgElement = svgCreatePicture(drawSpec[1], drawObj);
                break;
            case 'text':
                svgElement = svgCreateText(drawSpec[1], drawObj);
                break;
            default:
                assert(0);
        }
        var paintObj = (new Function("return " + entry.paint))();
        svgSetPaintData(svgElement, paintObj);
        svgPicture.appendChild(svgElement);
    }
}

function svgSetTextData(svgElement, text) {
    svgElement.setAttribute('x', text.x);
    svgElement.setAttribute('y', text.y);
}
