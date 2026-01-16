diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn inout_params_are_distinct_bhh(x: ptr<function, f32>, y: ptr<function, f32>) -> bool {
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
    var _skTemp1: f32 = x;
    var _skTemp2: f32 = x;
    let _skTemp3 = inout_params_are_distinct_bhh(&_skTemp1, &_skTemp2);
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
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
