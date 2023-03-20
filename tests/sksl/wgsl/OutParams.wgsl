struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    uFloat: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _outParamHelper_0_one_out_param_vh(h: ptr<function, f32>) {
    var _var0: f32;
    one_out_param_vh(&_var0);
    (*h) = _var0;
}
fn _outParamHelper_1_one_out_param_vh(x: ptr<function, f32>) {
    var _var0: f32;
    one_out_param_vh(&_var0);
    (*x) = _var0;
}
fn _outParamHelper_2_one_out_param_indirect_vh(x: ptr<function, f32>) {
    var _var0: f32;
    one_out_param_indirect_vh(&_var0);
    (*x) = _var0;
}
fn _outParamHelper_3_various_parameter_types_vhhh(_stageOut: ptr<function, FSOut>, _var0: f32, x: ptr<function, f32>, _unused2: ptr<function, f32>) {
    var _var1: f32;
    var _var2: f32 = (*x);
    various_parameter_types_vhhh(_stageOut, _var0, &_var1, &_var2);
    (*x) = _var1;
    (*x) = _var2;
}
fn _outParamHelper_4_various_parameter_types_vhhh(_stageOut: ptr<function, FSOut>, _var0: f32, v: ptr<function, vec4<f32>>, _unused2: ptr<function, vec4<f32>>) {
    var _var1: f32;
    var _var2: f32 = (*v).x;
    various_parameter_types_vhhh(_stageOut, _var0, &_var1, &_var2);
    (*v).x = _var1;
    (*v).x = _var2;
}
fn _outParamHelper_5_various_parameter_types_vhhh(_stageOut: ptr<function, FSOut>, _var0: f32, v: ptr<function, vec4<f32>>, _unused2: ptr<function, vec4<f32>>) {
    var _var1: f32;
    var _var2: f32 = (*v).y;
    various_parameter_types_vhhh(_stageOut, _var0, &_var1, &_var2);
    (*v).y = _var1;
    (*v).y = _var2;
}
fn _outParamHelper_6_various_parameter_types_vhhh(_stageOut: ptr<function, FSOut>, _var0: f32, v: ptr<function, vec4<f32>>, _unused2: ptr<function, vec4<f32>>) {
    var _var1: f32;
    var _var2: f32 = (*v).y;
    various_parameter_types_vhhh(_stageOut, _var0, &_var1, &_var2);
    (*v).x = _var1;
    (*v).y = _var2;
}
fn _outParamHelper_7_various_parameter_types_vhhh(_stageOut: ptr<function, FSOut>, _var0: f32, s: ptr<function, S>, x: ptr<function, f32>) {
    var _var1: f32;
    var _var2: f32 = (*x);
    various_parameter_types_vhhh(_stageOut, _var0, &_var1, &_var2);
    (*s).v.x = _var1;
    (*x) = _var2;
}
fn _outParamHelper_8_various_parameter_types_vhhh(_stageOut: ptr<function, FSOut>, _var0: f32, s: ptr<function, S>, x: ptr<function, f32>) {
    var _var1: f32;
    var _var2: f32 = (*x);
    various_parameter_types_vhhh(_stageOut, _var0, &_var1, &_var2);
    (*s).v.y = _var1;
    (*x) = _var2;
}
fn various_parameter_types_vhhh(_stageOut: ptr<function, FSOut>, a: f32, b: ptr<function, f32>, c: ptr<function, f32>) {
    (*_stageOut).sk_FragColor = vec4<f32>(a, (*b), (*c), _globalUniforms.uFloat);
    (*b) = a;
    (*c) = _globalUniforms.uFloat;
}
fn one_out_param_vh(h: ptr<function, f32>) {
    (*h) = 2.0;
}
fn one_out_param_indirect_vh(h: ptr<function, f32>) {
    _outParamHelper_0_one_out_param_vh(&(*h));
}
struct S {
    v: vec4<f32>,
};
fn main(_stageOut: ptr<function, FSOut>) {
    var x: f32 = 1.0;
    _outParamHelper_1_one_out_param_vh(&x);
    _outParamHelper_2_one_out_param_indirect_vh(&x);
    _outParamHelper_3_various_parameter_types_vhhh(_stageOut, x + 1.0, &x, &x);
    var v: vec4<f32>;
    _outParamHelper_4_various_parameter_types_vhhh(_stageOut, x + 1.0, &v, &v);
    _outParamHelper_5_various_parameter_types_vhhh(_stageOut, x + 1.0, &v, &v);
    _outParamHelper_6_various_parameter_types_vhhh(_stageOut, x + 1.0, &v, &v);
    var s: S;
    _outParamHelper_7_various_parameter_types_vhhh(_stageOut, x + 1.0, &s, &x);
    _outParamHelper_8_various_parameter_types_vhhh(_stageOut, x + 1.0, &s, &x);
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    main(&_stageOut);
    return _stageOut;
}
