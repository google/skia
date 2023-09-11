diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorWhite: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var color: vec4<f32> = vec4<f32>(0.0);
    if _globalUniforms.colorWhite.x == 1.0 {
      color.y = 1.0;
    }
    if _globalUniforms.colorWhite.x == 2.0 {
      ;
    } else {
      color.w = 1.0;
    }
    loop {
      if _globalUniforms.colorWhite.x == 2.0 {
        ;
      } else {
        break;
      }
    }
    loop {
      ;
      continuing {
        break if _globalUniforms.colorWhite.x != 2.0;
      }
    }
    return color;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
