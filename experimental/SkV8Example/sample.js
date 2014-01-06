/**
 * @fileoverview Sample onDraw script for use with SkV8Example.
 */
var onDraw = function(){
  var tick = 0;
  function f(context) {
    tick += 0.1;
    context.fillStyle = '#0000ff';
    context.fillRect(100, 100, Math.sin(tick)*100, Math.cos(tick)*100);
  };
  return f;
}();

function onTimeout() {
  inval();
  print(setTimeout(onTimeout, 33));
}

setTimeout(onTimeout, 33);
