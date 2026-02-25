diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var val: f32 = 0.0;
    {
      var a: f32 = 12.0;
      loop {
        let _skTemp0 = sqrt(a); if a > (_skTemp0 - 0.5 * floor(_skTemp0 / 0.5)) {
          {
            val = val + a;
          }
        } else {
          break;
        }
        continuing {
          a = a - 1.0; 
        }
      }
    }
    {
      var a: f32 = 0.0;
      loop {
        if a < 20.0 {
          {
            val = val + a;
          }
        } else {
          break;
        }
        continuing {
          let _skTemp1 = 1.0 - sqrt(a); a = (a + (_skTemp1 - 3.0 * floor(_skTemp1 / 3.0))) + 1.0; 
        }
      }
    }
    {
      var a: f32 = 12.0;
      loop {
        let _skTemp2 = sqrt(a); if a > (_skTemp2 - 0.5 * floor(_skTemp2 / 0.5)) {
          {
            val = val + a;
          }
        } else {
          break;
        }
        continuing {
          let _skTemp3 = 1.0 - sqrt(a); a = (a - (_skTemp3 - 3.0 * floor(_skTemp3 / 3.0))) - 1.0; 
        }
      }
    }
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(val > 0.0));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
