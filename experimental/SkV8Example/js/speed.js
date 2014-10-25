/**
 * @fileoverview Sample onDraw script for use with SkV8Example.
 */
var onDraw = function(){
  var tick = 0;
  function f(canvas) {
    tick += 0.1;
    canvas.fillStyle = '#0000ff';
    canvas.fillRect(100, 100, Math.sin(tick)*100, Math.cos(tick)*100);
    inval();
  };

  function onTimeout() {
      print(tick*10, " FPS");
      setTimeout(onTimeout, 1000);
      tick=0;
  }

  setTimeout(onTimeout, 1000);

  return f;
}();

