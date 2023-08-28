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
fn returns_float2_f2() -> vec2<f32> {
  {
    return vec2<f32>(2.0);
  }
}
fn returns_float3_f3() -> vec3<f32> {
  {
    return vec3<f32>(3.0);
  }
}
fn returns_float4_f4() -> vec4<f32> {
  {
    return vec4<f32>(4.0);
  }
}
fn returns_float2x2_f22() -> mat2x2<f32> {
  {
    return mat2x2<f32>(2.0, 0.0, 0.0, 2.0);
  }
}
fn returns_float3x3_f33() -> mat3x3<f32> {
  {
    return mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0);
  }
}
fn returns_float4x4_f44() -> mat4x4<f32> {
  {
    return mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0);
  }
}
fn returns_half_h() -> f32 {
  {
    return 1.0;
  }
}
fn returns_half2_h2() -> vec2<f32> {
  {
    return vec2<f32>(2.0);
  }
}
fn returns_half3_h3() -> vec3<f32> {
  {
    return vec3<f32>(3.0);
  }
}
fn returns_half4_h4() -> vec4<f32> {
  {
    return vec4<f32>(4.0);
  }
}
fn returns_half2x2_h22() -> mat2x2<f32> {
  {
    return mat2x2<f32>(2.0, 0.0, 0.0, 2.0);
  }
}
fn returns_half3x3_h33() -> mat3x3<f32> {
  {
    return mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0);
  }
}
fn returns_half4x4_h44() -> mat4x4<f32> {
  {
    return mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0);
  }
}
fn returns_bool_b() -> bool {
  {
    return true;
  }
}
fn returns_bool2_b2() -> vec2<bool> {
  {
    return vec2<bool>(true);
  }
}
fn returns_bool3_b3() -> vec3<bool> {
  {
    return vec3<bool>(true);
  }
}
fn returns_bool4_b4() -> vec4<bool> {
  {
    return vec4<bool>(true);
  }
}
fn returns_int_i() -> i32 {
  {
    return 1;
  }
}
fn returns_int2_i2() -> vec2<i32> {
  {
    return vec2<i32>(2);
  }
}
fn returns_int3_i3() -> vec3<i32> {
  {
    return vec3<i32>(3);
  }
}
fn returns_int4_i4() -> vec4<i32> {
  {
    return vec4<i32>(4);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x1: f32 = 1.0;
    var x2: vec2<f32> = vec2<f32>(2.0);
    var x3: vec3<f32> = vec3<f32>(3.0);
    var x4: vec4<f32> = vec4<f32>(4.0);
    var x5: mat2x2<f32> = mat2x2<f32>(2.0, 0.0, 0.0, 2.0);
    var x6: mat3x3<f32> = mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0);
    var x7: mat4x4<f32> = mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0);
    var x8: f32 = 1.0;
    var x9: vec2<f32> = vec2<f32>(2.0);
    var x10: vec3<f32> = vec3<f32>(3.0);
    var x11: vec4<f32> = vec4<f32>(4.0);
    var x12: mat2x2<f32> = mat2x2<f32>(2.0, 0.0, 0.0, 2.0);
    var x13: mat3x3<f32> = mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0);
    var x14: mat4x4<f32> = mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0);
    var x15: bool = true;
    var x16: vec2<bool> = vec2<bool>(true);
    var x17: vec3<bool> = vec3<bool>(true);
    var x18: vec4<bool> = vec4<bool>(true);
    var x19: i32 = 1;
    var x20: vec2<i32> = vec2<i32>(2);
    var x21: vec3<i32> = vec3<i32>(3);
    var x22: vec4<i32> = vec4<i32>(4);
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    var _skTemp8: bool;
    var _skTemp9: bool;
    var _skTemp10: bool;
    var _skTemp11: bool;
    var _skTemp12: bool;
    var _skTemp13: bool;
    var _skTemp14: bool;
    var _skTemp15: bool;
    var _skTemp16: bool;
    var _skTemp17: bool;
    var _skTemp18: bool;
    var _skTemp19: bool;
    var _skTemp20: bool;
    var _skTemp21: bool;
    if x1 == 1.0 {
      let _skTemp22 = returns_float2_f2();
      _skTemp21 = all(x2 == _skTemp22);
    } else {
      _skTemp21 = false;
    }
    if _skTemp21 {
      let _skTemp23 = returns_float3_f3();
      _skTemp20 = all(x3 == _skTemp23);
    } else {
      _skTemp20 = false;
    }
    if _skTemp20 {
      let _skTemp24 = returns_float4_f4();
      _skTemp19 = all(x4 == _skTemp24);
    } else {
      _skTemp19 = false;
    }
    if _skTemp19 {
      let _skTemp25 = returns_float2x2_f22();
      let _skTemp26 = _skTemp25;
      _skTemp18 = (all(x5[0] == _skTemp26[0]) && all(x5[1] == _skTemp26[1]));
    } else {
      _skTemp18 = false;
    }
    if _skTemp18 {
      let _skTemp27 = returns_float3x3_f33();
      let _skTemp28 = _skTemp27;
      _skTemp17 = (all(x6[0] == _skTemp28[0]) && all(x6[1] == _skTemp28[1]) && all(x6[2] == _skTemp28[2]));
    } else {
      _skTemp17 = false;
    }
    if _skTemp17 {
      let _skTemp29 = returns_float4x4_f44();
      let _skTemp30 = _skTemp29;
      _skTemp16 = (all(x7[0] == _skTemp30[0]) && all(x7[1] == _skTemp30[1]) && all(x7[2] == _skTemp30[2]) && all(x7[3] == _skTemp30[3]));
    } else {
      _skTemp16 = false;
    }
    if _skTemp16 {
      let _skTemp31 = returns_half_h();
      _skTemp15 = (x8 == _skTemp31);
    } else {
      _skTemp15 = false;
    }
    if _skTemp15 {
      let _skTemp32 = returns_half2_h2();
      _skTemp14 = all(x9 == _skTemp32);
    } else {
      _skTemp14 = false;
    }
    if _skTemp14 {
      let _skTemp33 = returns_half3_h3();
      _skTemp13 = all(x10 == _skTemp33);
    } else {
      _skTemp13 = false;
    }
    if _skTemp13 {
      let _skTemp34 = returns_half4_h4();
      _skTemp12 = all(x11 == _skTemp34);
    } else {
      _skTemp12 = false;
    }
    if _skTemp12 {
      let _skTemp35 = returns_half2x2_h22();
      let _skTemp36 = _skTemp35;
      _skTemp11 = (all(x12[0] == _skTemp36[0]) && all(x12[1] == _skTemp36[1]));
    } else {
      _skTemp11 = false;
    }
    if _skTemp11 {
      let _skTemp37 = returns_half3x3_h33();
      let _skTemp38 = _skTemp37;
      _skTemp10 = (all(x13[0] == _skTemp38[0]) && all(x13[1] == _skTemp38[1]) && all(x13[2] == _skTemp38[2]));
    } else {
      _skTemp10 = false;
    }
    if _skTemp10 {
      let _skTemp39 = returns_half4x4_h44();
      let _skTemp40 = _skTemp39;
      _skTemp9 = (all(x14[0] == _skTemp40[0]) && all(x14[1] == _skTemp40[1]) && all(x14[2] == _skTemp40[2]) && all(x14[3] == _skTemp40[3]));
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      let _skTemp41 = returns_bool_b();
      _skTemp8 = (x15 == _skTemp41);
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      let _skTemp42 = returns_bool2_b2();
      _skTemp7 = all(x16 == _skTemp42);
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp43 = returns_bool3_b3();
      _skTemp6 = all(x17 == _skTemp43);
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp44 = returns_bool4_b4();
      _skTemp5 = all(x18 == _skTemp44);
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp45 = returns_int_i();
      _skTemp4 = (x19 == _skTemp45);
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp46 = returns_int2_i2();
      _skTemp3 = all(x20 == _skTemp46);
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      let _skTemp47 = returns_int3_i3();
      _skTemp2 = all(x21 == _skTemp47);
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp48 = returns_int4_i4();
      _skTemp1 = all(x22 == _skTemp48);
    } else {
      _skTemp1 = false;
    }
    if _skTemp1 {
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
