diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorWhite: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn ifElseTest_h4h4h4h4(colorBlue: vec4<f32>, colorGreen: vec4<f32>, colorRed: vec4<f32>) -> vec4<f32> {
  {
    var result: vec4<f32> = vec4<f32>(0.0);
    if any(_globalUniforms.colorWhite != colorBlue) {
      {
        if all(colorGreen == colorRed) {
          {
            result = colorRed;
          }
        } else {
          {
            result = colorGreen;
          }
        }
      }
    } else {
      {
        if any(colorRed != colorGreen) {
          {
            result = colorBlue;
          }
        } else {
          {
            result = _globalUniforms.colorWhite;
          }
        }
      }
    }
    if all(colorRed == colorBlue) {
      {
        return _globalUniforms.colorWhite;
      }
    }
    if any(colorRed != colorGreen) {
      {
        return result;
      }
    }
    if all(colorRed == _globalUniforms.colorWhite) {
      {
        return colorBlue;
      }
    }
    return colorRed;
  }
}
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = ifElseTest_h4h4h4h4(vec4<f32>(0.0, 0.0, _globalUniforms.colorWhite.z, 1.0), vec4<f32>(0.0, _globalUniforms.colorWhite.y, 0.0, 1.0), vec4<f32>(_globalUniforms.colorWhite.x, 0.0, 0.0, 1.0));
    return _skTemp0;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
