diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(10000) var test2Dˢ: sampler;
@group(0) @binding(10001) var test2Dᵗ: texture_2d<f32>;
@group(0) @binding(10002) var test2DRectˢ: sampler;
@group(0) @binding(10003) var test2DRectᵗ: texture_2d<f32>;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = textureSample(test2Dᵗ, test2Dˢ, vec2<f32>(0.5));
    (*_stageOut).sk_FragColor = textureSample(test2DRectᵗ, test2DRectˢ, vec2<f32>(0.5));
    let _skTemp4 = vec3<f32>(0.5);
    (*_stageOut).sk_FragColor = textureSample(test2DRectᵗ, test2DRectˢ, _skTemp4.xy / _skTemp4.z);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}
