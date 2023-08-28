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
fn fnGreen_h4bf2(b: bool, _skParam1: vec2<f32>) -> vec4<f32> {
  {
    return _globalUniforms.colorGreen;
  }
}
fn fnRed_h4ifS(_skParam0: i32, f: f32, _skParam2: S) -> vec4<f32> {
  {
    return _globalUniforms.colorRed;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
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
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
