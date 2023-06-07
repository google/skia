struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn shouldLoop_bh4(_skParam0: vec4<f32>) -> bool {
  let value = _skParam0;
  {
    return any(value != _globalUniforms.colorGreen);
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var result: vec4<f32> = _globalUniforms.colorRed;
    loop {
      {
        result = _globalUniforms.colorGreen;
      }
      continuing {
        let _skTemp0 = shouldLoop_bh4(result);
        break if !(_skTemp0);
      }
    }
    return result;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
