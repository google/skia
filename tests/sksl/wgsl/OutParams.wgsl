diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  uFloat: f16,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn various_parameter_types_vhhh(_stageOut: ptr<function, FSOut>, a: f16, b: ptr<function, f16>, c: ptr<function, f16>) {
  {
    (*_stageOut).sk_FragColor = vec4<f16>(a, (*b), (*c), _globalUniforms.uFloat);
    (*b) = a;
    (*c) = _globalUniforms.uFloat;
  }
}
fn one_out_param_vh(h: ptr<function, f16>) {
  {
    (*h) = 2.0h;
  }
}
fn one_out_param_indirect_vh(h: ptr<function, f16>) {
  {
    var _skTemp0: f16;
    one_out_param_vh(&_skTemp0);
    (*h) = _skTemp0;
  }
}
struct S {
  v: vec4<f16>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var x: f16 = 1.0h;
    var _skTemp1: f16;
    one_out_param_vh(&_skTemp1);
    x = _skTemp1;
    var _skTemp2: f16;
    one_out_param_indirect_vh(&_skTemp2);
    x = _skTemp2;
    var _skTemp3: f16;
    var _skTemp4: f16 = x;
    various_parameter_types_vhhh(_stageOut, x + 1.0h, &_skTemp3, &_skTemp4);
    x = _skTemp3;
    x = _skTemp4;
    var v: vec4<f16>;
    var _skTemp5: f16;
    var _skTemp6: f16 = v.x;
    various_parameter_types_vhhh(_stageOut, x + 1.0h, &_skTemp5, &_skTemp6);
    v.x = _skTemp5;
    v.x = _skTemp6;
    var _skTemp7: f16;
    var _skTemp8: f16 = v.y;
    various_parameter_types_vhhh(_stageOut, x + 1.0h, &_skTemp7, &_skTemp8);
    v.y = _skTemp7;
    v.y = _skTemp8;
    var _skTemp9: f16;
    var _skTemp10: f16 = v.y;
    various_parameter_types_vhhh(_stageOut, x + 1.0h, &_skTemp9, &_skTemp10);
    v.x = _skTemp9;
    v.y = _skTemp10;
    var s: S;
    var _skTemp11: f16;
    var _skTemp12: f16 = x;
    various_parameter_types_vhhh(_stageOut, x + 1.0h, &_skTemp11, &_skTemp12);
    s.v.x = _skTemp11;
    x = _skTemp12;
    var _skTemp13: f16;
    var _skTemp14: f16 = x;
    various_parameter_types_vhhh(_stageOut, x + 1.0h, &_skTemp13, &_skTemp14);
    s.v.y = _skTemp13;
    x = _skTemp14;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
