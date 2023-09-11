//
// This file houses functions that deal with color.
//

// Constructs a Color with the same API as CSS's rgba(), that is
// r,g,b are 0-255, and a is 0.0 to 1.0.
// if a is omitted, it will be assumed to be 1.0
// Internally, Colors are a TypedArray of four unpremultiplied 32-bit floats: a, r, g, b
// In order to construct one with more precision or in a wider gamut, use
// CanvasKit.Color4f
CanvasKit.Color = function(r, g, b, a) {
  if (a === undefined) {
      a = 1;
  }
  return CanvasKit.Color4f(clamp(r)/255, clamp(g)/255, clamp(b)/255, a);
};

// Constructs a Color as a 32 bit unsigned integer, with 8 bits assigned to each channel.
// Channels are expected to be between 0 and 255 and will be clamped as such.
CanvasKit.ColorAsInt = function(r, g, b, a) {
  // default to opaque
  if (a === undefined) {
      a = 255;
  }
  // This is consistent with how Skia represents colors in C++, as an unsigned int.
  // This is also consistent with how Flutter represents colors:
  // https://github.com/flutter/engine/blob/243bb59c7179a7e701ce478080d6ce990710ae73/lib/web_ui/lib/src/ui/painting.dart#L50
  return (((clamp(a) << 24) | (clamp(r) << 16) | (clamp(g) << 8) | (clamp(b) << 0)
   & 0xFFFFFFF) // This truncates the unsigned to 32 bits and signals to JS engines they can
                // represent the number with an int instead of a double.
    >>> 0);     // This makes the value an unsigned int.
};
// Construct a 4-float color.
// Opaque if opacity is omitted.
CanvasKit.Color4f = function(r, g, b, a) {
  if (a === undefined) {
    a = 1;
  }
  return Float32Array.of(r, g, b, a);
};

// Color constants use property getters to prevent other code from accidentally
// changing them.
Object.defineProperty(CanvasKit, 'TRANSPARENT', {
    get: function() { return CanvasKit.Color4f(0, 0, 0, 0); }
});
Object.defineProperty(CanvasKit, 'BLACK', {
    get: function() { return CanvasKit.Color4f(0, 0, 0, 1); }
});
Object.defineProperty(CanvasKit, 'WHITE', {
    get: function() { return CanvasKit.Color4f(1, 1, 1, 1); }
});
Object.defineProperty(CanvasKit, 'RED', {
    get: function() { return CanvasKit.Color4f(1, 0, 0, 1); }
});
Object.defineProperty(CanvasKit, 'GREEN', {
    get: function() { return CanvasKit.Color4f(0, 1, 0, 1); }
});
Object.defineProperty(CanvasKit, 'BLUE', {
    get: function() { return CanvasKit.Color4f(0, 0, 1, 1); }
});
Object.defineProperty(CanvasKit, 'YELLOW', {
    get: function() { return CanvasKit.Color4f(1, 1, 0, 1); }
});
Object.defineProperty(CanvasKit, 'CYAN', {
    get: function() { return CanvasKit.Color4f(0, 1, 1, 1); }
});
Object.defineProperty(CanvasKit, 'MAGENTA', {
    get: function() { return CanvasKit.Color4f(1, 0, 1, 1); }
});

// returns a css style [r, g, b, a] from a CanvasKit.Color
// where r, g, b are returned as ints in the range [0, 255]
// where a is scaled between 0 and 1.0
CanvasKit.getColorComponents = function(color) {
  return [
    Math.floor(color[0]*255),
    Math.floor(color[1]*255),
    Math.floor(color[2]*255),
    color[3]
  ];
};

// parseColorString takes in a CSS color value and returns a CanvasKit.Color
// (which is an array of 4 floats in RGBA order). An optional colorMap
// may be provided which maps custom strings to values.
// In the CanvasKit canvas2d shim layer, we provide this map for processing
// canvas2d calls, but not here for code size reasons.
CanvasKit.parseColorString = function(colorStr, colorMap) {
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
    // TODO(kjlubick)
  } else if (colorStr.startsWith('hsl')) {
    // TODO(kjlubick)
  } else if (colorMap) {
    // Try for named color
    var nc = colorMap[colorStr];
    if (nc !== undefined) {
      return nc;
    }
  }
  Debug('unrecognized color ' + colorStr);
  return CanvasKit.BLACK;
};

function isCanvasKitColor(ob) {
  if (!ob) {
    return false;
  }
  return (ob.constructor === Float32Array && ob.length === 4);
}

// Warning information is lost by this conversion
function toUint32Color(c) {
  return ((clamp(c[3]*255) << 24) | (clamp(c[0]*255) << 16) | (clamp(c[1]*255) << 8) | (clamp(c[2]*255) << 0)) >>> 0;
}
// Accepts various colors representations and converts them to an array of int colors.
// Does not handle builders.
function assureIntColors(arr) {
  if (wasMalloced(arr)) {
    return arr; // Assume if the memory was malloced that the user has done it correctly.
  } else if (arr instanceof Float32Array) {
    var count = Math.floor(arr.length / 4);
    var result = new Uint32Array(count);
    for (var i = 0; i < count; i ++) {
      result[i] = toUint32Color(arr.slice(i*4, (i+1)*4));
    }
    return result;
  } else if (arr instanceof Uint32Array) {
    return arr;
  } else if (arr instanceof Array && arr[0] instanceof Float32Array) {
    return arr.map(toUint32Color);
  }
}

function uIntColorToCanvasKitColor(c) {
    return CanvasKit.Color(
     (c >> 16) & 0xFF,
     (c >>  8) & 0xFF,
     (c >>  0) & 0xFF,
    ((c >> 24) & 0xFF) / 255
  );
}

function valueOrPercent(aStr) {
  if (aStr === undefined) {
    return 1; // default to opaque.
  }
  var a = parseFloat(aStr);
  if (aStr && aStr.indexOf('%') !== -1) {
    return a / 100;
  }
  return a;
}

function clamp(c) {
  return Math.round(Math.max(0, Math.min(c || 0, 255)));
}

// TODO(kjlubick) delete this, as it is now trivial with 4f colors
CanvasKit.multiplyByAlpha = function(color, alpha) {
  // make a copy of the color so the function remains pure.
  var result = color.slice();
  result[3] = Math.max(0, Math.min(result[3] * alpha, 1));
  return result;
};