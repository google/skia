diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(10000) var sˢ: sampler;
@group(0) @binding(10001) var sᵗ: texture_2d<f32>;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    var a: vec4<f32> = vec4<f32>(textureSampleBias(sᵗ, sˢ, vec2<f32>(0.0), -0.475));
    let _skTemp2 = vec3<f32>(0.0);
    var b: vec4<f32> = vec4<f32>(textureSampleBias(sᵗ, sˢ, _skTemp2.xy / _skTemp2.z, -0.475));
    (*_stageOut).sk_FragColor = vec4<f32>(vec2<f32>(a.xy), vec2<f32>(b.xy));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}
