// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, the code that emulates the HTML Canvas interface
// (which may be called HTMLCanvas or similar to avoid confusion with
// SkCanvas).
(function(CanvasKit) {

  var isNode = typeof btoa === undefined;

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

    this.toDataURL = function() {
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
  }

  function CanvasRenderingContext2D(skcanvas) {
    this._canvas = skcanvas;
    this._paint = new CanvasKit.SkPaint();
    this._paint.setAntiAlias(true);
    this._currentPath = null;
    this._pathStarted = false;

    Object.defineProperty(this, 'font', {
      enumerable: true,
      set: function(newStyle) {
        var size = parseFontSize(newStyle);
        // TODO styles
        this._paint.setTextSize(size);
      }
    });

    Object.defineProperty(this, 'strokeStyle', {
      enumerable: true,
      set: function(newStyle) {
        this._paint.setColor(parseColor(newStyle));
      }
    });

    this.beginPath = function() {
      if (this._currentPath) {
        this._currentPath.delete();
      }
      this._currentPath = new CanvasKit.SkPath();
      this._pathStarted = false;
    }

    // ensureSubpath makes SkPath behave like the browser's path object
    // in that the first lineTo/cubicTo, etc, really acts like a moveTo.
    // ensureSubpath is the term used in the canvas spec:
    // https://html.spec.whatwg.org/multipage/canvas.html#ensure-there-is-a-subpath
    // ensureSubpath returns true if the drawing command can proceed,
    // false otherwise (i.e. it was the first command and replaced
    // with a moveTo).
    this._ensureSubpath = function(x, y) {
      if (!this._currentPath) {
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
      if (this._ensureSubpath(x, y)) {
        this._currentPath.lineTo(x, y);
      }
    }

    this.measureText = function(text) {
      return {
        width: this._paint.measureText(text),
        // TODO other measurements?
      }
    }

    this.moveTo = function(x, y) {
      if (this._ensureSubpath(x, y)) {
        this._currentPath.moveTo(x, y);
      }
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

    this.setTransform = function(a, b, c, d, e, f) {
      this._canvas.setMatrix([a, c, e,
                              b, d, f,
                              0, 0, 1]);
    }

    this.skew = function(sx, sy) {
      this._canvas.skew(sx, sy);
    }

    this.stroke = function() {
      if (this._currentPath) {
        this._paint.setStyle(CanvasKit.PaintStyle.Stroke);
        this._canvas.drawPath(this._currentPath, this._paint);
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
  }

  CanvasKit.MakeCanvas = function(width, height) {
    // TODO do fonts the "correct" way
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
      console.err('Could not parse font size', fontStr);
      return 16;
    }
    var size = fontSize[1];
    var unit = fontSize[2];
    switch (unit) {
      // TODO are these correctly in points?
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
      var nc = colorMap(colorStr);
      if (nc !== undefined) {
        return nc;
      }
    }
    console.err('unrecognized color ', colorStr);
    return CanvasKit.BLACK;
  }

  CanvasKit._testing['parseColor'] = parseColor;

  // Create the following with
  // node ./htmlcanvas/_namedcolors.js --expose-wasm
  // JS/closure doesn't have a constexpr like thing which
  // would really help here. Since we don't, we pre-compute
  // the map, which saves (a tiny amount of) startup time
  // and (a small amount of) code size.
  var colorMap = {
    aliceblue: 15792383,
    antiquewhite: 16444375,
    aqua: 65535,
    aquamarine: 8388564,
    azure: 15794175,
    beige: 16119260,
    bisque: 16770244,
    black: 0,
    blanchedalmond: 16772045,
    blue: 255,
    blueviolet: 9055202,
    brown: 10824234,
    burlywood: 14596231,
    cadetblue: 6266528,
    chartreuse: 8388352,
    chocolate: 13789470,
    coral: 16744272,
    cornflowerblue: 6591981,
    cornsilk: 16775388,
    crimson: 14423100,
    cyan: 65535,
    darkblue: 139,
    darkcyan: 35723,
    darkgoldenrod: 12092939,
    darkgray: 11119017,
    darkgreen: 25600,
    darkgrey: 11119017,
    darkkhaki: 12433259,
    darkmagenta: 9109643,
    darkolivegreen: 5597999,
    darkorange: 16747520,
    darkorchid: 10040012,
    darkred: 9109504,
    darksalmon: 15308410,
    darkseagreen: 9419919,
    darkslateblue: 4734347,
    darkslategray: 3100495,
    darkslategrey: 3100495,
    darkturquoise: 52945,
    darkviolet: 9699539,
    deeppink: 16716947,
    deepskyblue: 49151,
    dimgray: 6908265,
    dimgrey: 6908265,
    dodgerblue: 2003199,
    firebrick: 11674146,
    floralwhite: 16775920,
    forestgreen: 2263842,
    fuchsia: 16711935,
    gainsboro: 14474460,
    ghostwhite: 16316671,
    gold: 16766720,
    goldenrod: 14329120,
    gray: 8421504,
    green: 32768,
    greenyellow: 11403055,
    grey: 8421504,
    honeydew: 15794160,
    hotpink: 16738740,
    indianred: 13458524,
    indigo: 4915330,
    ivory: 16777200,
    khaki: 15787660,
    lavender: 15132410,
    lavenderblush: 16773365,
    lawngreen: 8190976,
    lemonchiffon: 16775885,
    lightblue: 11393254,
    lightcoral: 15761536,
    lightcyan: 14745599,
    lightgoldenrodyellow: 16448210,
    lightgray: 13882323,
    lightgreen: 9498256,
    lightgrey: 13882323,
    lightpink: 16758465,
    lightsalmon: 16752762,
    lightseagreen: 2142890,
    lightskyblue: 8900346,
    lightslategray: 7833753,
    lightslategrey: 7833753,
    lightsteelblue: 11584734,
    lightyellow: 16777184,
    lime: 65280,
    limegreen: 3329330,
    linen: 16445670,
    magenta: 16711935,
    maroon: 8388608,
    mediumaquamarine: 6737322,
    mediumblue: 205,
    mediumorchid: 12211667,
    mediumpurple: 9662683,
    mediumseagreen: 3978097,
    mediumslateblue: 8087790,
    mediumspringgreen: 64154,
    mediumturquoise: 4772300,
    mediumvioletred: 13047173,
    midnightblue: 1644912,
    mintcream: 16121850,
    mistyrose: 16770273,
    moccasin: 16770229,
    navajowhite: 16768685,
    navy: 128,
    oldlace: 16643558,
    olive: 8421376,
    olivedrab: 7048739,
    orange: 16753920,
    orangered: 16729344,
    orchid: 14315734,
    palegoldenrod: 15657130,
    palegreen: 10025880,
    paleturquoise: 11529966,
    palevioletred: 14381203,
    papayawhip: 16773077,
    peachpuff: 16767673,
    peru: 13468991,
    pink: 16761035,
    plum: 14524637,
    powderblue: 11591910,
    purple: 8388736,
    rebeccapurple: 6697881,
    red: 16711680,
    rosybrown: 12357519,
    royalblue: 4286945,
    saddlebrown: 9127187,
    salmon: 16416882,
    sandybrown: 16032864,
    seagreen: 3050327,
    seashell: 16774638,
    sienna: 10506797,
    silver: 12632256,
    skyblue: 8900331,
    slateblue: 6970061,
    slategray: 7372944,
    slategrey: 7372944,
    snow: 16775930,
    springgreen: 65407,
    steelblue: 4620980,
    tan: 13808780,
    teal: 32896,
    thistle: 14204888,
    tomato: 16737095,
    turquoise: 4251856,
    violet: 15631086,
    wheat: 16113331,
    white: 16777215,
    whitesmoke: 16119285,
    yellow: 16776960,
    yellowgreen: 10145074
  };

}(Module)); // When this file is loaded in, the high level object is "Module";
