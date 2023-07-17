diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct S {
  i: i32,
};
fn fnGreen_h4bf2(_skParam0: bool, _skParam1: vec2<f32>) -> vec4<f32> {
  let b = _skParam0;
  {
    return _globalUniforms.colorGreen;
  }
}
fn fnRed_h4ifS(_skParam0: i32, _skParam1: f32, _skParam2: S) -> vec4<f32> {
  let f = _skParam1;
  {
    return _globalUniforms.colorRed;
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var _skTemp0: vec4<f32>;
    if bool(_globalUniforms.colorGreen.y) {
      let _skTemp1 = fnGreen_h4bf2(true, coords);
      _skTemp0 = _skTemp1;
    } else {
      let _skTemp2 = fnRed_h4ifS(123, 3.14, S(0));
      _skTemp0 = _skTemp2;
    }
    return _skTemp0;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
