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
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct S {
  f: f32,
  af: array<f32, 5>,
  h4: vec4<f32>,
  ah4: array<vec4<f32>, 5>,
};
var<private> globalVar: vec4<f32>;
var<private> globalStruct: S;
fn keepAlive_vh(h: ptr<function, f32>) {
  {
  }
}
fn keepAlive_vf(f: ptr<function, f32>) {
  {
  }
}
fn keepAlive_vi(i: ptr<function, i32>) {
  {
  }
}
fn assignToFunctionParameter_vif(_skParam0: i32, y: ptr<function, f32>) {
  var x = _skParam0;
  {
    x = 1;
    (*y) = 1.0;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var i: i32 = 0;
    var i4: vec4<i32> = vec4<i32>(1, 2, 3, 4);
    var f3x3: mat3x3<f32> = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    var x: vec4<f32>;
    x.w = 0.0;
    x = vec4<f32>((vec2<f32>(0.0)), x.zw).yxzw;
    var ai: array<i32, 1>;
    ai[0] = 0;
    var ai4: array<vec4<i32>, 1>;
    ai4[0] = vec4<i32>(1, 2, 3, 4);
    var ah3x3: array<mat3x3<f32>, 1>;
    ah3x3[0] = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    var af4: array<vec4<f32>, 1>;
    af4[0].x = 0.0;
    af4[0] = (vec4<f32>(1.0)).zxwy;
    var s: S;
    s.f = 0.0;
    s.af[1] = 0.0;
    s.h4 = vec4<f32>((vec3<f32>(9.0)), s.h4.w).yzxw;
    s.ah4[2] = vec4<f32>((vec2<f32>(5.0)), s.ah4[2].xz).zxwy;
    globalVar = vec4<f32>(0.0);
    globalStruct.f = 0.0;
    var _skTemp0: f32 = f3x3[0].x;
    assignToFunctionParameter_vif(0, &_skTemp0);
    f3x3[0].x = _skTemp0;
    var l: f32;
    l = 0.0;
    ai[0] = ai[0] + ai4[0].x;
    s.f = 1.0;
    s.af[0] = 2.0;
    s.h4 = vec4<f32>(1.0);
    s.ah4[0] = vec4<f32>(2.0);
    var repeat: f32;
    repeat = 1.0;
    repeat = repeat;
    var _skTemp1: f32 = af4[0].x;
    keepAlive_vf(&_skTemp1);
    af4[0].x = _skTemp1;
    var _skTemp2: f32 = ah3x3[0][0].x;
    keepAlive_vh(&_skTemp2);
    ah3x3[0][0].x = _skTemp2;
    var _skTemp3: i32 = i;
    keepAlive_vi(&_skTemp3);
    i = _skTemp3;
    var _skTemp4: i32 = i4.y;
    keepAlive_vi(&_skTemp4);
    i4.y = _skTemp4;
    var _skTemp5: i32 = ai[0];
    keepAlive_vi(&_skTemp5);
    ai[0] = _skTemp5;
    var _skTemp6: i32 = ai4[0].x;
    keepAlive_vi(&_skTemp6);
    ai4[0].x = _skTemp6;
    var _skTemp7: f32 = x.y;
    keepAlive_vh(&_skTemp7);
    x.y = _skTemp7;
    var _skTemp8: f32 = s.f;
    keepAlive_vf(&_skTemp8);
    s.f = _skTemp8;
    var _skTemp9: f32 = l;
    keepAlive_vh(&_skTemp9);
    l = _skTemp9;
    var _skTemp10: f32 = f3x3[0].x;
    keepAlive_vf(&_skTemp10);
    f3x3[0].x = _skTemp10;
    var _skTemp11: f32 = repeat;
    keepAlive_vf(&_skTemp11);
    repeat = _skTemp11;
    return _globalUniforms.colorGreen;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
