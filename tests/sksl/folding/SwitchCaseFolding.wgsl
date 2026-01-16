diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var color: vec4<f32> = _globalUniforms.colorRed;
    let _skTemp0 = i32(_globalUniforms.colorGreen.y);
    switch _skTemp0 {
      case 0 {
        break;
      }
      case 1 {
        color = _globalUniforms.colorGreen;
        break;
      }
      case 2 {
        break;
      }
      case 3 {
        break;
      }
      case 4 {
        break;
      }
      case 5 {
        break;
      }
      case default {
        break;
      }
    }
    return color;
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
