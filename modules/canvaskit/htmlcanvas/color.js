// Functions dealing with parsing/stringifying color go here.

// Create the following with
// node ./htmlcanvas/_namedcolors.js --expose-wasm
// JS/closure doesn't have a constexpr like thing which
// would really help here. Since we don't, we pre-compute
// the map, which saves (a tiny amount of) startup time
// and (a small amount of) code size.
/* @dict */
var colorMap = {'aliceblue':4293982463,'antiquewhite':4294634455,'aqua':4278255615,'aquamarine':4286578644,'azure':4293984255,'beige':4294309340,'bisque':4294960324,'black':4278190080,'blanchedalmond':4294962125,'blue':4278190335,'blueviolet':4287245282,'brown':4289014314,'burlywood':4292786311,'cadetblue':4284456608,'chartreuse':4286578432,'chocolate':4291979550,'coral':4294934352,'cornflowerblue':4284782061,'cornsilk':4294965468,'crimson':4292613180,'cyan':4278255615,'darkblue':4278190219,'darkcyan':4278225803,'darkgoldenrod':4290283019,'darkgray':4289309097,'darkgreen':4278215680,'darkgrey':4289309097,'darkkhaki':4290623339,'darkmagenta':4287299723,'darkolivegreen':4283788079,'darkorange':4294937600,'darkorchid':4288230092,'darkred':4287299584,'darksalmon':4293498490,'darkseagreen':4287609999,'darkslateblue':4282924427,'darkslategray':4281290575,'darkslategrey':4281290575,'darkturquoise':4278243025,'darkviolet':4287889619,'deeppink':4294907027,'deepskyblue':4278239231,'dimgray':4285098345,'dimgrey':4285098345,'dodgerblue':4280193279,'firebrick':4289864226,'floralwhite':4294966000,'forestgreen':4280453922,'fuchsia':4294902015,'gainsboro':4292664540,'ghostwhite':4294506751,'gold':4294956800,'goldenrod':4292519200,'gray':4286611584,'green':4278222848,'greenyellow':4289593135,'grey':4286611584,'honeydew':4293984240,'hotpink':4294928820,'indianred':4291648604,'indigo':4283105410,'ivory':4294967280,'khaki':4293977740,'lavender':4293322490,'lavenderblush':4294963445,'lawngreen':4286381056,'lemonchiffon':4294965965,'lightblue':4289583334,'lightcoral':4293951616,'lightcyan':4292935679,'lightgoldenrodyellow':4294638290,'lightgray':4292072403,'lightgreen':4287688336,'lightgrey':4292072403,'lightpink':4294948545,'lightsalmon':4294942842,'lightseagreen':4280332970,'lightskyblue':4287090426,'lightslategray':4286023833,'lightslategrey':4286023833,'lightsteelblue':4289774814,'lightyellow':4294967264,'lime':4278255360,'limegreen':4281519410,'linen':4294635750,'magenta':4294902015,'maroon':4286578688,'mediumaquamarine':4284927402,'mediumblue':4278190285,'mediumorchid':4290401747,'mediumpurple':4287852763,'mediumseagreen':4282168177,'mediumslateblue':4286277870,'mediumspringgreen':4278254234,'mediumturquoise':4282962380,'mediumvioletred':4291237253,'midnightblue':4279834992,'mintcream':4294311930,'mistyrose':4294960353,'moccasin':4294960309,'navajowhite':4294958765,'navy':4278190208,'oldlace':4294833638,'olive':4286611456,'olivedrab':4285238819,'orange':4294944000,'orangered':4294919424,'orchid':4292505814,'palegoldenrod':4293847210,'palegreen':4288215960,'paleturquoise':4289720046,'palevioletred':4292571283,'papayawhip':4294963157,'peachpuff':4294957753,'peru':4291659071,'pink':4294951115,'plum':4292714717,'powderblue':4289781990,'purple':4286578816,'rebeccapurple':4284887961,'red':4294901760,'rosybrown':4290547599,'royalblue':4282477025,'saddlebrown':4287317267,'salmon':4294606962,'sandybrown':4294222944,'seagreen':4281240407,'seashell':4294964718,'sienna':4288696877,'silver':4290822336,'skyblue':4287090411,'slateblue':4285160141,'slategray':4285563024,'slategrey':4285563024,'snow':4294966010,'springgreen':4278255487,'steelblue':4282811060,'tan':4291998860,'teal':4278222976,'thistle':4292394968,'transparent':0,'tomato':4294927175,'turquoise':4282441936,'violet':4293821166,'wheat':4294303411,'white':4294967295,'whitesmoke':4294309365,'yellow':4294967040,'yellowgreen':4288335154};
function colorToString(skcolor) {
  // https://html.spec.whatwg.org/multipage/canvas.html#serialisation-of-a-color
  var components = CanvasKit.getColorComponents(skcolor);
  var r = components[0];
  var g = components[1];
  var b = components[2];
  var a = components[3];
  if (a === 1.0) {
    // hex
    r = r.toString(16).toLowerCase();
    g = g.toString(16).toLowerCase();
    b = b.toString(16).toLowerCase();
    r = (r.length === 1 ? '0'+r: r);
    g = (g.length === 1 ? '0'+g: g);
    b = (b.length === 1 ? '0'+b: b);
    return '#'+r+g+b;
  } else {
    a = (a === 0 || a === 1) ? a : a.toFixed(8);
    return 'rgba('+r+', '+g+', '+b+', '+a+')';
  }
}

function parseColor(colorStr) {
  return CanvasKit.parseColorString(colorStr, colorMap);
}

CanvasKit._testing['parseColor'] = parseColor;
CanvasKit._testing['colorToString'] = colorToString;
