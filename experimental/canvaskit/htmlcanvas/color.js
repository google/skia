// Functions dealing with parsing/stringifying color go here.

// Create the following with
// node ./htmlcanvas/_namedcolors.js --expose-wasm
// JS/closure doesn't have a constexpr like thing which
// would really help here. Since we don't, we pre-compute
// the map, which saves (a tiny amount of) startup time
// and (a small amount of) code size.
/* @dict */
var colorMap = {'aliceblue':-984833,'antiquewhite':-332841,'aqua':-16711681,'aquamarine':-8388652,'azure':-983041,'beige':-657956,'bisque':-6972,'black':-16777216,'blanchedalmond':-5171,'blue':-16776961,'blueviolet':-7722014,'brown':-5952982,'burlywood':-2180985,'cadetblue':-10510688,'chartreuse':-8388864,'chocolate':-2987746,'coral':-32944,'cornflowerblue':-10185235,'cornsilk':-1828,'crimson':-2354116,'cyan':-16711681,'darkblue':-16777077,'darkcyan':-16741493,'darkgoldenrod':-4684277,'darkgray':-5658199,'darkgreen':-16751616,'darkgrey':-5658199,'darkkhaki':-4343957,'darkmagenta':-7667573,'darkolivegreen':-11179217,'darkorange':-29696,'darkorchid':-6737204,'darkred':-7667712,'darksalmon':-1468806,'darkseagreen':-7357297,'darkslateblue':-12042869,'darkslategray':-13676721,'darkslategrey':-13676721,'darkturquoise':-16724271,'darkviolet':-7077677,'deeppink':-60269,'deepskyblue':-16728065,'dimgray':-9868951,'dimgrey':-9868951,'dodgerblue':-14774017,'firebrick':-5103070,'floralwhite':-1296,'forestgreen':-14513374,'fuchsia':-65281,'gainsboro':-2302756,'ghostwhite':-460545,'gold':-10496,'goldenrod':-2448096,'gray':-8355712,'green':-16744448,'greenyellow':-5374161,'grey':-8355712,'honeydew':-983056,'hotpink':-38476,'indianred':-3318692,'indigo':-11861886,'ivory':-16,'khaki':-989556,'lavender':-1644806,'lavenderblush':-3851,'lawngreen':-8586240,'lemonchiffon':-1331,'lightblue':-5383962,'lightcoral':-1015680,'lightcyan':-2031617,'lightgoldenrodyellow':-329006,'lightgray':-2894893,'lightgreen':-7278960,'lightgrey':-2894893,'lightpink':-18751,'lightsalmon':-24454,'lightseagreen':-14634326,'lightskyblue':-7876870,'lightslategray':-8943463,'lightslategrey':-8943463,'lightsteelblue':-5192482,'lightyellow':-32,'lime':-16711936,'limegreen':-13447886,'linen':-331546,'magenta':-65281,'maroon':-8388608,'mediumaquamarine':-10039894,'mediumblue':-16777011,'mediumorchid':-4565549,'mediumpurple':-7114533,'mediumseagreen':-12799119,'mediumslateblue':-8689426,'mediumspringgreen':-16713062,'mediumturquoise':-12004916,'mediumvioletred':-3730043,'midnightblue':-15132304,'mintcream':-655366,'mistyrose':-6943,'moccasin':-6987,'navajowhite':-8531,'navy':-16777088,'oldlace':-133658,'olive':-8355840,'olivedrab':-9728477,'orange':-23296,'orangered':-47872,'orchid':-2461482,'palegoldenrod':-1120086,'palegreen':-6751336,'paleturquoise':-5247250,'palevioletred':-2396013,'papayawhip':-4139,'peachpuff':-9543,'peru':-3308225,'pink':-16181,'plum':-2252579,'powderblue':-5185306,'purple':-8388480,'rebeccapurple':-10079335,'red':-65536,'rosybrown':-4419697,'royalblue':-12490271,'saddlebrown':-7650029,'salmon':-360334,'sandybrown':-744352,'seagreen':-13726889,'seashell':-2578,'sienna':-6270419,'silver':-4144960,'skyblue':-7876885,'slateblue':-9807155,'slategray':-9404272,'slategrey':-9404272,'snow':-1286,'springgreen':-16711809,'steelblue':-12156236,'tan':-2968436,'teal':-16744320,'thistle':-2572328,'transparent':0,'tomato':-40121,'turquoise':-12525360,'violet':-1146130,'wheat':-663885,'white':-1,'whitesmoke':-657931,'yellow':-256,'yellowgreen':-6632142};

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
  SkDebug('unrecognized color ' + colorStr);
  return CanvasKit.BLACK;
}

CanvasKit._testing['parseColor'] = parseColor;
CanvasKit._testing['colorToString'] = colorToString;
