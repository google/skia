diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var x: f32;
    var y: f32;
    y = 1.0;
    x = y;
    var a: f16;
    var b: f16;
    var c: f16;
    c = 0.0h;
    b = c;
    a = b;
    return vec4<f16>(a * b, f16(x), c, f16(y));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
