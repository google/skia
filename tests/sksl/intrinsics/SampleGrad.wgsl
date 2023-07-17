### Compilation failed:

error: :9:17 error: unresolved type 'sampler2D'
var<private> t: sampler2D;
                ^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
var<private> t: sampler2D;
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    let _skTemp0 = dFdx(coords);
    let _skTemp1 = dFdy(coords);
    let _skTemp2 = sampleGrad(t, coords, _skTemp0, _skTemp1);
    return _skTemp2;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}

1 error
