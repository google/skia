/**
 * @fileoverview Sample onDraw script for use with SkV8Example.
 */
var onDraw = function(){
    var p = new Path2D();
    p.moveTo(0, 0);
    p.bezierCurveTo(0, 100, 100, 0, 200, 200);
    p.close();
    p.moveTo(0, 300);
    p.arc(0, 300, 40, Math.PI/2, 3/2*Math.PI);
    function f(context) {
        context.translate(10, 10);
        for (var i=0; i<256; i++) {
            context.strokeStyle = '#0000' + toHex(i);
            context.stroke(p);
            context.translate(1, 0);
        }
        context.fillStyle = '#ff0000';
        print(context.width, context.height);
        context.resetTransform();
        context.fillRect(context.width/2, context.height/2, 20, 20);
    };
    return f;
}();


function toHex(n) {
  var s = n.toString(16);
  if (s.length == 1) {
    s = "0" + s;
  }
  return s;
}
