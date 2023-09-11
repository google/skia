diagnostic(off, derivative_uniformity);
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_matrix_b() -> bool {
  {
    var ok: bool = true;
    var num: f32 = 0.0;
    var _skTemp0: bool;
    if ok {
      num = num + f32(1);
      _skTemp0 = all(mat2x2<f32>(1.0, 2.0, 3.0, num)[0] == vec2<f32>(1.0, 2.0));
    } else {
      _skTemp0 = false;
    }
    ok = _skTemp0;
    var _skTemp1: bool;
    if ok {
      num = num + f32(1);
      let _skTemp2 = vec2<f32>(num);
      _skTemp1 = all(mat2x2<f32>(_skTemp2[0], _skTemp2[1], 3.0, 4.0)[1] == vec2<f32>(3.0, 4.0));
    } else {
      _skTemp1 = false;
    }
    ok = _skTemp1;
    var _skTemp3: bool;
    if ok {
      num = num + f32(1);
      let _skTemp4 = vec3<f32>(num);
      _skTemp3 = all(mat3x3<f32>(vec3<f32>(1.0)[0], vec3<f32>(1.0)[1], vec3<f32>(1.0)[2], _skTemp4[0], _skTemp4[1], _skTemp4[2], vec3<f32>(0.0)[0], vec3<f32>(0.0)[1], vec3<f32>(0.0)[2])[0] == vec3<f32>(1.0));
    } else {
      _skTemp3 = false;
    }
    ok = _skTemp3;
    var _skTemp5: bool;
    if ok {
      num = num + f32(1);
      let _skTemp6 = vec3<f32>(num);
      _skTemp5 = all(mat3x3<f32>(vec3<f32>(1.0)[0], vec3<f32>(1.0)[1], vec3<f32>(1.0)[2], _skTemp6[0], _skTemp6[1], _skTemp6[2], vec3<f32>(0.0)[0], vec3<f32>(0.0)[1], vec3<f32>(0.0)[2])[2] == vec3<f32>(0.0));
    } else {
      _skTemp5 = false;
    }
    ok = _skTemp5;
    var _skTemp7: bool;
    if ok {
      num = num + f32(1);
      let _skTemp8 = vec3<f32>(num);
      _skTemp7 = all(mat3x3<f32>(_skTemp8[0], _skTemp8[1], _skTemp8[2], vec3<f32>(1.0)[0], vec3<f32>(1.0)[1], vec3<f32>(1.0)[2], vec3<f32>(0.0)[0], vec3<f32>(0.0)[1], vec3<f32>(0.0)[2])[1] == vec3<f32>(1.0));
    } else {
      _skTemp7 = false;
    }
    ok = _skTemp7;
    var _skTemp9: bool;
    if ok {
      num = num + f32(1);
      _skTemp9 = all(mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, num, 7.0, 8.0, 9.0)[0] == vec3<f32>(1.0, 2.0, 3.0));
    } else {
      _skTemp9 = false;
    }
    ok = _skTemp9;
    var _skTemp10: bool;
    if ok {
      let _skTemp11 = num;
      num = num + f32(1);
      _skTemp10 = all(mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, _skTemp11, 8.0, 9.0)[1] == vec3<f32>(4.0, 5.0, 6.0));
    } else {
      _skTemp10 = false;
    }
    ok = _skTemp10;
    var _skTemp12: bool;
    if ok {
      num = num + f32(1);
      let _skTemp13 = vec4<f32>(num);
      _skTemp12 = all(mat4x4<f32>(_skTemp13[0], _skTemp13[1], _skTemp13[2], _skTemp13[3], vec4<f32>(1.0)[0], vec4<f32>(1.0)[1], vec4<f32>(1.0)[2], vec4<f32>(1.0)[3], vec4<f32>(2.0)[0], vec4<f32>(2.0)[1], vec4<f32>(2.0)[2], vec4<f32>(2.0)[3], vec4<f32>(3.0)[0], vec4<f32>(3.0)[1], vec4<f32>(3.0)[2], vec4<f32>(3.0)[3])[1] == vec4<f32>(1.0));
    } else {
      _skTemp12 = false;
    }
    ok = _skTemp12;
    var _skTemp14: bool;
    if ok {
      num = num + f32(1);
      let _skTemp15 = vec4<f32>(num);
      _skTemp14 = all(mat4x4<f32>(vec4<f32>(1.0)[0], vec4<f32>(1.0)[1], vec4<f32>(1.0)[2], vec4<f32>(1.0)[3], _skTemp15[0], _skTemp15[1], _skTemp15[2], _skTemp15[3], vec4<f32>(2.0)[0], vec4<f32>(2.0)[1], vec4<f32>(2.0)[2], vec4<f32>(2.0)[3], vec4<f32>(3.0)[0], vec4<f32>(3.0)[1], vec4<f32>(3.0)[2], vec4<f32>(3.0)[3])[2] == vec4<f32>(2.0));
    } else {
      _skTemp14 = false;
    }
    ok = _skTemp14;
    var _skTemp16: bool;
    if ok {
      num = num + f32(1);
      let _skTemp17 = vec4<f32>(num);
      _skTemp16 = all(mat4x4<f32>(vec4<f32>(1.0)[0], vec4<f32>(1.0)[1], vec4<f32>(1.0)[2], vec4<f32>(1.0)[3], vec4<f32>(1.0)[0], vec4<f32>(1.0)[1], vec4<f32>(1.0)[2], vec4<f32>(1.0)[3], _skTemp17[0], _skTemp17[1], _skTemp17[2], _skTemp17[3], vec4<f32>(3.0)[0], vec4<f32>(3.0)[1], vec4<f32>(3.0)[2], vec4<f32>(3.0)[3])[3] == vec4<f32>(3.0));
    } else {
      _skTemp16 = false;
    }
    ok = _skTemp16;
    var _skTemp18: bool;
    if ok {
      num = num + f32(1);
      _skTemp18 = all(mat4x4<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, num, 16.0)[3].xy == vec2<f32>(13.0, 14.0));
    } else {
      _skTemp18 = false;
    }
    ok = _skTemp18;
    return ok && (num == 11.0);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_ok: bool = true;
    var _1_num: f32 = 0.0;
    var _skTemp19: bool;
    if _0_ok {
      _1_num = _1_num + f32(1);
      _skTemp19 = (vec2<f32>(_1_num, 0.0).y == 0.0);
    } else {
      _skTemp19 = false;
    }
    _0_ok = _skTemp19;
    var _skTemp20: bool;
    if _0_ok {
      _1_num = _1_num + f32(1);
      _skTemp20 = (vec2<f32>(0.0, _1_num).x == 0.0);
    } else {
      _skTemp20 = false;
    }
    _0_ok = _skTemp20;
    var _skTemp21: bool;
    if _0_ok {
      _1_num = _1_num + f32(1);
      _skTemp21 = all(vec3<f32>(_1_num, 1.0, 0.0).yz == vec2<f32>(1.0, 0.0));
    } else {
      _skTemp21 = false;
    }
    _0_ok = _skTemp21;
    var _skTemp22: bool;
    if _0_ok {
      _1_num = _1_num + f32(1);
      _skTemp22 = all(vec3<f32>(1.0, 0.0, _1_num).xy == vec2<f32>(1.0, 0.0));
    } else {
      _skTemp22 = false;
    }
    _0_ok = _skTemp22;
    var _skTemp23: bool;
    if _0_ok {
      _1_num = _1_num + f32(1);
      _skTemp23 = all(vec3<f32>(_1_num, 1.0, 0.0).yz == vec2<f32>(1.0, 0.0));
    } else {
      _skTemp23 = false;
    }
    _0_ok = _skTemp23;
    var _skTemp24: bool;
    if _0_ok {
      _1_num = _1_num + f32(1);
      _skTemp24 = all(vec4<f32>(_1_num, 1.0, 0.0, 0.0).yzw == vec3<f32>(1.0, 0.0, 0.0));
    } else {
      _skTemp24 = false;
    }
    _0_ok = _skTemp24;
    var _skTemp25: bool;
    if _0_ok {
      _1_num = _1_num + f32(1);
      _skTemp25 = (vec4<f32>(1.0, _1_num, 1.0, 0.0).x == 1.0);
    } else {
      _skTemp25 = false;
    }
    _0_ok = _skTemp25;
    var _skTemp26: bool;
    if _0_ok {
      _1_num = _1_num + f32(1);
      _skTemp26 = (vec4<f32>(1.0, 0.0, _1_num, 1.0).w == 1.0);
    } else {
      _skTemp26 = false;
    }
    _0_ok = _skTemp26;
    var _skTemp27: bool;
    if _0_ok {
      _1_num = _1_num + f32(1);
      _skTemp27 = all(vec4<f32>(1.0, 0.0, 1.0, _1_num).xyz == vec3<f32>(1.0, 0.0, 1.0));
    } else {
      _skTemp27 = false;
    }
    _0_ok = _skTemp27;
    var _skTemp28: vec4<f32>;
    var _skTemp29: bool;
    if _0_ok && (_1_num == 9.0) {
      let _skTemp30 = test_matrix_b();
      _skTemp29 = _skTemp30;
    } else {
      _skTemp29 = false;
    }
    if _skTemp29 {
      _skTemp28 = _globalUniforms.colorGreen;
    } else {
      _skTemp28 = _globalUniforms.colorRed;
    }
    return _skTemp28;
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
