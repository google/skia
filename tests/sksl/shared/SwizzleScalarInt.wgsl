diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var i: i32 = i32(_globalUniforms.unknownInput);
    var i4: vec4<i32> = vec4<i32>(i);
    i4 = vec4<i32>(vec2<i32>(i), 0, 1);
    i4 = vec4<i32>(0, i, 1, 0);
    i4 = vec4<i32>(0, i, 0, i);
    return vec4<f32>(i4);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
