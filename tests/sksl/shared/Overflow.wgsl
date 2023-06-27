### Compilation failed:

error: :15:21 error: value 900000100000000007342924977966020526635882908016508349721763453525374162398817767209077443987414408920445052873542129070112768.0 cannot be represented as 'f32'
    var huge: f32 = f32((((((((((9.000001e+35 * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09);
                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var huge: f32 = f32((((((((((9.000001e+35 * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09);
    var hugeI: i32 = i32((((((((((((((((((((1073741824 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    var hugeU: u32 = ((((((((((((((((((2147483648u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    var hugeS: i32 = ((((((((((((((((16384 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    var hugeUS: u32 = (((((((((((((((32768u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    var hugeNI: i32 = i32(((((((((((((((((((-2147483648 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    var hugeNS: i32 = (((((((((((((((-32768 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    const i4: vec4<i32> = vec4<i32>(2);
    var hugeIvec: vec4<i32> = ((((((((((((((vec4<i32>(1073741824) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4;
    const u4: vec4<u32> = vec4<u32>(2u);
    var hugeUvec: vec4<u32> = (((((((((((((vec4<u32>(2147483648u) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4;
    let _skTemp0 = saturate(huge);
    let _skTemp1 = saturate(f32(hugeI));
    let _skTemp2 = saturate(f32(hugeU));
    let _skTemp3 = saturate(f32(hugeS));
    let _skTemp4 = saturate(f32(hugeUS));
    let _skTemp5 = saturate(f32(hugeNI));
    let _skTemp6 = saturate(f32(hugeNS));
    let _skTemp7 = saturate(vec4<f32>(hugeIvec));
    let _skTemp8 = saturate(vec4<f32>(hugeUvec));
    return ((((((((_globalUniforms.colorGreen * _skTemp0) * _skTemp1) * _skTemp2) * _skTemp3) * _skTemp4) * _skTemp5) * _skTemp6) * _skTemp7) * _skTemp8;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}

1 error
