// Functions dealing with parsing/stringifying fonts go here.
var fontStringRegex = new RegExp(
  '(italic|oblique|normal|)\\s*' +              // style
  '(small-caps|normal|)\\s*' +                  // variant
  '(bold|bolder|lighter|[1-9]00|normal|)\\s*' + // weight
  '([\\d\\.]+)' +                               // size
  '(px|pt|pc|in|cm|mm|%|em|ex|ch|rem|q)' +      // unit
  // line-height is ignored here, as per the spec
  '(.+)'                                        // family
  );

function stripWhitespace(str) {
  return str.replace(/^\s+|\s+$/, '');
}

var defaultHeight = 16;
// Based off of node-canvas's parseFont
// returns font size in px, which represents the em width.
function parseFontString(fontStr) {

  var font = fontStringRegex.exec(fontStr);
  if (!font) {
    Debug('Invalid font string ' + fontStr);
    return null;
  }

  var size = parseFloat(font[4]);
  var sizePx = defaultHeight;
  var unit = font[5];
  switch (unit) {
    case 'em':
    case 'rem':
      sizePx = size * defaultHeight;
      break;
    case 'pt':
      sizePx = size * 4/3;
      break;
    case 'px':
      sizePx = size;
      break;
    case 'pc':
      sizePx = size * defaultHeight;
      break;
    case 'in':
      sizePx = size * 96;
      break;
    case 'cm':
      sizePx = size * 96.0 / 2.54;
      break;
    case 'mm':
      sizePx = size * (96.0 / 25.4);
      break;
    case 'q': // quarter millimeters
      sizePx = size * (96.0 / 25.4 / 4);
      break;
    case '%':
      sizePx = size * (defaultHeight / 75);
      break;
  }
  return {
    'style':   font[1],
    'variant': font[2],
    'weight':  font[3],
    'sizePx':  sizePx,
    'family':  font[6].trim()
  };
}

function getTypeface(fontstr) {
  var descriptors = parseFontString(fontstr);
  var typeface = getFromFontCache(descriptors);
  descriptors['typeface'] = typeface;
  return descriptors;
}

var fontCache;
function initCache() {
  if (!fontCache) {
    fontCache = {
      'Noto Mono': {
         // is used if we have this font family, but not the right style/variant/weight
        '*': CanvasKit.Typeface.GetDefault(),
      },
      'monospace': {
        '*': CanvasKit.Typeface.GetDefault(),
      }
    };
  }
}

// descriptors is like https://developer.mozilla.org/en-US/docs/Web/API/FontFace/FontFace
// The ones currently supported are family, style, variant, weight.
function addToFontCache(typeface, descriptors) {
  var key = (descriptors['style']   || 'normal') + '|' +
            (descriptors['variant'] || 'normal') + '|' +
            (descriptors['weight']  || 'normal');
  var fam = descriptors['family'];
  initCache();
  if (!fontCache[fam]) {
    // preload with a fallback to this typeface
    fontCache[fam] = {
      '*': typeface,
    };
  }
  fontCache[fam][key] = typeface;
}

function getFromFontCache(descriptors) {
  var key = (descriptors['style']   || 'normal') + '|' +
            (descriptors['variant'] || 'normal') + '|' +
            (descriptors['weight']  || 'normal');
  var fam = descriptors['family'];
  initCache();
  if (!fontCache[fam]) {
    return CanvasKit.Typeface.GetDefault();
  }
  return fontCache[fam][key] || fontCache[fam]['*'];
}

CanvasKit._testing['parseFontString'] = parseFontString;
