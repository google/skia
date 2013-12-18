/**
 * @fileoverview Sample onDraw script for use with SkV8Example.
 */
var onDraw = function(){
  var tick = 0;
  function f(canvas) {
    tick += 0.1;
    canvas.fillStyle = '#0000ff';
    canvas.fillRect(100, 100, Math.sin(tick)*100, Math.cos(tick)*100);
  };
  return f;
}();

function onTimeout() {
  inval();
  print("Got a timeout!");
  setTimeout(onTimeout, 33);
}

setTimeout(onTimeout, 33);
