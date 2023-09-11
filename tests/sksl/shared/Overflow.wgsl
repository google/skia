diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const h: f32 = 1e+09;
    let _skTemp0 = 1e+36;
    var hugeH: f32 = ((((((((((_skTemp0 * h) * h) * h) * h) * h) * h) * h) * h) * h) * h) * h;
    const f: f32 = 1e+09;
    let _skTemp1 = 1e+36;
    var hugeF: f32 = ((((((((((_skTemp1 * f) * f) * f) * f) * f) * f) * f) * f) * f) * f) * f;
    let _skTemp2 = 1073741824;
    var hugeI: i32 = i32((((((((((((((((((((_skTemp2 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    let _skTemp3 = 2147483648u;
    var hugeU: u32 = ((((((((((((((((((_skTemp3 * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    let _skTemp4 = 16384;
    var hugeS: i32 = ((((((((((((((((_skTemp4 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    let _skTemp5 = 32768u;
    var hugeUS: u32 = (((((((((((((((_skTemp5 * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    let _skTemp6 = -2147483648;
    var hugeNI: i32 = i32(((((((((((((((((((_skTemp6 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    let _skTemp7 = -32768;
    var hugeNS: i32 = (((((((((((((((_skTemp7 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    const i4: vec4<i32> = vec4<i32>(2);
    let _skTemp8 = vec4<i32>(1073741824);
    var hugeIvec: vec4<i32> = ((((((((((((((_skTemp8 * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4;
    const u4: vec4<u32> = vec4<u32>(2u);
    let _skTemp9 = vec4<u32>(2147483648u);
    var hugeUvec: vec4<u32> = (((((((((((((_skTemp9 * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4;
    let _skTemp10 = mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    var hugeMxM: mat4x4<f32> = _skTemp10 * mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    let _skTemp11 = mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    var hugeMxV: vec4<f32> = _skTemp11 * vec4<f32>(1e+20);
    let _skTemp12 = vec4<f32>(1e+20);
    var hugeVxM: vec4<f32> = _skTemp12 * mat4x4<f32>(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    let _skTemp13 = saturate(hugeH);
    let _skTemp14 = saturate(f32(hugeF));
    let _skTemp15 = saturate(f32(hugeI));
    let _skTemp16 = saturate(f32(hugeU));
    let _skTemp17 = saturate(f32(hugeS));
    let _skTemp18 = saturate(f32(hugeUS));
    let _skTemp19 = saturate(f32(hugeNI));
    let _skTemp20 = saturate(f32(hugeNS));
    let _skTemp21 = saturate(vec4<f32>(hugeIvec));
    let _skTemp22 = saturate(vec4<f32>(hugeUvec));
    let _skTemp23 = saturate(vec4<f32>(hugeMxM[0]));
    let _skTemp24 = saturate(vec4<f32>(hugeMxV));
    let _skTemp25 = saturate(vec4<f32>(hugeVxM));
    return ((((((((((((_globalUniforms.colorGreen * _skTemp13) * _skTemp14) * _skTemp15) * _skTemp16) * _skTemp17) * _skTemp18) * _skTemp19) * _skTemp20) * _skTemp21) * _skTemp22) * _skTemp23) * _skTemp24) * _skTemp25;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
