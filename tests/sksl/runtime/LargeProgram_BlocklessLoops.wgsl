fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let xy = _skParam0;
  {
    var i: i32;
    {
      var a: i32 = 0;
      loop {
        {
          var b: i32 = 0;
          loop {
            {
              var c: i32 = 0;
              loop {
                {
                  var d: i32 = 0;
                  loop {
                    i = i + i32(1);
                    continuing {
                      d = d + i32(1);
                      break if d >= 10;
                    }
                  }
                }
                continuing {
                  c = c + i32(1);
                  break if c >= 10;
                }
              }
            }
            continuing {
              b = b + i32(1);
              break if b >= 10;
            }
          }
        }
        continuing {
          a = a + i32(1);
          break if a >= 10;
        }
      }
    }
    return vec4<f32>(0.0);
  }
}
@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return main(_coords);
}
