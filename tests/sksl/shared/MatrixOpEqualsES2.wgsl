### Compilation failed:

error: :29:13 error: no matching overload for operator / (mat3x3<f32>, mat3x3<f32>)

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
      const splat_4: mat3x3<f32> = mat3x3<f32>(4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0);
      const splat_2: mat3x3<f32> = mat3x3<f32>(2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0);
      var m: mat3x3<f32> = mat3x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
      m = m + splat_4;
      let _skTemp0 = mat3x3<f32>(6.0, 4.0, 4.0, 4.0, 6.0, 4.0, 4.0, 4.0, 6.0);
      ok = ok && (all(m[0] == _skTemp0[0]) && all(m[1] == _skTemp0[1]) && all(m[2] == _skTemp0[2]));
      m = mat3x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
      m = m - splat_4;
      let _skTemp1 = mat3x3<f32>(-2.0, -4.0, -4.0, -4.0, -2.0, -4.0, -4.0, -4.0, -2.0);
      ok = ok && (all(m[0] == _skTemp1[0]) && all(m[1] == _skTemp1[1]) && all(m[2] == _skTemp1[2]));
      m = mat3x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
      m = m / splat_4;
      let _skTemp2 = mat3x3<f32>(0.5, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5);
      ok = ok && (all(m[0] == _skTemp2[0]) && all(m[1] == _skTemp2[1]) && all(m[2] == _skTemp2[2]));
      m = splat_4;
      m = m + mat3x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
      let _skTemp3 = mat3x3<f32>(6.0, 4.0, 4.0, 4.0, 6.0, 4.0, 4.0, 4.0, 6.0);
      ok = ok && (all(m[0] == _skTemp3[0]) && all(m[1] == _skTemp3[1]) && all(m[2] == _skTemp3[2]));
      m = splat_4;
      m = m - mat3x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
      let _skTemp4 = mat3x3<f32>(2.0, 4.0, 4.0, 4.0, 2.0, 4.0, 4.0, 4.0, 2.0);
      ok = ok && (all(m[0] == _skTemp4[0]) && all(m[1] == _skTemp4[1]) && all(m[2] == _skTemp4[2]));
      m = splat_4;
      m = m / splat_2;
      let _skTemp5 = mat3x3<f32>(2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0);
      ok = ok && (all(m[0] == _skTemp5[0]) && all(m[1] == _skTemp5[1]) && all(m[2] == _skTemp5[2]));
    }
    {
      var m: mat4x4<f32> = mat4x4<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
      m = m + mat4x4<f32>(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
      let _skTemp6 = mat4x4<f32>(17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0);
      ok = ok && (all(m[0] == _skTemp6[0]) && all(m[1] == _skTemp6[1]) && all(m[2] == _skTemp6[2]) && all(m[3] == _skTemp6[3]));
    }
    {
      var m: mat2x2<f32> = mat2x2<f32>(10.0, 20.0, 30.0, 40.0);
      m = m - mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
      let _skTemp7 = mat2x2<f32>(9.0, 18.0, 27.0, 36.0);
      ok = ok && (all(m[0] == _skTemp7[0]) && all(m[1] == _skTemp7[1]));
    }
    {
      var m: mat2x2<f32> = mat2x2<f32>(2.0, 4.0, 6.0, 8.0);
      m = m / mat2x2<f32>(2.0, 2.0, 2.0, 4.0);
      let _skTemp8 = mat2x2<f32>(1.0, 2.0, 3.0, 2.0);
      ok = ok && (all(m[0] == _skTemp8[0]) && all(m[1] == _skTemp8[1]));
    }
    {
      var m: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 7.0, 4.0);
      m = m * mat2x2<f32>(3.0, 5.0, 3.0, 2.0);
      let _skTemp9 = mat2x2<f32>(38.0, 26.0, 17.0, 14.0);
      ok = ok && (all(m[0] == _skTemp9[0]) && all(m[1] == _skTemp9[1]));
    }
    {
      var m: mat3x3<f32> = mat3x3<f32>(10.0, 4.0, 2.0, 20.0, 5.0, 3.0, 10.0, 6.0, 5.0);
      m = m * mat3x3<f32>(3.0, 3.0, 4.0, 2.0, 3.0, 4.0, 4.0, 9.0, 2.0);
      let _skTemp10 = mat3x3<f32>(130.0, 51.0, 35.0, 120.0, 47.0, 33.0, 240.0, 73.0, 45.0);
      ok = ok && (all(m[0] == _skTemp10[0]) && all(m[1] == _skTemp10[1]) && all(m[2] == _skTemp10[2]));
    }
    return ok;
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var _0_ok: bool = true;
    {
      const _1_splat_4: mat3x3<f32> = mat3x3<f32>(4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0);
      const _2_splat_2: mat3x3<f32> = mat3x3<f32>(2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0);
      var _3_m: mat3x3<f32> = mat3x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
      _3_m = _3_m + _1_splat_4;
      let _skTemp11 = mat3x3<f32>(6.0, 4.0, 4.0, 4.0, 6.0, 4.0, 4.0, 4.0, 6.0);
      _0_ok = _0_ok && (all(_3_m[0] == _skTemp11[0]) && all(_3_m[1] == _skTemp11[1]) && all(_3_m[2] == _skTemp11[2]));
      _3_m = mat3x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
      _3_m = _3_m - _1_splat_4;
      let _skTemp12 = mat3x3<f32>(-2.0, -4.0, -4.0, -4.0, -2.0, -4.0, -4.0, -4.0, -2.0);
      _0_ok = _0_ok && (all(_3_m[0] == _skTemp12[0]) && all(_3_m[1] == _skTemp12[1]) && all(_3_m[2] == _skTemp12[2]));
      _3_m = mat3x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
      _3_m = _3_m / _1_splat_4;
      let _skTemp13 = mat3x3<f32>(0.5, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5);
      _0_ok = _0_ok && (all(_3_m[0] == _skTemp13[0]) && all(_3_m[1] == _skTemp13[1]) && all(_3_m[2] == _skTemp13[2]));
      _3_m = _1_splat_4;
      _3_m = _3_m + mat3x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
      let _skTemp14 = mat3x3<f32>(6.0, 4.0, 4.0, 4.0, 6.0, 4.0, 4.0, 4.0, 6.0);
      _0_ok = _0_ok && (all(_3_m[0] == _skTemp14[0]) && all(_3_m[1] == _skTemp14[1]) && all(_3_m[2] == _skTemp14[2]));
      _3_m = _1_splat_4;
      _3_m = _3_m - mat3x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
      let _skTemp15 = mat3x3<f32>(2.0, 4.0, 4.0, 4.0, 2.0, 4.0, 4.0, 4.0, 2.0);
      _0_ok = _0_ok && (all(_3_m[0] == _skTemp15[0]) && all(_3_m[1] == _skTemp15[1]) && all(_3_m[2] == _skTemp15[2]));
      _3_m = _1_splat_4;
      _3_m = _3_m / _2_splat_2;
      let _skTemp16 = mat3x3<f32>(2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0);
      _0_ok = _0_ok && (all(_3_m[0] == _skTemp16[0]) && all(_3_m[1] == _skTemp16[1]) && all(_3_m[2] == _skTemp16[2]));
    }
    {
      var _4_m: mat4x4<f32> = mat4x4<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
      _4_m = _4_m + mat4x4<f32>(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
      let _skTemp17 = mat4x4<f32>(17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0, 17.0);
      _0_ok = _0_ok && (all(_4_m[0] == _skTemp17[0]) && all(_4_m[1] == _skTemp17[1]) && all(_4_m[2] == _skTemp17[2]) && all(_4_m[3] == _skTemp17[3]));
    }
    {
      var _5_m: mat2x2<f32> = mat2x2<f32>(10.0, 20.0, 30.0, 40.0);
      _5_m = _5_m - mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
      let _skTemp18 = mat2x2<f32>(9.0, 18.0, 27.0, 36.0);
      _0_ok = _0_ok && (all(_5_m[0] == _skTemp18[0]) && all(_5_m[1] == _skTemp18[1]));
    }
    {
      var _6_m: mat2x2<f32> = mat2x2<f32>(2.0, 4.0, 6.0, 8.0);
      _6_m = _6_m / mat2x2<f32>(2.0, 2.0, 2.0, 4.0);
      let _skTemp19 = mat2x2<f32>(1.0, 2.0, 3.0, 2.0);
      _0_ok = _0_ok && (all(_6_m[0] == _skTemp19[0]) && all(_6_m[1] == _skTemp19[1]));
    }
    {
      var _7_m: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 7.0, 4.0);
      _7_m = _7_m * mat2x2<f32>(3.0, 5.0, 3.0, 2.0);
      let _skTemp20 = mat2x2<f32>(38.0, 26.0, 17.0, 14.0);
      _0_ok = _0_ok && (all(_7_m[0] == _skTemp20[0]) && all(_7_m[1] == _skTemp20[1]));
    }
    {
      var _8_m: mat3x3<f32> = mat3x3<f32>(10.0, 4.0, 2.0, 20.0, 5.0, 3.0, 10.0, 6.0, 5.0);
      _8_m = _8_m * mat3x3<f32>(3.0, 3.0, 4.0, 2.0, 3.0, 4.0, 4.0, 9.0, 2.0);
      let _skTemp21 = mat3x3<f32>(130.0, 51.0, 35.0, 120.0, 47.0, 33.0, 240.0, 73.0, 45.0);
      _0_ok = _0_ok && (all(_8_m[0] == _skTemp21[0]) && all(_8_m[1] == _skTemp21[1]) && all(_8_m[2] == _skTemp21[2]));
    }
    var _skTemp22: vec4<f32>;
    var _skTemp23: bool;
    if _0_ok {
      let _skTemp24 = test_matrix_op_matrix_half_b();
      _skTemp23 = _skTemp24;
    } else {
      _skTemp23 = false;
    }
    if _skTemp23 {
      _skTemp22 = _globalUniforms.colorGreen;
    } else {
      _skTemp22 = _globalUniforms.colorRed;
    }
    return _skTemp22;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}

1 error
