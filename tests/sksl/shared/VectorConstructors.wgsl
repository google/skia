diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(v1: vec2<f32>, v2: vec2<f32>, v3: vec2<f32>, v4: vec3<f32>, v5: vec2<i32>, v6: vec2<i32>, v7: vec2<f32>, v8: vec2<f32>, v9: vec4<f32>, v10: vec2<i32>, v11: vec4<bool>, v12: vec2<f32>, v13: vec2<f32>, v14: vec2<f32>, v15: vec2<bool>, v16: vec2<bool>, v17: vec3<bool>, v18: vec4<i32>) -> bool {
  {
    return (((((((((((((((((f32(v1.x) + f32(v2.x)) + f32(v3.x)) + f32(v4.x)) + f32(v5.x)) + f32(v6.x)) + f32(v7.x)) + f32(v8.x)) + f32(v9.x)) + f32(v10.x)) + f32(v11.x)) + f32(v12.x)) + f32(v13.x)) + f32(v14.x)) + f32(v15.x)) + f32(v16.x)) + f32(v17.x)) + f32(v18.x)) == 18.0;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
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
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
