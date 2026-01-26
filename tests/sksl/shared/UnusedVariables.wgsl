diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn userfunc_ff(v: f32) -> f32 {
  {
    return v + 1.0;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var b: f32 = 2.0;
    const c: f32 = 3.0;
    b = 2.0;
    b = c + 77.0;
    b = sin(c + 77.0);
    userfunc_ff(c + 77.0);
    b = userfunc_ff(c + 77.0);
    b = cos(c);
    b = b;
    {
      var x: i32 = 0;
      loop {
        {
          continue;
        }
        continuing {
          x = x + i32(1);
          break if x >= 1;
        }
      }
    }
    var d: f32 = c;
    b = 3.0;
    d = d + f32(1);
    return vec4<f16>(f16(b == 2.0), f16(b == 3.0), f16(d == 5.0), f16(d == 4.0));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
