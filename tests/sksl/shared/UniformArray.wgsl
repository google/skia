### Compilation failed:

error: Tint compilation failed.

diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testArray: array<f32, 5>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    {
      var index: i32 = 0;
      loop {
        {
          if _globalUniforms.testArray[index] != f32(index + 1) {
            {
              return _globalUniforms.colorRed;
            }
          }
        }
        continuing {
          index = index + i32(1);
          break if index >= 5;
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

1 error
