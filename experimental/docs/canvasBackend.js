var canvas;
var ctx;
var canvasGradients = {};

function canvas_rbga(color) {
    var a = canvas_opacity(color);
    var r = (color >> 16) & 0xFF;
    var g = (color >>  8) & 0xFF;
    var b = (color >>  0) & 0xFF;
    return "rgba(" + r + "," + g + "," + b + "," + a + ")";
}

function canvas_opacity(color) {
    var a = (color >> 24) & 0xFF;
    return a / 255.;
}

function displayCanvas(displayList) {
    if (displayList.clear) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
    }
    for (var index = 0; index < displayList.length; ++index) {
        drawToCanvas(displayList[index]);
    }
}

function drawToCanvas(action) {
    ctx.save();
    var paint = paintToCanvas(action.paint);
    var draw = action.draw;
    if ('string' == typeof(draw)) {
        draw = (new Function("return " + draw))();
    }
    if (isArray(draw)) {
        assert(draw.length > 0);
        var picture = 'draw' in draw[0];
        if (picture) {
            for (var index = 0; index < draw.length; ++index) {
                drawToCanvas(draw[index]);
            }
            return;
        }
        ctx.beginPath();
        for (var index = 0; index < draw.length; ++index) {
            for (var prop in draw[index]) {
                var v = draw[index][prop];
                switch (prop) {
                    case 'arcTo':
                        ctx.arcTo(v[0], v[1], v[2], v[3], v[4]);
                        break;
                    case 'close':
                        ctx.closePath();
                        break;
                    case 'cubic':
                        ctx.moveTo(v[0], v[1]);
                        ctx.bezierCurveTo(v[2], v[3], v[4], v[5], v[6], v[7]);
                        break;
                    case 'line':
                        ctx.moveTo(v[0], v[1]);
                        ctx.lineTo(v[2], v[3]);
                        break;
                    case 'quad':
                        ctx.moveTo(v[0], v[1]);
                        ctx.quadraticCurveTo(v[2], v[3], v[4], v[5]);
                        break;
                    default:
                        assert(0);
                }
            }
        }
        if ('fill' == paint.style) {
            ctx.fill();
        } else {
            assert('stroke' == paint.style);
            ctx.stroke();
        }
    } else {
        assert('string' in draw);
        if ('fill' == paint.style) {
            ctx.fillText(draw.string, draw.x, draw.y);
        } else {
            assert('stroke' == paint.style);
            ctx.strokeText(draw.string, draw.x, draw.y);
        }
    }
    ctx.restore();
}

function keyframeCanvasInit(displayList, first) {
    if ('canvas' in first && 'clear' == first.canvas) {
        displayList.clear = true;
    }
}

function paintToCanvas(paint) {
    var color;
    var inPicture = 'string' == typeof(paint);
    if (inPicture) {
        paint = (new Function("return " + paint))();
        assert('object' == typeof(paint) && !isArray(paint));
    }
    if ('gradient' in paint) {
        var gradient = paint.gradient.split('.');
        var gradName = gradient[1];
        if (!canvasGradients[gradName]) {
            var g = window[gradient[0]][gradient[1]];
            var grad = ctx.createRadialGradient(g.cx, g.cy, 0, g.cx, g.cy, g.r);
            var stopLen = g.stops.length;
            for (var index = 0; index < stopLen; ++index) {
                var stop = g.stops[index];
                var color = canvas_rbga(stop.color);
                grad.addColorStop(index, color);
            }
            canvasGradients[gradName] = grad;
        }
        color = canvasGradients[gradName];
        if (!inPicture) {
            ctx.globalAlpha = canvas_opacity(paint.color);
        }
    } else {
        color = canvas_rbga(paint.color);
    }
    if ('fill' == paint.style) {
        ctx.fillStyle = color;
    } else if ('stroke' == paint.style) {
        ctx.strokeStyle = color;
    } else {
        ctx.globalAlpha = canvas_opacity(paint.color);
    }
    if ('strokeWidth' in paint) {
        ctx.lineWidth = paint.strokeWidth;
    }
    if ('typeface' in paint) {
        var typeface = typefaces[paint.typeface];
        var font = typeface.style;
        if ('textSize' in paint) {
            font += " " + paint.textSize;
        }
        if ('family' in typeface) {
            font += " " + typeface.family;
        }
        ctx.font = font;
        if ('textAlign' in paint) {
            ctx.textAlign = paint.textAlign;
        }
        if ('textBaseline' in paint) {
            ctx.textBaseline = paint.textBaseline;
        }
    }
    return paint;
}

function setupCanvas() {
    canvas = document.getElementById("canvas");
    ctx = canvas ? canvas.getContext("2d") : null;
    assert(ctx);
    var resScale = window.devicePixelRatio ? window.devicePixelRatio : 1;
    var unscaledWidth = canvas.width;
    var unscaledHeight = canvas.height;
    canvas.width = unscaledWidth * resScale;
    canvas.height = unscaledHeight * resScale;
    canvas.style.width = unscaledWidth + 'px';
    canvas.style.height = unscaledHeight + 'px';
    if (resScale != 1) {
        ctx.scale(resScale, resScale);
    }
}
