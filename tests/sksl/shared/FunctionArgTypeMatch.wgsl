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
fn takes_void_b() -> bool {
  {
    return true;
  }
}
fn takes_float_bf(x: f32) -> bool {
  {
    return true;
  }
}
fn takes_float2_bf2(x: vec2<f32>) -> bool {
  {
    return true;
  }
}
fn takes_float3_bf3(x: vec3<f32>) -> bool {
  {
    return true;
  }
}
fn takes_float4_bf4(x: vec4<f32>) -> bool {
  {
    return true;
  }
}
fn takes_float2x2_bf22(x: mat2x2<f32>) -> bool {
  {
    return true;
  }
}
fn takes_float3x3_bf33(x: mat3x3<f32>) -> bool {
  {
    return true;
  }
}
fn takes_float4x4_bf44(x: mat4x4<f32>) -> bool {
  {
    return true;
  }
}
fn takes_half_bh(x: f32) -> bool {
  {
    return true;
  }
}
fn takes_half2_bh2(x: vec2<f32>) -> bool {
  {
    return true;
  }
}
fn takes_half3_bh3(x: vec3<f32>) -> bool {
  {
    return true;
  }
}
fn takes_half4_bh4(x: vec4<f32>) -> bool {
  {
    return true;
  }
}
fn takes_half2x2_bh22(x: mat2x2<f32>) -> bool {
  {
    return true;
  }
}
fn takes_half3x3_bh33(x: mat3x3<f32>) -> bool {
  {
    return true;
  }
}
fn takes_half4x4_bh44(x: mat4x4<f32>) -> bool {
  {
    return true;
  }
}
fn takes_bool_bb(x: bool) -> bool {
  {
    return true;
  }
}
fn takes_bool2_bb2(x: vec2<bool>) -> bool {
  {
    return true;
  }
}
fn takes_bool3_bb3(x: vec3<bool>) -> bool {
  {
    return true;
  }
}
fn takes_bool4_bb4(x: vec4<bool>) -> bool {
  {
    return true;
  }
}
fn takes_int_bi(x: i32) -> bool {
  {
    return true;
  }
}
fn takes_int2_bi2(x: vec2<i32>) -> bool {
  {
    return true;
  }
}
fn takes_int3_bi3(x: vec3<i32>) -> bool {
  {
    return true;
  }
}
fn takes_int4_bi4(x: vec4<i32>) -> bool {
  {
    return true;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
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
    var _skTemp22: bool;
    var _skTemp23: bool;
    if true {
      let _skTemp24 = takes_void_b();
      _skTemp23 = _skTemp24;
    } else {
      _skTemp23 = false;
    }
    if _skTemp23 {
      let _skTemp25 = takes_float_bf(1.0);
      _skTemp22 = _skTemp25;
    } else {
      _skTemp22 = false;
    }
    if _skTemp22 {
      let _skTemp26 = takes_float2_bf2(vec2<f32>(2.0));
      _skTemp21 = _skTemp26;
    } else {
      _skTemp21 = false;
    }
    if _skTemp21 {
      let _skTemp27 = takes_float3_bf3(vec3<f32>(3.0));
      _skTemp20 = _skTemp27;
    } else {
      _skTemp20 = false;
    }
    if _skTemp20 {
      let _skTemp28 = takes_float4_bf4(vec4<f32>(4.0));
      _skTemp19 = _skTemp28;
    } else {
      _skTemp19 = false;
    }
    if _skTemp19 {
      let _skTemp29 = takes_float2x2_bf22(mat2x2<f32>(2.0, 0.0, 0.0, 2.0));
      _skTemp18 = _skTemp29;
    } else {
      _skTemp18 = false;
    }
    if _skTemp18 {
      let _skTemp30 = takes_float3x3_bf33(mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0));
      _skTemp17 = _skTemp30;
    } else {
      _skTemp17 = false;
    }
    if _skTemp17 {
      let _skTemp31 = takes_float4x4_bf44(mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0));
      _skTemp16 = _skTemp31;
    } else {
      _skTemp16 = false;
    }
    if _skTemp16 {
      let _skTemp32 = takes_half_bh(1.0);
      _skTemp15 = _skTemp32;
    } else {
      _skTemp15 = false;
    }
    if _skTemp15 {
      let _skTemp33 = takes_half2_bh2(vec2<f32>(2.0));
      _skTemp14 = _skTemp33;
    } else {
      _skTemp14 = false;
    }
    if _skTemp14 {
      let _skTemp34 = takes_half3_bh3(vec3<f32>(3.0));
      _skTemp13 = _skTemp34;
    } else {
      _skTemp13 = false;
    }
    if _skTemp13 {
      let _skTemp35 = takes_half4_bh4(vec4<f32>(4.0));
      _skTemp12 = _skTemp35;
    } else {
      _skTemp12 = false;
    }
    if _skTemp12 {
      let _skTemp36 = takes_half2x2_bh22(mat2x2<f32>(2.0, 0.0, 0.0, 2.0));
      _skTemp11 = _skTemp36;
    } else {
      _skTemp11 = false;
    }
    if _skTemp11 {
      let _skTemp37 = takes_half3x3_bh33(mat3x3<f32>(3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0));
      _skTemp10 = _skTemp37;
    } else {
      _skTemp10 = false;
    }
    if _skTemp10 {
      let _skTemp38 = takes_half4x4_bh44(mat4x4<f32>(4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0));
      _skTemp9 = _skTemp38;
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      let _skTemp39 = takes_bool_bb(true);
      _skTemp8 = _skTemp39;
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      let _skTemp40 = takes_bool2_bb2(vec2<bool>(true));
      _skTemp7 = _skTemp40;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp41 = takes_bool3_bb3(vec3<bool>(true));
      _skTemp6 = _skTemp41;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp42 = takes_bool4_bb4(vec4<bool>(true));
      _skTemp5 = _skTemp42;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp43 = takes_int_bi(1);
      _skTemp4 = _skTemp43;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp44 = takes_int2_bi2(vec2<i32>(2));
      _skTemp3 = _skTemp44;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      let _skTemp45 = takes_int3_bi3(vec3<i32>(3));
      _skTemp2 = _skTemp45;
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp46 = takes_int4_bi4(vec4<i32>(4));
      _skTemp1 = _skTemp46;
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
