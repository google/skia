diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: f32;
    var y: f32;
    y = 1.0;
    x = y;
    var a: f32;
    var b: f32;
    var c: f32;
    c = 0.0;
    b = c;
    a = b;
    return vec4<f32>(a * b, f32(x), c, f32(y));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
