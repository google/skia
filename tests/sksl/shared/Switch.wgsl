diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var color: vec4<f32>;
    let _skTemp0 = i32(_globalUniforms.colorGreen.y);
    switch _skTemp0 {
      case 0 {
        color = _globalUniforms.colorRed;
        break;
      }
      case 1 {
        color = _globalUniforms.colorGreen;
        break;
      }
      case default {
        color = _globalUniforms.colorRed;
        break;
      }
    }
    return color;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
