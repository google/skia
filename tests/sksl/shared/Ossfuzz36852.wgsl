diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const x: mat2x2<f16> = mat2x2<f16>(0.0h, 1.0h, 2.0h, 3.0h);
    let y: vec2<f32> = vec2<f32>(vec4<f16>(x[0], x[1]).xy);
    return vec4<f16>(vec4<f32>(y, 0.0, 1.0));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
