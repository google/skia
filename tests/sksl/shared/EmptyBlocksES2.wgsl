diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  unknownInput: f16,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var color: vec4<f16> = vec4<f16>(0.0h);
    if _globalUniforms.unknownInput == 1.0h {
      color.y = 1.0h;
    }
    if _globalUniforms.unknownInput == 2.0h {
      ;
    } else {
      color.w = 1.0h;
    }
    return color;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
