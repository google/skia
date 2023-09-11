diagnostic(off, derivative_uniformity);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct S {
  i: f32,
  j: f32,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: vec4<f32> = vec4<f32>(3.0, 2.0, 1.0, 0.0);
    x = vec4<f32>((x.zyx), x.w).xyzw;
    var s: S;
    s.i = 2.0;
    s.j = 2.0;
    s.i = s.j;
    s.j = s.i;
    var a: array<f32, 2>;
    a[0] = 1.0;
    a[1] = 0.0;
    a[1] = a[0];
    return vec4<f32>(x.w, s.i / s.j, a[0] - a[1], a[0] * a[1]);
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
