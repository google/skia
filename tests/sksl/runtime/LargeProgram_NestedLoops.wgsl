diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
fn _skslMain(xy: vec2<f32>) -> vec4<f16> {
  {
    var i: i32;
    {
      var a: i32 = 0;
      for (; a < 10; a = a + i32(1)) {
        {
          {
            var b: i32 = 0;
            for (; b < 10; b = b + i32(1)) {
              {
                {
                  var c: i32 = 0;
                  for (; c < 10; c = c + i32(1)) {
                    {
                      {
                        var d: i32 = 0;
                        for (; d < 10; d = d + i32(1)) {
                          {
                            i = i + i32(1);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    return vec4<f16>(0.0h);
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f16> {
  return _skslMain(_coords);
}
