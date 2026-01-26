diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
@group(0) @binding(10000) var tex_Sampler: sampler;
@group(0) @binding(10001) var tex_Texture: texture_2d<f32>;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let a: vec4<f32> = vec4<f32>(vec4<f16>(textureSample(tex_Texture, tex_Sampler, vec2<f32>(0.0))));
    let _skTemp2 = vec3<f32>(0.0);
    let b: vec4<f32> = vec4<f32>(vec4<f16>(textureSample(tex_Texture, tex_Sampler, _skTemp2.xy / _skTemp2.z)));
    (*_stageOut).sk_FragColor = vec4<f16>(vec2<f16>(a.xy), vec2<f16>(b.zw));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
