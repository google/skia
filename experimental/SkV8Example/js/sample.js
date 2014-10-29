/**
 * @fileoverview Sample onDraw script for use with SkV8Example.
 */
var onDraw = function(){
  var ticks = 0;
  var b = new Path2DBuilder();
  b.rect(0, 0, 200, 200);
  var p = b.finalize();

  function f(context) {
    ticks += 1;

    context.translate(context.width/2, context.height/2);
    context.rotate(ticks/10);
    context.drawPath(p);

    inval();
  };

  function onTimeout() {
      console.log(ticks);
      ticks = 0;
      setTimeout(onTimeout, 1000);
  }
  setTimeout(onTimeout, 1000);

  return f;
}();

