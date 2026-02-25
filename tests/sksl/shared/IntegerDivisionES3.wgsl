diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    let zero: i32 = i32(_globalUniforms.colorGreen.x);
    let one: i32 = i32(_globalUniforms.colorGreen.y);
    {
      var x: i32 = zero;
      for (; x < 100; x = x + i32(1)) {
        {
          {
            var y: i32 = one;
            for (; y < 100; y = y + i32(1)) {
              {
                var _0_x: i32 = x;
                var _1_result: i32 = 0;
                for (; _0_x >= y; ) {
                  {
                    _1_result = _1_result + i32(1);
                    _0_x = _0_x - y;
                  }
                }
                if (x / y) != _1_result {
                  {
                    return vec4<f16>(1.0h, f16(f32(x) * 0.003921569), f16(f32(y) * 0.003921569), 1.0h);
                  }
                }
              }
            }
          }
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
