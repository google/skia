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
fn out_params_are_distinct_bhh(x: ptr<function, f32>, y: ptr<function, f32>) -> bool {
  {
    (*x) = 1.0;
    (*y) = 2.0;
    return ((*x) == 1.0) && ((*y) == 2.0);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: f32 = 0.0;
    var _skTemp0: vec4<f32>;
    var _skTemp1: f32;
    var _skTemp2: f32;
    let _skTemp3 = out_params_are_distinct_bhh(&_skTemp1, &_skTemp2);
    x = _skTemp1;
    x = _skTemp2;
    if _skTemp3 {
      _skTemp0 = _globalUniforms.colorGreen;
    } else {
      _skTemp0 = _globalUniforms.colorRed;
    }
    return _skTemp0;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
