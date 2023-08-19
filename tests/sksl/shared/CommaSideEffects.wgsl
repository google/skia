diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
  colorWhite: vec4<f32>,
  colorBlack: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn setToColorBlack_vh4(_skParam0: ptr<function, vec4<f32>>) {
  let x = _skParam0;
  {
    (*x) = _globalUniforms.colorBlack;
  }
}
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var a: vec4<f32>;
    var b: vec4<f32>;
    var c: vec4<f32>;
    var d: vec4<f32>;
    b = _globalUniforms.colorRed;
    c = _globalUniforms.colorGreen;
    var _skTemp0: vec4<f32>;
    setToColorBlack_vh4(&_skTemp0);
    d = _skTemp0;
    a = _globalUniforms.colorWhite;
    a = a * a;
    b = b * b;
    c = c * c;
    d = d * d;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((all(a == _globalUniforms.colorWhite) && all(b == _globalUniforms.colorRed)) && all(c == _globalUniforms.colorGreen)) && all(d == _globalUniforms.colorBlack)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
