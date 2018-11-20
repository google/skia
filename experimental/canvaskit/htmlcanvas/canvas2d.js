// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, the code that emulates the HTML Canvas interface
// (which may be called HTMLCanvas or similar to avoid confusion with
// SkCanvas).
(function(CanvasKit) {

  var isNode = typeof btoa === undefined;

  function argsAreFinite(args) {
    for (var i = 0; i < args.length; i++) {
      if (!Number.isFinite(args[0])) {
        return false;
      }
    }
    return true;
  }

  CanvasKit._testing = {};

  function HTMLCanvas(skSurface) {
    this._surface = skSurface;
    this._context = new CanvasRenderingContext2D(skSurface.getCanvas());

    // A normal <canvas> requires that clients call getContext
    this.getContext = function(type) {
      if (type === '2d') {
        return this._context;
      }
      return null;
    }

    this.toDataURL = function(codec) {
      // TODO(kjlubick): maybe support other codecs?
      // For now, just to png
      this._surface.flush();

      var img = this._surface.makeImageSnapshot();
      if (!img) {
        console.error('no snapshot');
        return;
      }
      var png = img.encodeToData();
      if (!png) {
        console.error('encoding failure');
        return
      }
      // TODO(kjlubick): clean this up a bit - maybe better naming?
      var pngBytes = CanvasKit.getSkDataBytes(png);
      if (isNode) {
        // See https://stackoverflow.com/a/12713326
        var b64encoded = Buffer.from(pngBytes).toString('base64');
      } else {
        var b64encoded = btoa(String.fromCharCode.apply(null, pngBytes));
      }
      return 'data:image/png;base64,' + b64encoded;
    }

    this.dispose = function() {
      this._context._dispose();
      this._surface.dispose();
    }
  }

  function CanvasRenderingContext2D(skcanvas) {
    this._canvas = skcanvas;
    this._paint = new CanvasKit.SkPaint();
    this._paint.setAntiAlias(true);
    this._paint.setStrokeWidth(2);
    this._currentSubPath = null;
    this._paths = [];
    this._pathStarted = false;

    this._dispose = function() {
      this._paths.forEach(function(path) {
        path.delete();
      });
      this._paint.delete();
      // Don't delete this._canvas as it will be disposed
      // by the surface of which it is based.
    }

    Object.defineProperty(this, 'font', {
      enumerable: true,
      set: function(newStyle) {
        var size = parseFontSize(newStyle);
        // TODO styles
        this._paint.setTextSize(size);
      }
    });

    Object.defineProperty(this, 'lineWidth', {
      enumerable: true,
      set: function(newWidth) {
        if (newWidth <= 0 || !newWidth) {
          // Spec says to ignore NaN/Inf/0/negative values
          return;
        }
        this._paint.setStrokeWidth(newWidth);
      }
    });

    Object.defineProperty(this, 'strokeStyle', {
      enumerable: true,
      set: function(newStyle) {
        this._paint.setColor(parseColor(newStyle));
      }
    });

    this.arc = function(x, y, radius, startAngle, endAngle, ccw) {
      // As per  https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-arc
      // arc is essentially a simpler version of ellipse.
      this.ellipse(x, y, radius, radius, 0, startAngle, endAngle, ccw);
    }

    this.arcTo = function(x1, y1, x2, y2, radius) {
      if (!argsAreFinite(arguments)) {
        return;
      }
      this._ensureSubpath(x1, y1);
      if (radius < 0) {
        throw 'radii cannot be negative';
      }
      this._currentSubPath.arcTo(x1, y1, x2, y2, radius);
    }

    this.beginPath = function() {
      this._currentSubPath = new CanvasKit.SkPath();
      this._paths.push(this._currentSubPath);
      this._pathStarted = false;
    }

    this.bezierCurveTo = function(cp1x, cp1y, cp2x, cp2y, x, y) {
      if (!argsAreFinite(arguments)) {
        return;
      }
      this._ensureSubpath(cp1x, cp1y);
      this._currentSubPath.cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
    }

    this.closePath = function() {
      if (this._currentSubPath) {
        this._currentSubPath.close();
        this._currentSubPath = null;
        this._pathStarted = false;
      }
    }

    this.ellipse = function(x, y, radiusX, radiusY, rotation,
                            startAngle, endAngle, ccw) {
      if (!argsAreFinite(arguments)) {
        return;
      }
      if (radiusX < 0 || radiusY < 0) {
        throw 'radii cannot be negative';
      }
      this._pathStarted = true;

      var bounds = CanvasKit.LTRBRect(x-radiusX, y-radiusY, x+radiusX, y+radiusY);
      var sweep = radiansToDegrees(endAngle - startAngle) - (360 * !!ccw);
      var temp = new CanvasKit.SkPath();
      temp.addArc(bounds, radiansToDegrees(startAngle), sweep);
      var m = CanvasKit.SkMatrix.rotated(radiansToDegrees(rotation), x, y);
      this._currentSubPath.addPath(temp, m, true);
      temp.delete();
    }

    // ensureSubpath makes SkPath behave like the browser's path object
    // in that the first lineTo really acts like a moveTo.
    // ensureSubpath is the term used in the canvas spec:
    // https://html.spec.whatwg.org/multipage/canvas.html#ensure-there-is-a-subpath
    // ensureSubpath returns true if the drawing command can proceed,
    // false otherwise (i.e. it was the first command and may be replaced
    // with a moveTo).
    this._ensureSubpath = function(x, y) {
      if (!this._currentSubPath) {
        this.beginPath();
      }
      if (!this._pathStarted) {
        this._pathStarted = true;
        this.moveTo(x, y);
        return false;
      }
      return true;
    }

    this.fillText = function(text, x, y, maxWidth) {
      // TODO do something with maxWidth, probably involving measure
      this._canvas.drawText(text, x, y, this._paint);
    }

    this.lineTo = function(x, y) {
      if (!argsAreFinite(arguments)) {
        return;
      }
      // lineTo is the odd-ball in the sense that a line-to without
      // a previous subpath is the same as a moveTo.
      if (this._ensureSubpath(x, y)) {
        this._currentSubPath.lineTo(x, y);
      }
    }

    this.measureText = function(text) {
      return {
        width: this._paint.measureText(text),
        // TODO other measurements?
      }
    }

    this.moveTo = function(x, y) {
      if (!argsAreFinite(arguments)) {
        return;
      }
      if (this._ensureSubpath(x, y)) {
        this._currentSubPath.moveTo(x, y);
      }
    }

    this.quadraticCurveTo = function(cpx, cpy, x, y) {
      if (!argsAreFinite(arguments)) {
        return;
      }
      this._ensureSubpath(cpx, cpy);
      this._currentSubPath.quadTo(cpx, cpy, x, y);
    }

    this.rect = function(x, y, width, height) {
      if (!argsAreFinite(arguments)) {
        return;
      }
      // https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-rect
      this.beginPath();
      this._currentSubPath.addRect(x, y, x+width, y+height);
      this.beginPath();
      this._currentSubPath.moveTo(x, y);
    }

    this.resetTransform = function() {
      this.setTransform(1, 0, 0, 1, 0, 0);
    }

    this.rotate = function(radians, px, py) {
      // bindings can't turn undefined into floats
      this._canvas.rotate(radians * 180/Math.PI, px || 0, py || 0);
    }

    this.scale = function(sx, sy) {
      this._canvas.scale(sx, sy);
    }

    this.scale = function(sx, sy) {
      this._canvas.scale(sx, sy);
    }

    this.setTransform = function(a, b, c, d, e, f) {
      this._canvas.setMatrix([a, c, e,
                              b, d, f,
                              0, 0, 1]);
    }

    this.skew = function(sx, sy) {
      this._canvas.skew(sx, sy);
    }

    this.stroke = function() {
      if (this._currentSubPath) {
        this._paint.setStyle(CanvasKit.PaintStyle.Stroke);
        this._paint.setLine
        for (var i = 0; i < this._paths.length; i++) {
          this._canvas.drawPath(this._paths[i], this._paint);
        }
      }
    }

    this.strokeText = function(text, x, y, maxWidth) {
      // TODO do something with maxWidth, probably involving measure
      this._paint.setStyle(CanvasKit.PaintStyle.Stroke);
      this._canvas.drawText(text, x, y, this._paint);
    }

    this.translate = function(dx, dy) {
      this._canvas.translate(dx, dy);
    }


    // Not supported operations (e.g. for Web only)
    this.addHitRegion = function() {};
    this.clearHitRegions = function() {};
    this.drawFocusIfNeeded = function() {};
    this.removeHitRegion = function() {};
    this.scrollPathIntoView = function() {};

    Object.defineProperty(this, 'canvas', {
      value: null,
      writable: false
    });
  }

  CanvasKit.MakeCanvas = function(width, height) {
    // TODO(kjlubick) do fonts the "correct" way
    CanvasKit.initFonts();
    var surf = CanvasKit.MakeSurface(width, height);
    if (surf) {
      return new HTMLCanvas(surf);
    }
    return null;
  }

  var units = 'px|pt|pc|in|cm|mm|%|em|ex|ch|rem|q';
  var fontSizeRegex = new RegExp('([\\d\\.]+)(' + units + ')');
  var defaultHeight = 12;
  // Based off of node-canvas's parseFont
  // returns font size in *points* (original impl was in px);
  function parseFontSize(fontStr) {
    // This is naive and doesn't account for line-height yet
    // (but neither does node-canvas's?)
    var fontSize = fontSizeRegex.exec(fontStr);
    if (!fontSize) {
      console.error('Could not parse font size', fontStr);
      return 16;
    }
    var size = fontSize[1];
    var unit = fontSize[2];
    switch (unit) {
      case 'pt':
        return size;
      case 'px':
        return size * 3/4;
      case 'pc':
        return size * 12;
      case 'in':
        return size * 72;
      case 'cm':
        return size * 72.0 / 2.54;
      case 'mm':
        return size * (72.0 / 25.4);
      case '%':
        return size * (defaultHeight / 100);
      case 'em':
      case 'rem':
        return size * defaultHeight;
      case 'q':
        return size * (96 / 25.4 / 3);
    }
  }

  function valueOrPercent(aStr) {
    var a = parseFloat(aStr) || 1;
    if (aStr && aStr.indexOf('%') !== -1) {
      return a / 100;
    }
    return a;
  }

  function parseColor(colorStr) {
    colorStr = colorStr.toLowerCase();
    // See https://drafts.csswg.org/css-color/#typedef-hex-color
    if (colorStr.startsWith('#')) {
      var r, g, b, a = 255;
      switch (colorStr.length) {
        case 9: // 8 hex chars #RRGGBBAA
          a = parseInt(colorStr.slice(7, 9), 16);
        case 7: // 6 hex chars #RRGGBB
          r = parseInt(colorStr.slice(1, 3), 16);
          g = parseInt(colorStr.slice(3, 5), 16);
          b = parseInt(colorStr.slice(5, 7), 16);
          break;
        case 5: // 4 hex chars #RGBA
          // multiplying by 17 is the same effect as
          // appending another character of the same value
          // e.g. e => ee == 14 => 238
          a = parseInt(colorStr.slice(4, 5), 16) * 17;
        case 4: // 6 hex chars #RGB
          r = parseInt(colorStr.slice(1, 2), 16) * 17;
          g = parseInt(colorStr.slice(2, 3), 16) * 17;
          b = parseInt(colorStr.slice(3, 4), 16) * 17;
          break;
      }
      return CanvasKit.Color(r, g, b, a/255);

    } else if (colorStr.startsWith('rgba')) {
      // Trim off rgba( and the closing )
      colorStr = colorStr.slice(5, -1);
      var nums = colorStr.split(',');
      return CanvasKit.Color(+nums[0], +nums[1], +nums[2],
                             valueOrPercent(nums[3]));
    } else if (colorStr.startsWith('rgb')) {
      // Trim off rgba( and the closing )
      colorStr = colorStr.slice(4, -1);
      var nums = colorStr.split(',');
      // rgb can take 3 or 4 arguments
      return CanvasKit.Color(+nums[0], +nums[1], +nums[2],
                             valueOrPercent(nums[3]));
    } else if (colorStr.startsWith('gray(')) {
      // TODO
    } else if (colorStr.startsWith('hsl')) {
      // TODO
    } else {
      // Try for named color
      var nc = colorMap[colorStr];
      if (nc !== undefined) {
        return nc;
      }
    }
    console.error('unrecognized color ', colorStr);
    return CanvasKit.BLACK;
  }

  CanvasKit._testing['parseColor'] = parseColor;

  // Create the following with
  // node ./htmlcanvas/_namedcolors.js --expose-wasm
  // JS/closure doesn't have a constexpr like thing which
  // would really help here. Since we don't, we pre-compute
  // the map, which saves (a tiny amount of) startup time
  // and (a small amount of) code size.
  var colorMap = {"aliceblue":-984833,"antiquewhite":-332841,"aqua":-16711681,"aquamarine":-8388652,"azure":-983041,"beige":-657956,"bisque":-6972,"black":-16777216,"blanchedalmond":-5171,"blue":-16776961,"blueviolet":-7722014,"brown":-5952982,"burlywood":-2180985,"cadetblue":-10510688,"chartreuse":-8388864,"chocolate":-2987746,"coral":-32944,"cornflowerblue":-10185235,"cornsilk":-1828,"crimson":-2354116,"cyan":-16711681,"darkblue":-16777077,"darkcyan":-16741493,"darkgoldenrod":-4684277,"darkgray":-5658199,"darkgreen":-16751616,"darkgrey":-5658199,"darkkhaki":-4343957,"darkmagenta":-7667573,"darkolivegreen":-11179217,"darkorange":-29696,"darkorchid":-6737204,"darkred":-7667712,"darksalmon":-1468806,"darkseagreen":-7357297,"darkslateblue":-12042869,"darkslategray":-13676721,"darkslategrey":-13676721,"darkturquoise":-16724271,"darkviolet":-7077677,"deeppink":-60269,"deepskyblue":-16728065,"dimgray":-9868951,"dimgrey":-9868951,"dodgerblue":-14774017,"firebrick":-5103070,"floralwhite":-1296,"forestgreen":-14513374,"fuchsia":-65281,"gainsboro":-2302756,"ghostwhite":-460545,"gold":-10496,"goldenrod":-2448096,"gray":-8355712,"green":-16744448,"greenyellow":-5374161,"grey":-8355712,"honeydew":-983056,"hotpink":-38476,"indianred":-3318692,"indigo":-11861886,"ivory":-16,"khaki":-989556,"lavender":-1644806,"lavenderblush":-3851,"lawngreen":-8586240,"lemonchiffon":-1331,"lightblue":-5383962,"lightcoral":-1015680,"lightcyan":-2031617,"lightgoldenrodyellow":-329006,"lightgray":-2894893,"lightgreen":-7278960,"lightgrey":-2894893,"lightpink":-18751,"lightsalmon":-24454,"lightseagreen":-14634326,"lightskyblue":-7876870,"lightslategray":-8943463,"lightslategrey":-8943463,"lightsteelblue":-5192482,"lightyellow":-32,"lime":-16711936,"limegreen":-13447886,"linen":-331546,"magenta":-65281,"maroon":-8388608,"mediumaquamarine":-10039894,"mediumblue":-16777011,"mediumorchid":-4565549,"mediumpurple":-7114533,"mediumseagreen":-12799119,"mediumslateblue":-8689426,"mediumspringgreen":-16713062,"mediumturquoise":-12004916,"mediumvioletred":-3730043,"midnightblue":-15132304,"mintcream":-655366,"mistyrose":-6943,"moccasin":-6987,"navajowhite":-8531,"navy":-16777088,"oldlace":-133658,"olive":-8355840,"olivedrab":-9728477,"orange":-23296,"orangered":-47872,"orchid":-2461482,"palegoldenrod":-1120086,"palegreen":-6751336,"paleturquoise":-5247250,"palevioletred":-2396013,"papayawhip":-4139,"peachpuff":-9543,"peru":-3308225,"pink":-16181,"plum":-2252579,"powderblue":-5185306,"purple":-8388480,"rebeccapurple":-10079335,"red":-65536,"rosybrown":-4419697,"royalblue":-12490271,"saddlebrown":-7650029,"salmon":-360334,"sandybrown":-744352,"seagreen":-13726889,"seashell":-2578,"sienna":-6270419,"silver":-4144960,"skyblue":-7876885,"slateblue":-9807155,"slategray":-9404272,"slategrey":-9404272,"snow":-1286,"springgreen":-16711809,"steelblue":-12156236,"tan":-2968436,"teal":-16744320,"thistle":-2572328,"transparent":0,"tomato":-40121,"turquoise":-12525360,"violet":-1146130,"wheat":-663885,"white":-1,"whitesmoke":-657931,"yellow":-256,"yellowgreen":-6632142};

}(Module)); // When this file is loaded in, the high level object is "Module";
