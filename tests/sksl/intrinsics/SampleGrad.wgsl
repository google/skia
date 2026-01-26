diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
@group(0) @binding(10000) var t_Sampler: sampler;
@group(0) @binding(10001) var t_Texture: texture_2d<f32>;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    return vec4<f16>(textureSampleGrad(t_Texture, t_Sampler, coords, dpdx(coords), dpdy(coords)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
