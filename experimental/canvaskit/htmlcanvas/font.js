// Functions dealing with parsing/stringifying fonts go here.

var units = 'px|pt|pc|in|cm|mm|%|em|ex|ch|rem|q';
var fontSizeRegex = new RegExp('([\\d\\.]+)(' + units + ')');
var defaultHeight = 16;
// Based off of node-canvas's parseFont
// returns font size in px, which represents the em width.
function parseFontSize(fontStr) {
  // This is naive and doesn't account for line-height yet
  // (but neither does node-canvas's?)
  var fontSize = fontSizeRegex.exec(fontStr);
  if (!fontSize) {
    SkDebug('Could not parse font size' + fontStr);
    return 16;
  }
  var size = parseFloat(fontSize[1]);
  var unit = fontSize[2];
  switch (unit) {
    case 'pt':
    case 'em':
    case 'rem':
      return size * 4/3;
    case 'px':
      return size;
    case 'pc':
      return size * 16;
    case 'in':
      return size * 96;
    case 'cm':
      return size * 96.0 / 2.54;
    case 'mm':
      return size * (96.0 / 25.4);
    case 'q': // quarter millimeters
      return size * (96.0 / 25.4 / 4);
    case '%':
      return size * (defaultHeight / 75);
  }
}