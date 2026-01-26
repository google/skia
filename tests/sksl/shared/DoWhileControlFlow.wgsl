diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var x: vec4<f16> = vec4<f16>(1.0h);
    loop {
      {
        x.x = x.x - 0.25h;
        if x.x <= 0.0h {
          break;
        }
      }
      continuing {
        break if x.w != 1.0h;
      }
    }
    loop {
      {
        x.z = x.z - 0.25h;
        if x.w == 1.0h {
          continue;
        }
        x.y = 0.0h;
      }
      continuing {
        break if x.z <= 0.0h;
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
