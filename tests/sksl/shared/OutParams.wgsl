diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  colorWhite: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn out_half_vh(v: ptr<function, f32>) {
  {
    (*v) = _globalUniforms.colorWhite.x;
  }
}
fn out_half2_vh2(v: ptr<function, vec2<f32>>) {
  {
    (*v) = vec2<f32>(_globalUniforms.colorWhite.y);
  }
}
fn out_half3_vh3(v: ptr<function, vec3<f32>>) {
  {
    (*v) = vec3<f32>(_globalUniforms.colorWhite.z);
  }
}
fn out_half4_vh4(v: ptr<function, vec4<f32>>) {
  {
    (*v) = vec4<f32>(_globalUniforms.colorWhite.w);
  }
}
fn out_half2x2_vh22(v: ptr<function, mat2x2<f32>>) {
  {
    let _skTemp0 = _globalUniforms.colorWhite.x;
    (*v) = mat2x2<f32>(_skTemp0, 0.0, 0.0, _skTemp0);
  }
}
fn out_half3x3_vh33(v: ptr<function, mat3x3<f32>>) {
  {
    let _skTemp1 = _globalUniforms.colorWhite.y;
    (*v) = mat3x3<f32>(_skTemp1, 0.0, 0.0, 0.0, _skTemp1, 0.0, 0.0, 0.0, _skTemp1);
  }
}
fn out_half4x4_vh44(v: ptr<function, mat4x4<f32>>) {
  {
    let _skTemp2 = _globalUniforms.colorWhite.z;
    (*v) = mat4x4<f32>(_skTemp2, 0.0, 0.0, 0.0, 0.0, _skTemp2, 0.0, 0.0, 0.0, 0.0, _skTemp2, 0.0, 0.0, 0.0, 0.0, _skTemp2);
  }
}
fn out_int_vi(v: ptr<function, i32>) {
  {
    (*v) = i32(_globalUniforms.colorWhite.x);
  }
}
fn out_int2_vi2(v: ptr<function, vec2<i32>>) {
  {
    (*v) = vec2<i32>(i32(_globalUniforms.colorWhite.y));
  }
}
fn out_int3_vi3(v: ptr<function, vec3<i32>>) {
  {
    (*v) = vec3<i32>(i32(_globalUniforms.colorWhite.z));
  }
}
fn out_int4_vi4(v: ptr<function, vec4<i32>>) {
  {
    (*v) = vec4<i32>(i32(_globalUniforms.colorWhite.w));
  }
}
fn out_float_vf(v: ptr<function, f32>) {
  {
    (*v) = f32(_globalUniforms.colorWhite.x);
  }
}
fn out_float2_vf2(v: ptr<function, vec2<f32>>) {
  {
    (*v) = vec2<f32>(f32(_globalUniforms.colorWhite.y));
  }
}
fn out_float3_vf3(v: ptr<function, vec3<f32>>) {
  {
    (*v) = vec3<f32>(f32(_globalUniforms.colorWhite.z));
  }
}
fn out_float4_vf4(v: ptr<function, vec4<f32>>) {
  {
    (*v) = vec4<f32>(f32(_globalUniforms.colorWhite.w));
  }
}
fn out_float2x2_vf22(v: ptr<function, mat2x2<f32>>) {
  {
    let _skTemp3 = f32(_globalUniforms.colorWhite.x);
    (*v) = mat2x2<f32>(_skTemp3, 0.0, 0.0, _skTemp3);
  }
}
fn out_float3x3_vf33(v: ptr<function, mat3x3<f32>>) {
  {
    let _skTemp4 = f32(_globalUniforms.colorWhite.y);
    (*v) = mat3x3<f32>(_skTemp4, 0.0, 0.0, 0.0, _skTemp4, 0.0, 0.0, 0.0, _skTemp4);
  }
}
fn out_float4x4_vf44(v: ptr<function, mat4x4<f32>>) {
  {
    let _skTemp5 = f32(_globalUniforms.colorWhite.z);
    (*v) = mat4x4<f32>(_skTemp5, 0.0, 0.0, 0.0, 0.0, _skTemp5, 0.0, 0.0, 0.0, 0.0, _skTemp5, 0.0, 0.0, 0.0, 0.0, _skTemp5);
  }
}
fn out_bool_vb(v: ptr<function, bool>) {
  {
    (*v) = bool(_globalUniforms.colorWhite.x);
  }
}
fn out_bool2_vb2(v: ptr<function, vec2<bool>>) {
  {
    (*v) = vec2<bool>(bool(_globalUniforms.colorWhite.y));
  }
}
fn out_bool3_vb3(v: ptr<function, vec3<bool>>) {
  {
    (*v) = vec3<bool>(bool(_globalUniforms.colorWhite.z));
  }
}
fn out_bool4_vb4(v: ptr<function, vec4<bool>>) {
  {
    (*v) = vec4<bool>(bool(_globalUniforms.colorWhite.w));
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var h: f32;
    var _skTemp6: f32;
    out_half_vh(&_skTemp6);
    h = _skTemp6;
    var h2: vec2<f32>;
    var _skTemp7: vec2<f32>;
    out_half2_vh2(&_skTemp7);
    h2 = _skTemp7;
    var h3: vec3<f32>;
    var _skTemp8: vec3<f32>;
    out_half3_vh3(&_skTemp8);
    h3 = _skTemp8;
    var h4: vec4<f32>;
    var _skTemp9: vec4<f32>;
    out_half4_vh4(&_skTemp9);
    h4 = _skTemp9;
    var _skTemp10: f32;
    out_half_vh(&_skTemp10);
    h3.y = _skTemp10;
    var _skTemp11: vec2<f32>;
    out_half2_vh2(&_skTemp11);
    h3 = vec3<f32>((_skTemp11), h3.y).xzy;
    var _skTemp12: vec4<f32>;
    out_half4_vh4(&_skTemp12);
    h4 = (_skTemp12).zwxy;
    var h2x2: mat2x2<f32>;
    var _skTemp13: mat2x2<f32>;
    out_half2x2_vh22(&_skTemp13);
    h2x2 = _skTemp13;
    var h3x3: mat3x3<f32>;
    var _skTemp14: mat3x3<f32>;
    out_half3x3_vh33(&_skTemp14);
    h3x3 = _skTemp14;
    var h4x4: mat4x4<f32>;
    var _skTemp15: mat4x4<f32>;
    out_half4x4_vh44(&_skTemp15);
    h4x4 = _skTemp15;
    var _skTemp16: vec3<f32>;
    out_half3_vh3(&_skTemp16);
    h3x3[1] = _skTemp16;
    var _skTemp17: f32;
    out_half_vh(&_skTemp17);
    h4x4[3].w = _skTemp17;
    var _skTemp18: f32;
    out_half_vh(&_skTemp18);
    h2x2[0].x = _skTemp18;
    var i: i32;
    var _skTemp19: i32;
    out_int_vi(&_skTemp19);
    i = _skTemp19;
    var i2: vec2<i32>;
    var _skTemp20: vec2<i32>;
    out_int2_vi2(&_skTemp20);
    i2 = _skTemp20;
    var i3: vec3<i32>;
    var _skTemp21: vec3<i32>;
    out_int3_vi3(&_skTemp21);
    i3 = _skTemp21;
    var i4: vec4<i32>;
    var _skTemp22: vec4<i32>;
    out_int4_vi4(&_skTemp22);
    i4 = _skTemp22;
    var _skTemp23: vec3<i32>;
    out_int3_vi3(&_skTemp23);
    i4 = vec4<i32>((_skTemp23), i4.w);
    var _skTemp24: i32;
    out_int_vi(&_skTemp24);
    i2.y = _skTemp24;
    var f: f32;
    var _skTemp25: f32;
    out_float_vf(&_skTemp25);
    f = _skTemp25;
    var f2: vec2<f32>;
    var _skTemp26: vec2<f32>;
    out_float2_vf2(&_skTemp26);
    f2 = _skTemp26;
    var f3: vec3<f32>;
    var _skTemp27: vec3<f32>;
    out_float3_vf3(&_skTemp27);
    f3 = _skTemp27;
    var f4: vec4<f32>;
    var _skTemp28: vec4<f32>;
    out_float4_vf4(&_skTemp28);
    f4 = _skTemp28;
    var _skTemp29: vec2<f32>;
    out_float2_vf2(&_skTemp29);
    f3 = vec3<f32>((_skTemp29), f3.z);
    var _skTemp30: f32;
    out_float_vf(&_skTemp30);
    f2.x = _skTemp30;
    var f2x2: mat2x2<f32>;
    var _skTemp31: mat2x2<f32>;
    out_float2x2_vf22(&_skTemp31);
    f2x2 = _skTemp31;
    var f3x3: mat3x3<f32>;
    var _skTemp32: mat3x3<f32>;
    out_float3x3_vf33(&_skTemp32);
    f3x3 = _skTemp32;
    var f4x4: mat4x4<f32>;
    var _skTemp33: mat4x4<f32>;
    out_float4x4_vf44(&_skTemp33);
    f4x4 = _skTemp33;
    var _skTemp34: f32;
    out_float_vf(&_skTemp34);
    f2x2[0].x = _skTemp34;
    var b: bool;
    var _skTemp35: bool;
    out_bool_vb(&_skTemp35);
    b = _skTemp35;
    var b2: vec2<bool>;
    var _skTemp36: vec2<bool>;
    out_bool2_vb2(&_skTemp36);
    b2 = _skTemp36;
    var b3: vec3<bool>;
    var _skTemp37: vec3<bool>;
    out_bool3_vb3(&_skTemp37);
    b3 = _skTemp37;
    var b4: vec4<bool>;
    var _skTemp38: vec4<bool>;
    out_bool4_vb4(&_skTemp38);
    b4 = _skTemp38;
    var _skTemp39: vec2<bool>;
    out_bool2_vb2(&_skTemp39);
    b4 = vec4<bool>((_skTemp39), b4.yz).xzwy;
    var _skTemp40: bool;
    out_bool_vb(&_skTemp40);
    b3.z = _skTemp40;
    var ok: bool = true;
    ok = ok && (1.0 == ((((((h * h2.x) * h3.x) * h4.x) * h2x2[0].x) * h3x3[0].x) * h4x4[0].x));
    ok = ok && (1.0 == ((((((f * f2.x) * f3.x) * f4.x) * f2x2[0].x) * f3x3[0].x) * f4x4[0].x));
    ok = ok && (1 == (((i * i2.x) * i3.x) * i4.x));
    ok = ok && (((b && b2.x) && b3.x) && b4.x);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
