struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorWhite: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testMatrix2x2: mat2x2<f32>,
  testMatrix3x3: mat3x3<f32>,
  testMatrix4x4: mat4x4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_iscalar_b() -> bool {
  {
    var x: i32 = i32(_globalUniforms.colorWhite.x);
    x = -x;
    return x == -1;
  }
}
fn test_fvec_b() -> bool {
  {
    var x: vec2<f32> = _globalUniforms.colorWhite.xy;
    x = -x;
    return all(x == vec2<f32>(-1.0));
  }
}
fn test_ivec_b() -> bool {
  {
    var x: vec2<i32> = vec2<i32>(i32(_globalUniforms.colorWhite.x));
    x = -x;
    return all(x == vec2<i32>(-1));
  }
}
fn test_mat2_b() -> bool {
  {
    let negated: mat2x2<f32> = mat2x2<f32>(-1.0, -2.0, -3.0, -4.0);
    var x: mat2x2<f32> = _globalUniforms.testMatrix2x2;
    x = (-1.0 * x);
    let _skTemp0 = x;
    let _skTemp1 = negated;
    return (all(_skTemp0[0] == _skTemp1[0]) && all(_skTemp0[1] == _skTemp1[1]));
  }
}
fn test_mat3_b() -> bool {
  {
    let negated: mat3x3<f32> = mat3x3<f32>(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0);
    var x: mat3x3<f32> = _globalUniforms.testMatrix3x3;
    x = (-1.0 * x);
    let _skTemp2 = x;
    let _skTemp3 = negated;
    return (all(_skTemp2[0] == _skTemp3[0]) && all(_skTemp2[1] == _skTemp3[1]) && all(_skTemp2[2] == _skTemp3[2]));
  }
}
fn test_mat4_b() -> bool {
  {
    let negated: mat4x4<f32> = mat4x4<f32>(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0, -10.0, -11.0, -12.0, -13.0, -14.0, -15.0, -16.0);
    var x: mat4x4<f32> = _globalUniforms.testMatrix4x4;
    x = (-1.0 * x);
    let _skTemp4 = x;
    let _skTemp5 = negated;
    return (all(_skTemp4[0] == _skTemp5[0]) && all(_skTemp4[1] == _skTemp5[1]) && all(_skTemp4[2] == _skTemp5[2]) && all(_skTemp4[3] == _skTemp5[3]));
  }
}
fn test_hmat2_b() -> bool {
  {
    let negated: mat2x2<f32> = mat2x2<f32>(-1.0, -2.0, -3.0, -4.0);
    var x: mat2x2<f32> = mat2x2<f32>(_globalUniforms.testMatrix2x2);
    x = (-1.0 * x);
    let _skTemp6 = x;
    let _skTemp7 = negated;
    return (all(_skTemp6[0] == _skTemp7[0]) && all(_skTemp6[1] == _skTemp7[1]));
  }
}
fn test_hmat3_b() -> bool {
  {
    let negated: mat3x3<f32> = mat3x3<f32>(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0);
    var x: mat3x3<f32> = mat3x3<f32>(_globalUniforms.testMatrix3x3);
    x = (-1.0 * x);
    let _skTemp8 = x;
    let _skTemp9 = negated;
    return (all(_skTemp8[0] == _skTemp9[0]) && all(_skTemp8[1] == _skTemp9[1]) && all(_skTemp8[2] == _skTemp9[2]));
  }
}
fn test_hmat4_b() -> bool {
  {
    let negated: mat4x4<f32> = mat4x4<f32>(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0, -10.0, -11.0, -12.0, -13.0, -14.0, -15.0, -16.0);
    var x: mat4x4<f32> = mat4x4<f32>(_globalUniforms.testMatrix4x4);
    x = (-1.0 * x);
    let _skTemp10 = x;
    let _skTemp11 = negated;
    return (all(_skTemp10[0] == _skTemp11[0]) && all(_skTemp10[1] == _skTemp11[1]) && all(_skTemp10[2] == _skTemp11[2]) && all(_skTemp10[3] == _skTemp11[3]));
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var _0_x: f32 = f32(_globalUniforms.colorWhite.x);
    _0_x = -_0_x;
    var _skTemp12: vec4<f32>;
    var _skTemp13: bool;
    var _skTemp14: bool;
    var _skTemp15: bool;
    var _skTemp16: bool;
    var _skTemp17: bool;
    var _skTemp18: bool;
    var _skTemp19: bool;
    var _skTemp20: bool;
    var _skTemp21: bool;
    if _0_x == -1.0 {
      let _skTemp22 = test_iscalar_b();
      _skTemp21 = _skTemp22;
    } else {
      _skTemp21 = false;
    }
    if _skTemp21 {
      let _skTemp23 = test_fvec_b();
      _skTemp20 = _skTemp23;
    } else {
      _skTemp20 = false;
    }
    if _skTemp20 {
      let _skTemp24 = test_ivec_b();
      _skTemp19 = _skTemp24;
    } else {
      _skTemp19 = false;
    }
    if _skTemp19 {
      let _skTemp25 = test_mat2_b();
      _skTemp18 = _skTemp25;
    } else {
      _skTemp18 = false;
    }
    if _skTemp18 {
      let _skTemp26 = test_mat3_b();
      _skTemp17 = _skTemp26;
    } else {
      _skTemp17 = false;
    }
    if _skTemp17 {
      let _skTemp27 = test_mat4_b();
      _skTemp16 = _skTemp27;
    } else {
      _skTemp16 = false;
    }
    if _skTemp16 {
      let _skTemp28 = test_hmat2_b();
      _skTemp15 = _skTemp28;
    } else {
      _skTemp15 = false;
    }
    if _skTemp15 {
      let _skTemp29 = test_hmat3_b();
      _skTemp14 = _skTemp29;
    } else {
      _skTemp14 = false;
    }
    if _skTemp14 {
      let _skTemp30 = test_hmat4_b();
      _skTemp13 = _skTemp30;
    } else {
      _skTemp13 = false;
    }
    if _skTemp13 {
      _skTemp12 = _globalUniforms.colorGreen;
    } else {
      _skTemp12 = _globalUniforms.colorRed;
    }
    return _skTemp12;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
