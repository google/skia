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
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(_skParam0: vec2<f32>, _skParam1: vec2<f32>, _skParam2: vec2<f32>, _skParam3: vec3<f32>, _skParam4: vec2<i32>, _skParam5: vec2<i32>, _skParam6: vec2<f32>, _skParam7: vec2<f32>, _skParam8: vec4<f32>, _skParam9: vec2<i32>, _skParam10: vec4<bool>, _skParam11: vec2<f32>, _skParam12: vec2<f32>, _skParam13: vec2<f32>, _skParam14: vec2<bool>, _skParam15: vec2<bool>, _skParam16: vec3<bool>, _skParam17: vec4<i32>) -> bool {
  let v1 = _skParam0;
  let v2 = _skParam1;
  let v3 = _skParam2;
  let v4 = _skParam3;
  let v5 = _skParam4;
  let v6 = _skParam5;
  let v7 = _skParam6;
  let v8 = _skParam7;
  let v9 = _skParam8;
  let v10 = _skParam9;
  let v11 = _skParam10;
  let v12 = _skParam11;
  let v13 = _skParam12;
  let v14 = _skParam13;
  let v15 = _skParam14;
  let v16 = _skParam15;
  let v17 = _skParam16;
  let v18 = _skParam17;
  {
    return (((((((((((((((((f32(v1.x) + f32(v2.x)) + f32(v3.x)) + f32(v4.x)) + f32(v5.x)) + f32(v6.x)) + f32(v7.x)) + f32(v8.x)) + f32(v9.x)) + f32(v10.x)) + f32(v11.x)) + f32(v12.x)) + f32(v13.x)) + f32(v14.x)) + f32(v15.x)) + f32(v16.x)) + f32(v17.x)) + f32(v18.x)) == 18.0;
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var v1: vec2<f32> = vec2<f32>(1.0);
    var v2: vec2<f32> = vec2<f32>(1.0, 2.0);
    var v3: vec2<f32> = vec2<f32>(1.0);
    var v4: vec3<f32> = vec3<f32>(1.0);
    var v5: vec2<i32> = vec2<i32>(1);
    var v6: vec2<i32> = vec2<i32>(1, 2);
    var v7: vec2<f32> = vec2<f32>(1.0, 2.0);
    var v8: vec2<f32> = vec2<f32>(v5);
    var v9: vec4<f32> = vec4<f32>(f32(v6.x), _globalUniforms.unknownInput, 3.0, 4.0);
    var v10: vec2<i32> = vec2<i32>(3, i32(v1.x));
    var v11: vec4<bool> = vec4<bool>(true, false, true, false);
    var v12: vec2<f32> = vec2<f32>(1.0, 0.0);
    var v13: vec2<f32> = vec2<f32>(0.0);
    var v14: vec2<f32> = vec2<f32>(0.0);
    var v15: vec2<bool> = vec2<bool>(true);
    var v16: vec2<bool> = vec2<bool>(true);
    var v17: vec3<bool> = vec3<bool>(true);
    var v18: vec4<i32> = vec4<i32>(1);
    var _skTemp0: vec4<f32>;
    let _skTemp1 = check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18);
    if _skTemp1 {
      _skTemp0 = _globalUniforms.colorGreen;
    } else {
      _skTemp0 = _globalUniforms.colorRed;
    }
    return _skTemp0;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
