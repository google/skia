diagnostic(off, derivative_uniformity);
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
    var test1: array<f32, 4> = array<f32, 4>(1.0, 2.0, 3.0, 4.0);
    var test2: array<vec2<f32>, 2> = array<vec2<f32>, 2>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
    var test3: array<mat4x4<f32>, 1> = array<mat4x4<f32>, 1>(mat4x4<f32>(16.0, 0.0, 0.0, 0.0, 0.0, 16.0, 0.0, 0.0, 0.0, 0.0, 16.0, 0.0, 0.0, 0.0, 0.0, 16.0));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((test1[3] + test2[1].y) + test3[0][3].w) == 24.0));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
