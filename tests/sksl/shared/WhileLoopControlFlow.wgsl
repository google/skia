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
      if x.w == 1.0h {
        {
          x.x = x.x - 0.25h;
          if x.x <= 0.0h {
            break;
          }
        }
      } else {
        break;
      }
    }
    loop {
      if x.z > 0.0h {
        {
          x.z = x.z - 0.25h;
          if x.w == 1.0h {
            continue;
          }
          x.y = 0.0h;
        }
      } else {
        break;
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
