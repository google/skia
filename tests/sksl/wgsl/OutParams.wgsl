diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  uFloat: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn various_parameter_types_vhhh(_stageOut: ptr<function, FSOut>, a: f32, b: ptr<function, f32>, c: ptr<function, f32>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(a, (*b), (*c), _globalUniforms.uFloat);
    (*b) = a;
    (*c) = _globalUniforms.uFloat;
  }
}
fn one_out_param_vh(h: ptr<function, f32>) {
  {
    (*h) = 2.0;
  }
}
fn one_out_param_indirect_vh(h: ptr<function, f32>) {
  {
    var _skTemp0: f32;
    one_out_param_vh(&_skTemp0);
    (*h) = _skTemp0;
  }
}
struct S {
  v: vec4<f32>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var x: f32 = 1.0;
    var _skTemp1: f32;
    one_out_param_vh(&_skTemp1);
    x = _skTemp1;
    var _skTemp2: f32;
    one_out_param_indirect_vh(&_skTemp2);
    x = _skTemp2;
    var _skTemp3: f32;
    var _skTemp4: f32 = x;
    various_parameter_types_vhhh(_stageOut, x + 1.0, &_skTemp3, &_skTemp4);
    x = _skTemp3;
    x = _skTemp4;
    var v: vec4<f32>;
    var _skTemp5: f32;
    var _skTemp6: f32 = v.x;
    various_parameter_types_vhhh(_stageOut, x + 1.0, &_skTemp5, &_skTemp6);
    v.x = _skTemp5;
    v.x = _skTemp6;
    var _skTemp7: f32;
    var _skTemp8: f32 = v.y;
    various_parameter_types_vhhh(_stageOut, x + 1.0, &_skTemp7, &_skTemp8);
    v.y = _skTemp7;
    v.y = _skTemp8;
    var _skTemp9: f32;
    var _skTemp10: f32 = v.y;
    various_parameter_types_vhhh(_stageOut, x + 1.0, &_skTemp9, &_skTemp10);
    v.x = _skTemp9;
    v.y = _skTemp10;
    var s: S;
    var _skTemp11: f32;
    var _skTemp12: f32 = x;
    various_parameter_types_vhhh(_stageOut, x + 1.0, &_skTemp11, &_skTemp12);
    s.v.x = _skTemp11;
    x = _skTemp12;
    var _skTemp13: f32;
    var _skTemp14: f32 = x;
    various_parameter_types_vhhh(_stageOut, x + 1.0, &_skTemp13, &_skTemp14);
    s.v.y = _skTemp13;
    x = _skTemp14;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
