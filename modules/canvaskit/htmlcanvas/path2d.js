// CanvasPath methods, which all take an SkPath object as the first param

function arc(skpath, x, y, radius, startAngle, endAngle, ccw) {
  // As per  https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-arc
  // arc is essentially a simpler version of ellipse.
  ellipse(skpath, x, y, radius, radius, 0, startAngle, endAngle, ccw);
}

function arcTo(skpath, x1, y1, x2, y2, radius) {
  if (!allAreFinite([x1, y1, x2, y2, radius])) {
    return;
  }
  if (radius < 0) {
    throw 'radii cannot be negative';
  }
  if (skpath.isEmpty()) {
    skpath.moveTo(x1, y1);
  }
  skpath.arcTo(x1, y1, x2, y2, radius);
}

function bezierCurveTo(skpath, cp1x, cp1y, cp2x, cp2y, x, y) {
  if (!allAreFinite([cp1x, cp1y, cp2x, cp2y, x, y])) {
    return;
  }
  if (skpath.isEmpty()) {
    skpath.moveTo(cp1x, cp1y);
  }
  skpath.cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
}

function closePath(skpath) {
  if (skpath.isEmpty()) {
    return;
  }
  // Check to see if we are not just a single point
  var bounds = skpath.getBounds();
  if ((bounds.fBottom - bounds.fTop) || (bounds.fRight - bounds.fLeft)) {
    skpath.close();
  }
}

function _ellipseHelper(skpath, x, y, radiusX, radiusY, startAngle, endAngle) {
  var sweepDegrees = radiansToDegrees(endAngle - startAngle);
  var startDegrees = radiansToDegrees(startAngle);

  var oval = CanvasKit.LTRBRect(x - radiusX, y - radiusY, x + radiusX, y + radiusY);

  // draw in 2 180 degree segments because trying to draw all 360 degrees at once
  // draws nothing.
  if (almostEqual(Math.abs(sweepDegrees), 360)) {
    var halfSweep = sweepDegrees/2;
    skpath.arcTo(oval, startDegrees, halfSweep, false);
    skpath.arcTo(oval, startDegrees + halfSweep, halfSweep, false);
    return;
  }
  skpath.arcTo(oval, startDegrees, sweepDegrees, false);
}

function ellipse(skpath, x, y, radiusX, radiusY, rotation,
                 startAngle, endAngle, ccw) {
  if (!allAreFinite([x, y, radiusX, radiusY, rotation, startAngle, endAngle])) {
    return;
  }
  if (radiusX < 0 || radiusY < 0) {
    throw 'radii cannot be negative';
  }

  // based off of CanonicalizeAngle in Chrome
  var tao = 2 * Math.PI;
  var newStartAngle = startAngle % tao;
  if (newStartAngle < 0) {
    newStartAngle += tao;
  }
  var delta = newStartAngle - startAngle;
  startAngle = newStartAngle;
  endAngle += delta;

  // Based off of AdjustEndAngle in Chrome.
  if (!ccw && (endAngle - startAngle) >= tao) {
    // Draw complete ellipse
    endAngle = startAngle + tao;
  } else if (ccw && (startAngle - endAngle) >= tao) {
    // Draw complete ellipse
    endAngle = startAngle - tao;
  } else if (!ccw && startAngle > endAngle) {
    endAngle = startAngle + (tao - (startAngle - endAngle) % tao);
  } else if (ccw && startAngle < endAngle) {
    endAngle = startAngle - (tao - (endAngle - startAngle) % tao);
  }

  // Based off of Chrome's implementation in
  // https://cs.chromium.org/chromium/src/third_party/blink/renderer/platform/graphics/path.cc
  // of note, can't use addArc or addOval because they close the arc, which
  // the spec says not to do (unless the user explicitly calls closePath).
  // This throws off points being in/out of the arc.
  if (!rotation) {
    _ellipseHelper(skpath, x, y, radiusX, radiusY, startAngle, endAngle);
    return;
  }
  var rotated = CanvasKit.SkMatrix.rotated(rotation, x, y);
  var rotatedInvert = CanvasKit.SkMatrix.rotated(-rotation, x, y);
  skpath.transform(rotatedInvert);
  _ellipseHelper(skpath, x, y, radiusX, radiusY, startAngle, endAngle);
  skpath.transform(rotated);
}

function lineTo(skpath, x, y) {
  if (!allAreFinite([x, y])) {
    return;
  }
  // A lineTo without a previous point has a moveTo inserted before it
  if (skpath.isEmpty()) {
    skpath.moveTo(x, y);
  }
  skpath.lineTo(x, y);
}

function moveTo(skpath, x, y) {
  if (!allAreFinite([x, y])) {
    return;
  }
  skpath.moveTo(x, y);
}

function quadraticCurveTo(skpath, cpx, cpy, x, y) {
  if (!allAreFinite([cpx, cpy, x, y])) {
    return;
  }
  if (skpath.isEmpty()) {
    skpath.moveTo(cpx, cpy);
  }
  skpath.quadTo(cpx, cpy, x, y);
}

function rect(skpath, x, y, width, height) {
  if (!allAreFinite([x, y, width, height])) {
    return;
  }
  // https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-rect
  skpath.addRect(x, y, x+width, y+height);
}

function Path2D(path) {
  this._path = null;
  if (typeof path === 'string') {
      this._path = CanvasKit.MakePathFromSVGString(path);
  } else if (path && path._getPath) {
      this._path = path._getPath().copy();
  } else {
    this._path = new CanvasKit.SkPath();
  }

  this._getPath = function() {
      return this._path;
  }

  this.addPath = function(path2d, transform) {
    if (!transform) {
      transform = {
        'a': 1, 'c': 0, 'e': 0,
        'b': 0, 'd': 1, 'f': 0,
      };
    }
    this._path.addPath(path2d._getPath(), [transform.a, transform.c, transform.e,
                                           transform.b, transform.d, transform.f]);
  }

  this.arc = function(x, y, radius, startAngle, endAngle, ccw) {
    arc(this._path, x, y, radius, startAngle, endAngle, ccw);
  }

  this.arcTo = function(x1, y1, x2, y2, radius) {
    arcTo(this._path, x1, y1, x2, y2, radius);
  }

  this.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
    bezierCurveTo(this._path, cp1x, cp1y, cp2x, cp2y, x, y);
  }

  this.closePath = function() {
    closePath(this._path);
  }

  this.ellipse = function(x, y, radiusX, radiusY, rotation,
                          startAngle, endAngle, ccw) {
    ellipse(this._path, x, y, radiusX, radiusY, rotation,
            startAngle, endAngle, ccw);
  }

  this.lineTo = function(x, y) {
    lineTo(this._path, x, y);
  }

  this.moveTo = function(x, y) {
    moveTo(this._path, x, y);
  }

  this.quadraticCurveTo = function(cpx, cpy, x, y) {
    quadraticCurveTo(this._path, cpx, cpy, x, y);
  }

  this.rect = function(x, y, width, height) {
    rect(this._path, x, y, width, height);
  }
}
