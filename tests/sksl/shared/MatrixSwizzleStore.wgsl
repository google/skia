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
  testMatrix3x3: mat3x3<f32>,
  testMatrix4x4: mat4x4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test4x4_b() -> bool {
  {
    var matrix: mat4x4<f32>;
    var values: vec4<f32> = vec4<f32>(4.0, 3.0, 2.0, 1.0);
    {
      var index: i32 = 0;
      loop {
        {
          matrix[index] = vec4<f32>((values.xw), matrix[index].yz).yzwx;
          matrix[index] = vec4<f32>((values.yz), matrix[index].xw).zyxw;
          values = values + 4.0;
        }
        continuing {
          index = index + i32(1);
          break if index >= 4;
        }
      }
    }
    return (all(matrix[0] == _globalUniforms.testMatrix4x4[0]) && all(matrix[1] == _globalUniforms.testMatrix4x4[1]) && all(matrix[2] == _globalUniforms.testMatrix4x4[2]) && all(matrix[3] == _globalUniforms.testMatrix4x4[3]));
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_matrix: mat3x3<f32>;
    var _1_values: vec3<f32> = vec3<f32>(3.0, 2.0, 1.0);
    {
      var _2_index: i32 = 0;
      loop {
        {
          _0_matrix[_2_index] = vec3<f32>((_1_values.xz), _0_matrix[_2_index].y).yzx;
          _0_matrix[_2_index].y = _1_values.y;
          _1_values = _1_values + 3.0;
        }
        continuing {
          _2_index = _2_index + i32(1);
          break if _2_index >= 3;
        }
      }
    }
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    if (all(_0_matrix[0] == _globalUniforms.testMatrix3x3[0]) && all(_0_matrix[1] == _globalUniforms.testMatrix3x3[1]) && all(_0_matrix[2] == _globalUniforms.testMatrix3x3[2])) {
      let _skTemp2 = test4x4_b();
      _skTemp1 = _skTemp2;
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
