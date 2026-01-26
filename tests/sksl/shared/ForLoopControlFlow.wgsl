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
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var x: vec4<f16> = _globalUniforms.colorWhite;
    {
      var r: f16 = -5.0h;
      loop {
        {
          x.x = saturate(r);
          if x.x == 0.0h {
            break;
          }
        }
        continuing {
          r = r + 1.0h;
          break if r >= 5.0h;
        }
      }
    }
    {
      var b: f16 = 5.0h;
      loop {
        {
          x.z = b;
          if x.w == 1.0h {
            continue;
          }
          x.y = 0.0h;
        }
        continuing {
          b = b - 1.0h;
          break if b < 0.0h;
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
