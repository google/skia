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
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f16> {
  {
    var counter: i32 = 0;
    const increment: i32 = 1;
    {
      var i: i32 = 0;
      loop {
        {
          const increment: i32 = 10;
          if i == 0 {
            {
              continue;
            }
          }
          counter = counter + increment;
        }
        continuing {
          i = i + increment;
          break if i >= 10;
        }
      }
    }
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(counter == 90));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
