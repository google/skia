diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorWhite: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testMatrix2x2: _skMatrix22,
  testMatrix3x3: mat3x3<f32>,
  testMatrix4x4: mat4x4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
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
    const negated: mat2x2<f32> = mat2x2<f32>(-1.0, -2.0, -3.0, -4.0);
    var x: mat2x2<f32> = _skUnpacked__globalUniforms_testMatrix2x2;
    x = (-1.0 * x);
    return (all(x[0] == negated[0]) && all(x[1] == negated[1]));
  }
}
fn test_mat3_b() -> bool {
  {
    const negated: mat3x3<f32> = mat3x3<f32>(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0);
    var x: mat3x3<f32> = _globalUniforms.testMatrix3x3;
    x = (-1.0 * x);
    return (all(x[0] == negated[0]) && all(x[1] == negated[1]) && all(x[2] == negated[2]));
  }
}
fn test_mat4_b() -> bool {
  {
    const negated: mat4x4<f32> = mat4x4<f32>(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0, -10.0, -11.0, -12.0, -13.0, -14.0, -15.0, -16.0);
    var x: mat4x4<f32> = _globalUniforms.testMatrix4x4;
    x = (-1.0 * x);
    return (all(x[0] == negated[0]) && all(x[1] == negated[1]) && all(x[2] == negated[2]) && all(x[3] == negated[3]));
  }
}
fn test_hmat2_b() -> bool {
  {
    const negated: mat2x2<f32> = mat2x2<f32>(-1.0, -2.0, -3.0, -4.0);
    var x: mat2x2<f32> = mat2x2<f32>(_skUnpacked__globalUniforms_testMatrix2x2);
    x = (-1.0 * x);
    return (all(x[0] == negated[0]) && all(x[1] == negated[1]));
  }
}
fn test_hmat3_b() -> bool {
  {
    const negated: mat3x3<f32> = mat3x3<f32>(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0);
    var x: mat3x3<f32> = mat3x3<f32>(_globalUniforms.testMatrix3x3);
    x = (-1.0 * x);
    return (all(x[0] == negated[0]) && all(x[1] == negated[1]) && all(x[2] == negated[2]));
  }
}
fn test_hmat4_b() -> bool {
  {
    const negated: mat4x4<f32> = mat4x4<f32>(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0, -10.0, -11.0, -12.0, -13.0, -14.0, -15.0, -16.0);
    var x: mat4x4<f32> = mat4x4<f32>(_globalUniforms.testMatrix4x4);
    x = (-1.0 * x);
    return (all(x[0] == negated[0]) && all(x[1] == negated[1]) && all(x[2] == negated[2]) && all(x[3] == negated[3]));
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_x: f32 = f32(_globalUniforms.colorWhite.x);
    _0_x = -_0_x;
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
    if _0_x == -1.0 {
      _skTemp9 = test_iscalar_b();
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      _skTemp8 = test_fvec_b();
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      _skTemp7 = test_ivec_b();
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      _skTemp6 = test_mat2_b();
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      _skTemp5 = test_mat3_b();
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      _skTemp4 = test_mat4_b();
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      _skTemp3 = test_hmat2_b();
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = test_hmat3_b();
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      _skTemp1 = test_hmat4_b();
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
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
struct _skRow2 {
  @align(16) r : vec2<f32>
};
struct _skMatrix22 {
  c : array<_skRow2, 2>
};
var<private> _skUnpacked__globalUniforms_testMatrix2x2: mat2x2<f32>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_testMatrix2x2 = mat2x2<f32>(_globalUniforms.testMatrix2x2.c[0].r, _globalUniforms.testMatrix2x2.c[1].r);
}
