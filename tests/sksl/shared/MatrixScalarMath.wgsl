diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testInputs: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
const minus: i32 = 2;
const star: i32 = 3;
const slash: i32 = 4;
fn test_bifffff22(op: i32, m11: f32, m12: f32, m21: f32, m22: f32, expected: mat2x2<f32>) -> bool {
  {
    let one: f32 = f32(_globalUniforms.colorRed.x);
    var m2: mat2x2<f32> = mat2x2<f32>(m11 * one, m12 * one, m21 * one, m22 * one);
    switch op {
      case 1 {
        m2 = mat2x2<f32>(1.0 + m2[0], 1.0 + m2[1]);
        break;
      }
      case 2 {
        m2 = mat2x2<f32>(m2[0] - 1.0, m2[1] - 1.0);
        break;
      }
      case 3 {
        m2 = m2 * 2.0;
        break;
      }
      case 4 {
        m2 = m2 * 0.5;
        break;
      }
      case default {}
    }
    return (((m2[0].x == expected[0].x) && (m2[0].y == expected[0].y)) && (m2[1].x == expected[1].x)) && (m2[1].y == expected[1].y);
  }
}
fn divisionTest_b() -> bool {
  {
    let ten: f32 = f32(_globalUniforms.colorRed.x * 10.0);
    let _skTemp0 = vec2<f32>(ten);
    let _skTemp1 = vec2<f32>(ten);
    var mat: mat2x2<f32> = mat2x2<f32>(_skTemp0[0], _skTemp0[1], _skTemp1[0], _skTemp1[1]);
    let div: mat2x2<f32> = mat * (1.0 / _globalUniforms.testInputs.x);
    mat = mat * (1.0 / _globalUniforms.testInputs.x);
    return all((abs(vec4<f32>(div[0], div[1]) + vec4<f32>(8.0)) < vec4<f32>(0.01))) && all((abs(vec4<f32>(mat[0], mat[1]) + vec4<f32>(8.0)) < vec4<f32>(0.01)));
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let f1: f32 = f32(_globalUniforms.colorGreen.y);
    let f2: f32 = f32(2.0 * _globalUniforms.colorGreen.y);
    let f3: f32 = f32(3.0 * _globalUniforms.colorGreen.y);
    let f4: f32 = f32(4.0 * _globalUniforms.colorGreen.y);
    let _0_expected: mat2x2<f32> = mat2x2<f32>(f1 + 1.0, f2 + 1.0, f3 + 1.0, f4 + 1.0);
    let _1_one: f32 = f32(_globalUniforms.colorRed.x);
    var _2_m2: mat2x2<f32> = mat2x2<f32>(f1 * _1_one, f2 * _1_one, f3 * _1_one, f4 * _1_one);
    {
      _2_m2 = mat2x2<f32>(1.0 + _2_m2[0], 1.0 + _2_m2[1]);
    }
    var _skTemp2: vec4<f32>;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    if (((_2_m2[0].x == _0_expected[0].x) && (_2_m2[0].y == _0_expected[0].y)) && (_2_m2[1].x == _0_expected[1].x)) && (_2_m2[1].y == _0_expected[1].y) {
      _skTemp6 = test_bifffff22(minus, f1, f2, f3, f4, mat2x2<f32>(f1 - 1.0, f2 - 1.0, f3 - 1.0, f4 - 1.0));
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      _skTemp5 = test_bifffff22(star, f1, f2, f3, f4, mat2x2<f32>(f1 * 2.0, f2 * 2.0, f3 * 2.0, f4 * 2.0));
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      _skTemp4 = test_bifffff22(slash, f1, f2, f3, f4, mat2x2<f32>(f1 * 0.5, f2 * 0.5, f3 * 0.5, f4 * 0.5));
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      _skTemp3 = divisionTest_b();
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = _globalUniforms.colorGreen;
    } else {
      _skTemp2 = _globalUniforms.colorRed;
    }
    return _skTemp2;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
