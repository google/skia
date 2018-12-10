// Functions dealing with parsing/stringifying fonts go here.

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
    SkDebug('Could not parse font size' + fontStr);
    return 16;
  }
  var size = parseFloat(fontSize[1]);
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