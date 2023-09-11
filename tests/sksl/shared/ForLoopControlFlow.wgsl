diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorWhite: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: vec4<f32> = _globalUniforms.colorWhite;
    {
      var r: f32 = -5.0;
      loop {
        {
          let _skTemp0 = saturate(r);
          x.x = _skTemp0;
          if x.x == 0.0 {
            break;
          }
        }
        continuing {
          r = r + 1.0;
          break if r >= 5.0;
        }
      }
    }
    {
      var b: f32 = 5.0;
      loop {
        {
          x.z = b;
          if x.w == 1.0 {
            continue;
          }
          x.y = 0.0;
        }
        continuing {
          b = b - 1.0;
          break if b < 0.0;
        }
      }
    }
    return x;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
