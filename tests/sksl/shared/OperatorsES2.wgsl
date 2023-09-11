diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: f32 = 1.0;
    var y: f32 = 2.0;
    var z: i32 = 3;
    x = (x - x) + ((y * x) * x) * (y - x);
    y = (x / y) / x;
    z = ((z / 2) * 3 + 4) - 2;
    var b: bool = ((x > 4.0) == (x < 2.0)) || ((2.0 >= _globalUniforms.unknownInput) && (y <= x));
    var c: bool = _globalUniforms.unknownInput > 2.0;
    var d: bool = b != c;
    var e: bool = b && c;
    var f: bool = b || c;
    x = x + 12.0;
    x = x - 12.0;
    y = y * 0.1;
    x = x * y;
    x = 6.0;
    y = (((f32(b) * f32(c)) * f32(d)) * f32(e)) * f32(f);
    y = 6.0;
    z = z - 1;
    z = 6;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((x == 6.0) && (y == 6.0)) && (z == 6)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
