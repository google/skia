### Compilation failed:

error: :28:13 error: no matching overload for operator / (mat3x2<f32>, mat3x2<f32>)

4 candidate operators:
  operator / (T, T) -> T  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator / (vecN<T>, T) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator / (T, vecN<T>) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator / (vecN<T>, vecN<T>) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32, u32 or f16

      m = m / splat_4;
            ^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_matrix_op_matrix_half_b() -> bool {
  {
    var ok: bool = true;
    {
      const splat_4: mat3x2<f32> = mat3x2<f32>(4.0, 4.0, 4.0, 4.0, 4.0, 4.0);
      var m: mat3x2<f32> = mat3x2<f32>(2.0, 0.0, 0.0, 2.0, 0.0, 0.0);
      m = m + splat_4;
      let _skTemp0 = mat3x2<f32>(6.0, 4.0, 4.0, 6.0, 4.0, 4.0);
      ok = ok && (all(m[0] == _skTemp0[0]) && all(m[1] == _skTemp0[1]) && all(m[2] == _skTemp0[2]));
      m = mat3x2<f32>(2.0, 0.0, 0.0, 2.0, 0.0, 0.0);
      m = m - splat_4;
      let _skTemp1 = mat3x2<f32>(-2.0, -4.0, -4.0, -2.0, -4.0, -4.0);
      ok = ok && (all(m[0] == _skTemp1[0]) && all(m[1] == _skTemp1[1]) && all(m[2] == _skTemp1[2]));
      m = mat3x2<f32>(2.0, 0.0, 0.0, 2.0, 0.0, 0.0);
      m = m / splat_4;
      let _skTemp2 = mat3x2<f32>(0.5, 0.0, 0.0, 0.5, 0.0, 0.0);
      ok = ok && (all(m[0] == _skTemp2[0]) && all(m[1] == _skTemp2[1]) && all(m[2] == _skTemp2[2]));
    }
    {
      const splat_4: mat2x3<f32> = mat2x3<f32>(4.0, 4.0, 4.0, 4.0, 4.0, 4.0);
      var m: mat2x3<f32> = splat_4;
      m = m + mat2x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0);
      let _skTemp3 = mat2x3<f32>(6.0, 4.0, 4.0, 4.0, 6.0, 4.0);
      ok = ok && (all(m[0] == _skTemp3[0]) && all(m[1] == _skTemp3[1]));
      m = splat_4;
      m = m - mat2x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0);
      let _skTemp4 = mat2x3<f32>(2.0, 4.0, 4.0, 4.0, 2.0, 4.0);
      ok = ok && (all(m[0] == _skTemp4[0]) && all(m[1] == _skTemp4[1]));
      m = splat_4;
      m = m / mat2x3<f32>(2.0, 2.0, 2.0, 2.0, 2.0, 2.0);
      let _skTemp5 = mat2x3<f32>(2.0, 2.0, 2.0, 2.0, 2.0, 2.0);
      ok = ok && (all(m[0] == _skTemp5[0]) && all(m[1] == _skTemp5[1]));
    }
    {
      var m: mat4x3<f32> = mat4x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0);
      m = m + mat4x3<f32>(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0);
      let _skTemp6 = mat4x3<f32>(17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0);
      ok = ok && (all(m[0] == _skTemp6[0]) && all(m[1] == _skTemp6[1]) && all(m[2] == _skTemp6[2]) && all(m[3] == _skTemp6[3]));
    }
    {
      var m: mat4x2<f32> = mat4x2<f32>(10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0);
      m = m - mat4x2<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
      let _skTemp7 = mat4x2<f32>(9.0, 18.0, 27.0, 36.0, 45.0, 54.0, 63.0, 72.0);
      ok = ok && (all(m[0] == _skTemp7[0]) && all(m[1] == _skTemp7[1]) && all(m[2] == _skTemp7[2]) && all(m[3] == _skTemp7[3]));
    }
    {
      var m: mat2x4<f32> = mat2x4<f32>(10.0, 20.0, 30.0, 40.0, 10.0, 20.0, 30.0, 40.0);
      m = m / mat2x4<f32>(10.0, 10.0, 10.0, 10.0, 5.0, 5.0, 5.0, 5.0);
      let _skTemp8 = mat2x4<f32>(1.0, 2.0, 3.0, 4.0, 2.0, 4.0, 6.0, 8.0);
      ok = ok && (all(m[0] == _skTemp8[0]) && all(m[1] == _skTemp8[1]));
    }
    {
      var m: mat2x3<f32> = mat2x3<f32>(7.0, 9.0, 11.0, 8.0, 10.0, 12.0);
      m = m * mat2x2<f32>(1.0, 4.0, 2.0, 5.0);
      let _skTemp9 = mat2x3<f32>(39.0, 49.0, 59.0, 54.0, 68.0, 82.0);
      ok = ok && (all(m[0] == _skTemp9[0]) && all(m[1] == _skTemp9[1]));
    }
    return ok;
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var _0_ok: bool = true;
    {
      const _1_splat_4: mat3x2<f32> = mat3x2<f32>(4.0, 4.0, 4.0, 4.0, 4.0, 4.0);
      var _2_m: mat3x2<f32> = mat3x2<f32>(2.0, 0.0, 0.0, 2.0, 0.0, 0.0);
      _2_m = _2_m + _1_splat_4;
      let _skTemp10 = mat3x2<f32>(6.0, 4.0, 4.0, 6.0, 4.0, 4.0);
      _0_ok = _0_ok && (all(_2_m[0] == _skTemp10[0]) && all(_2_m[1] == _skTemp10[1]) && all(_2_m[2] == _skTemp10[2]));
      _2_m = mat3x2<f32>(2.0, 0.0, 0.0, 2.0, 0.0, 0.0);
      _2_m = _2_m - _1_splat_4;
      let _skTemp11 = mat3x2<f32>(-2.0, -4.0, -4.0, -2.0, -4.0, -4.0);
      _0_ok = _0_ok && (all(_2_m[0] == _skTemp11[0]) && all(_2_m[1] == _skTemp11[1]) && all(_2_m[2] == _skTemp11[2]));
      _2_m = mat3x2<f32>(2.0, 0.0, 0.0, 2.0, 0.0, 0.0);
      _2_m = _2_m / _1_splat_4;
      let _skTemp12 = mat3x2<f32>(0.5, 0.0, 0.0, 0.5, 0.0, 0.0);
      _0_ok = _0_ok && (all(_2_m[0] == _skTemp12[0]) && all(_2_m[1] == _skTemp12[1]) && all(_2_m[2] == _skTemp12[2]));
    }
    {
      const _3_splat_4: mat2x3<f32> = mat2x3<f32>(4.0, 4.0, 4.0, 4.0, 4.0, 4.0);
      var _4_m: mat2x3<f32> = _3_splat_4;
      _4_m = _4_m + mat2x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0);
      let _skTemp13 = mat2x3<f32>(6.0, 4.0, 4.0, 4.0, 6.0, 4.0);
      _0_ok = _0_ok && (all(_4_m[0] == _skTemp13[0]) && all(_4_m[1] == _skTemp13[1]));
      _4_m = _3_splat_4;
      _4_m = _4_m - mat2x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0);
      let _skTemp14 = mat2x3<f32>(2.0, 4.0, 4.0, 4.0, 2.0, 4.0);
      _0_ok = _0_ok && (all(_4_m[0] == _skTemp14[0]) && all(_4_m[1] == _skTemp14[1]));
      _4_m = _3_splat_4;
      _4_m = _4_m / mat2x3<f32>(2.0, 2.0, 2.0, 2.0, 2.0, 2.0);
      let _skTemp15 = mat2x3<f32>(2.0, 2.0, 2.0, 2.0, 2.0, 2.0);
      _0_ok = _0_ok && (all(_4_m[0] == _skTemp15[0]) && all(_4_m[1] == _skTemp15[1]));
    }
    {
      var _5_m: mat4x3<f32> = mat4x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0);
      _5_m = _5_m + mat4x3<f32>(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0);
      let _skTemp16 = mat4x3<f32>(17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0);
      _0_ok = _0_ok && (all(_5_m[0] == _skTemp16[0]) && all(_5_m[1] == _skTemp16[1]) && all(_5_m[2] == _skTemp16[2]) && all(_5_m[3] == _skTemp16[3]));
    }
    {
      var _6_m: mat4x2<f32> = mat4x2<f32>(10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0);
      _6_m = _6_m - mat4x2<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
      let _skTemp17 = mat4x2<f32>(9.0, 18.0, 27.0, 36.0, 45.0, 54.0, 63.0, 72.0);
      _0_ok = _0_ok && (all(_6_m[0] == _skTemp17[0]) && all(_6_m[1] == _skTemp17[1]) && all(_6_m[2] == _skTemp17[2]) && all(_6_m[3] == _skTemp17[3]));
    }
    {
      var _7_m: mat2x4<f32> = mat2x4<f32>(10.0, 20.0, 30.0, 40.0, 10.0, 20.0, 30.0, 40.0);
      _7_m = _7_m / mat2x4<f32>(10.0, 10.0, 10.0, 10.0, 5.0, 5.0, 5.0, 5.0);
      let _skTemp18 = mat2x4<f32>(1.0, 2.0, 3.0, 4.0, 2.0, 4.0, 6.0, 8.0);
      _0_ok = _0_ok && (all(_7_m[0] == _skTemp18[0]) && all(_7_m[1] == _skTemp18[1]));
    }
    {
      var _8_m: mat2x3<f32> = mat2x3<f32>(7.0, 9.0, 11.0, 8.0, 10.0, 12.0);
      _8_m = _8_m * mat2x2<f32>(1.0, 4.0, 2.0, 5.0);
      let _skTemp19 = mat2x3<f32>(39.0, 49.0, 59.0, 54.0, 68.0, 82.0);
      _0_ok = _0_ok && (all(_8_m[0] == _skTemp19[0]) && all(_8_m[1] == _skTemp19[1]));
    }
    var _skTemp20: vec4<f32>;
    var _skTemp21: bool;
    if _0_ok {
      let _skTemp22 = test_matrix_op_matrix_half_b();
      _skTemp21 = _skTemp22;
    } else {
      _skTemp21 = false;
    }
    if _skTemp21 {
      _skTemp20 = _globalUniforms.colorGreen;
    } else {
      _skTemp20 = _globalUniforms.colorRed;
    }
    return _skTemp20;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}

1 error
