diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(10000) var test2D_Sampler: sampler;
@group(0) @binding(10001) var test2D_Texture: texture_2d<f32>;
@group(0) @binding(10002) var test2DRect_Sampler: sampler;
@group(0) @binding(10003) var test2DRect_Texture: texture_2d<f32>;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = textureSample(test2D_Texture, test2D_Sampler, vec2<f32>(0.5));
    (*_stageOut).sk_FragColor = textureSample(test2DRect_Texture, test2DRect_Sampler, vec2<f32>(0.5));
    let _skTemp4 = vec3<f32>(0.5);
    (*_stageOut).sk_FragColor = textureSample(test2DRect_Texture, test2DRect_Sampler, _skTemp4.xy / _skTemp4.z);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
