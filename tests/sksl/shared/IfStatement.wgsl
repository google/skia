diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorWhite: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn ifElseTest_h4h4h4h4(colorBlue: vec4<f16>, colorGreen: vec4<f16>, colorRed: vec4<f16>) -> vec4<f16> {
  {
    var result: vec4<f16> = vec4<f16>(0.0h);
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
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f16> {
  {
    return ifElseTest_h4h4h4h4(vec4<f16>(0.0h, 0.0h, _globalUniforms.colorWhite.z, 1.0h), vec4<f16>(0.0h, _globalUniforms.colorWhite.y, 0.0h, 1.0h), vec4<f16>(_globalUniforms.colorWhite.x, 0.0h, 0.0h, 1.0h));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
