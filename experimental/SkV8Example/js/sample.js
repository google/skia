/**
 * @fileoverview Sample onDraw script for use with SkV8Example.
 */
var onDraw = function(){
  var tick = 0;
  var p = new Path2D();
  p.rect(0, 0, 200, 200);

  function f(context) {
    tick += 0.1;

    context.translate(context.width/2, context.height/2);
    context.rotate(tick);
    context.drawPath(p);
  };
  return f;
}();

function onTimeout() {
  inval();
  print(setTimeout(onTimeout, 33));
}

setTimeout(onTimeout, 33);
