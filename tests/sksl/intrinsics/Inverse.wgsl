diagnostic(off, derivative_uniformity);
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
fn mat3_inverse(m: mat3x3<f32>) -> mat3x3<f32> {
let a00 = m[0].x; let a01 = m[0].y; let a02 = m[0].z;
let a10 = m[1].x; let a11 = m[1].y; let a12 = m[1].z;
let a20 = m[2].x; let a21 = m[2].y; let a22 = m[2].z;
let b01 =  a22*a11 - a12*a21;
let b11 = -a22*a10 + a12*a20;
let b21 =  a21*a10 - a11*a20;
let det = a00*b01 + a01*b11 + a02*b21;
return mat3x3<f32>(b01, (-a22*a01 + a02*a21), ( a12*a01 - a02*a11),
b11, ( a22*a00 - a02*a20), (-a12*a00 + a02*a10),
b21, (-a21*a00 + a01*a20), ( a11*a00 - a01*a10)) * (1/det);
}
fn mat2_inverse(m: mat2x2<f32>) -> mat2x2<f32> {
return mat2x2<f32>(m[1].y, -m[0].y, -m[1].x, m[0].x) * (1/determinant(m));
}
fn mat4_inverse(m: mat4x4<f32>) -> mat4x4<f32>{
let a00 = m[0].x; let a01 = m[0].y; let a02 = m[0].z; let a03 = m[0].w;
let a10 = m[1].x; let a11 = m[1].y; let a12 = m[1].z; let a13 = m[1].w;
let a20 = m[2].x; let a21 = m[2].y; let a22 = m[2].z; let a23 = m[2].w;
let a30 = m[3].x; let a31 = m[3].y; let a32 = m[3].z; let a33 = m[3].w;
let b00 = a00*a11 - a01*a10;
let b01 = a00*a12 - a02*a10;
let b02 = a00*a13 - a03*a10;
let b03 = a01*a12 - a02*a11;
let b04 = a01*a13 - a03*a11;
let b05 = a02*a13 - a03*a12;
let b06 = a20*a31 - a21*a30;
let b07 = a20*a32 - a22*a30;
let b08 = a20*a33 - a23*a30;
let b09 = a21*a32 - a22*a31;
let b10 = a21*a33 - a23*a31;
let b11 = a22*a33 - a23*a32;
let det = b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06;
return mat4x4<f32>(a11*b11 - a12*b10 + a13*b09,
a02*b10 - a01*b11 - a03*b09,
a31*b05 - a32*b04 + a33*b03,
a22*b04 - a21*b05 - a23*b03,
a12*b08 - a10*b11 - a13*b07,
a00*b11 - a02*b08 + a03*b07,
a32*b02 - a30*b05 - a33*b01,
a20*b05 - a22*b02 + a23*b01,
a10*b10 - a11*b08 + a13*b06,
a01*b08 - a00*b10 - a03*b06,
a30*b04 - a31*b02 + a33*b00,
a21*b02 - a20*b04 - a23*b00,
a11*b07 - a10*b09 - a12*b06,
a00*b09 - a01*b07 + a02*b06,
a31*b01 - a30*b03 - a32*b00,
a20*b03 - a21*b01 + a22*b00) * (1/det);
}
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    const matrix2x2: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    var inv2x2: mat2x2<f32> = mat2x2<f32>(-2.0, 1.0, 1.5, -0.5);
    var inv3x3: mat3x3<f32> = mat3x3<f32>(-24.0, 18.0, 5.0, 20.0, -15.0, -4.0, -5.0, 4.0, 1.0);
    var inv4x4: mat4x4<f32> = mat4x4<f32>(-2.0, -0.5, 1.0, 0.5, 1.0, 0.5, 0.0, -0.5, -8.0, -1.0, 2.0, 2.0, 3.0, 0.5, -1.0, -0.5);
    var Zero: f32 = f32(_globalUniforms.colorGreen.z);
    let _skTemp0 = mat2x2<f32>(-2.0, 1.0, 1.5, -0.5);
    let _skTemp1 = mat3x3<f32>(-24.0, 18.0, 5.0, 20.0, -15.0, -4.0, -5.0, 4.0, 1.0);
    let _skTemp2 = mat4x4<f32>(-2.0, -0.5, 1.0, 0.5, 1.0, 0.5, 0.0, -0.5, -8.0, -1.0, 2.0, 2.0, 3.0, 0.5, -1.0, -0.5);
    let _skTemp3 = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    let _skTemp4 = mat3_inverse(_skTemp3);
    let _skTemp5 = _skTemp4;
    let _skTemp6 = mat2_inverse(mat2x2<f32>(matrix2x2[0] + Zero, matrix2x2[1] + Zero));
    let _skTemp7 = _skTemp6;
    let _skTemp8 = mat3x3<f32>(1.0, 2.0, 3.0, 0.0, 1.0, 4.0, 5.0, 6.0, 0.0);
    let _skTemp9 = mat3_inverse(mat3x3<f32>(_skTemp8[0] + Zero, _skTemp8[1] + Zero, _skTemp8[2] + Zero));
    let _skTemp10 = _skTemp9;
    let _skTemp11 = mat4x4<f32>(1.0, 0.0, 0.0, 1.0, 0.0, 2.0, 1.0, 2.0, 2.0, 1.0, 0.0, 1.0, 2.0, 0.0, 1.0, 4.0);
    let _skTemp12 = mat4_inverse(mat4x4<f32>(_skTemp11[0] + Zero, _skTemp11[1] + Zero, _skTemp11[2] + Zero, _skTemp11[3] + Zero));
    let _skTemp13 = _skTemp12;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((all(_skTemp0[0] == inv2x2[0]) && all(_skTemp0[1] == inv2x2[1])) && (all(_skTemp1[0] == inv3x3[0]) && all(_skTemp1[1] == inv3x3[1]) && all(_skTemp1[2] == inv3x3[2]))) && (all(_skTemp2[0] == inv4x4[0]) && all(_skTemp2[1] == inv4x4[1]) && all(_skTemp2[2] == inv4x4[2]) && all(_skTemp2[3] == inv4x4[3]))) && (any(_skTemp5[0] != inv3x3[0]) || any(_skTemp5[1] != inv3x3[1]) || any(_skTemp5[2] != inv3x3[2]))) && (all(_skTemp7[0] == inv2x2[0]) && all(_skTemp7[1] == inv2x2[1]))) && (all(_skTemp10[0] == inv3x3[0]) && all(_skTemp10[1] == inv3x3[1]) && all(_skTemp10[2] == inv3x3[2]))) && (all(_skTemp13[0] == inv4x4[0]) && all(_skTemp13[1] == inv4x4[1]) && all(_skTemp13[2] == inv4x4[2]) && all(_skTemp13[3] == inv4x4[3]))));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
