diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(10000) var t_Sampler: sampler;
@group(0) @binding(10001) var t_Texture: texture_2d<f32>;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp2 = dpdx(coords);
    let _skTemp3 = dpdy(coords);
    return textureSampleGrad(t_Texture, t_Sampler, coords, _skTemp2, _skTemp3);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
