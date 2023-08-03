### Compilation failed:

error: :12:51 error: unresolved type 'sampler2D'
fn helpers_helper_h4ZT(_stageIn: FSIn, _skParam0: sampler2D, _skParam1: texture2D) -> vec4<f32> {
                                                  ^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @location(1) c: vec2<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
@group(0) @binding(10000) var aTexture: texture_2d<f32>;
@group(0) @binding(10001) var aSampledTextureˢ: sampler;
@group(0) @binding(10002) var aSampledTextureᵗ: texture_2d<f32>;
fn helpers_helper_h4ZT(_stageIn: FSIn, _skParam0: sampler2D, _skParam1: texture2D) -> vec4<f32> {
  let s = _skParam0;
  let t = _skParam1;
  {
    return textureSample(sᵗ, sˢ, _stageIn.c);
  }
}
fn helper_h4TZ(_stageIn: FSIn, _skParam0: texture2D, _skParam1: sampler2D) -> vec4<f32> {
  let t = _skParam0;
  let s = _skParam1;
  {
    let _skTemp3 = helpers_helper_h4ZT(_stageIn, s, t);
    return _skTemp3;
  }
}
fn main(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    let _skTemp4 = helper_h4TZ(_stageIn, aTexture, aSampledTexture);
    (*_stageOut).sk_FragColor = _skTemp4;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(_stageIn, &_stageOut);
  return _stageOut;
}

1 error
