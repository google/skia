diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct InnerLUT {
  values: vec3<f32>,
};
struct OuterLUT {
  inner: array<InnerLUT, 3>,
};
struct Root {
  outer: array<OuterLUT, 3>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var data: Root;
    data.outer[0].inner[0].values = vec3<f32>(1.0, 10.0, 100.0);
    data.outer[0].inner[1].values = vec3<f32>(2.0, 20.0, 200.0);
    data.outer[0].inner[2].values = vec3<f32>(3.0, 30.0, 300.0);
    data.outer[1].inner[0].values = vec3<f32>(4.0, 40.0, 400.0);
    data.outer[1].inner[1].values = vec3<f32>(5.0, 50.0, 500.0);
    data.outer[1].inner[2].values = vec3<f32>(6.0, 60.0, 600.0);
    data.outer[2].inner[0].values = vec3<f32>(7.0, 70.0, 700.0);
    data.outer[2].inner[1].values = vec3<f32>(8.0, 80.0, 800.0);
    data.outer[2].inner[2].values = vec3<f32>(9.0, 90.0, 900.0);
    var expected: vec3<f32> = vec3<f32>(0.0);
    {
      var i: i32 = 0;
      loop {
        {
          {
            var j: i32 = 0;
            loop {
              {
                expected = expected + vec3<f32>(1.0, 10.0, 100.0);
                if any(data.outer[i].inner[j].values != expected) {
                  {
                    return _globalUniforms.colorRed;
                  }
                }
                {
                  var k: i32 = 0;
                  loop {
                    {
                      if data.outer[i].inner[j].values[k] != expected[k] {
                        {
                          return _globalUniforms.colorRed;
                        }
                      }
                    }
                    continuing {
                      k = k + i32(1);
                      break if k >= 3;
                    }
                  }
                }
              }
              continuing {
                j = j + i32(1);
                break if j >= 3;
              }
            }
          }
        }
        continuing {
          i = i + i32(1);
          break if i >= 3;
        }
      }
    }
    return _globalUniforms.colorGreen;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
