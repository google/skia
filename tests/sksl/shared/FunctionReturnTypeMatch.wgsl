diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
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
    const x1: f32 = 1.0;
    const x2: vec2<f32> = vec2<f32>(2.0);
    const x3: vec3<f32> = vec3<f32>(3.0);
    const x4: vec4<f32> = vec4<f32>(4.0);
    const x5: mat2x2<f32> = mat2x2<f32>(2.0, 0.0, 0.0, 2.0);
    const x6: mat3x3<f32> = mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0);
    const x7: mat4x4<f32> = mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0);
    const x8: f32 = 1.0;
    const x9: vec2<f32> = vec2<f32>(2.0);
    const x10: vec3<f32> = vec3<f32>(3.0);
    const x11: vec4<f32> = vec4<f32>(4.0);
    const x12: mat2x2<f32> = mat2x2<f32>(2.0, 0.0, 0.0, 2.0);
    const x13: mat3x3<f32> = mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0);
    const x14: mat4x4<f32> = mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0);
    const x15: bool = true;
    const x16: vec2<bool> = vec2<bool>(true);
    const x17: vec3<bool> = vec3<bool>(true);
    const x18: vec4<bool> = vec4<bool>(true);
    const x19: i32 = 1;
    const x20: vec2<i32> = vec2<i32>(2);
    const x21: vec3<i32> = vec3<i32>(3);
    const x22: vec4<i32> = vec4<i32>(4);
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
      _skTemp21 = all(x2 == returns_float2_f2());
    } else {
      _skTemp21 = false;
    }
    if _skTemp21 {
      _skTemp20 = all(x3 == returns_float3_f3());
    } else {
      _skTemp20 = false;
    }
    if _skTemp20 {
      _skTemp19 = all(x4 == returns_float4_f4());
    } else {
      _skTemp19 = false;
    }
    if _skTemp19 {
      let _skTemp22 = returns_float2x2_f22();
      _skTemp18 = (all(x5[0] == _skTemp22[0]) && all(x5[1] == _skTemp22[1]));
    } else {
      _skTemp18 = false;
    }
    if _skTemp18 {
      let _skTemp23 = returns_float3x3_f33();
      _skTemp17 = (all(x6[0] == _skTemp23[0]) && all(x6[1] == _skTemp23[1]) && all(x6[2] == _skTemp23[2]));
    } else {
      _skTemp17 = false;
    }
    if _skTemp17 {
      let _skTemp24 = returns_float4x4_f44();
      _skTemp16 = (all(x7[0] == _skTemp24[0]) && all(x7[1] == _skTemp24[1]) && all(x7[2] == _skTemp24[2]) && all(x7[3] == _skTemp24[3]));
    } else {
      _skTemp16 = false;
    }
    if _skTemp16 {
      _skTemp15 = (x8 == returns_half_h());
    } else {
      _skTemp15 = false;
    }
    if _skTemp15 {
      _skTemp14 = all(x9 == returns_half2_h2());
    } else {
      _skTemp14 = false;
    }
    if _skTemp14 {
      _skTemp13 = all(x10 == returns_half3_h3());
    } else {
      _skTemp13 = false;
    }
    if _skTemp13 {
      _skTemp12 = all(x11 == returns_half4_h4());
    } else {
      _skTemp12 = false;
    }
    if _skTemp12 {
      let _skTemp25 = returns_half2x2_h22();
      _skTemp11 = (all(x12[0] == _skTemp25[0]) && all(x12[1] == _skTemp25[1]));
    } else {
      _skTemp11 = false;
    }
    if _skTemp11 {
      let _skTemp26 = returns_half3x3_h33();
      _skTemp10 = (all(x13[0] == _skTemp26[0]) && all(x13[1] == _skTemp26[1]) && all(x13[2] == _skTemp26[2]));
    } else {
      _skTemp10 = false;
    }
    if _skTemp10 {
      let _skTemp27 = returns_half4x4_h44();
      _skTemp9 = (all(x14[0] == _skTemp27[0]) && all(x14[1] == _skTemp27[1]) && all(x14[2] == _skTemp27[2]) && all(x14[3] == _skTemp27[3]));
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      _skTemp8 = (x15 == returns_bool_b());
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      _skTemp7 = all(x16 == returns_bool2_b2());
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      _skTemp6 = all(x17 == returns_bool3_b3());
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      _skTemp5 = all(x18 == returns_bool4_b4());
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      _skTemp4 = (x19 == returns_int_i());
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      _skTemp3 = all(x20 == returns_int2_i2());
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = all(x21 == returns_int3_i3());
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      _skTemp1 = all(x22 == returns_int4_i4());
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
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
