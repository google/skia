### Compilation failed:

error: Tint compilation failed.

diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct testStorageBuffer {
  testArr: array<f32>,
};
@group(0) @binding(0) var<storage, read> _storage0 : testStorageBuffer;
struct testStorageBufferStruct {
  testArrStruct: array<S>,
};
@group(0) @binding(1) var<storage, read> _storage1 : testStorageBufferStruct;
struct S {
  y: f32,
};
fn unsizedInParameterA_ff(x: array<f32>) -> f32 {
  {
    return x[0];
  }
}
fn unsizedInParameterB_fS(x: array<S>) -> f32 {
  {
    return x[0].y;
  }
}
fn unsizedInParameterC_ff(x: array<f32>) -> f32 {
  {
    return x[0];
  }
}
fn unsizedInParameterD_fS(x: array<S>) -> f32 {
  {
    return x[0].y;
  }
}
fn unsizedInParameterE_ff(_skParam0: array<f32>) -> f32 {
  {
    return 0.0;
  }
}
fn unsizedInParameterF_fS(_skParam0: array<S>) -> f32 {
  {
    return 0.0;
  }
}
fn getColor_h4f(arr: array<f32>) -> vec4<f32> {
  {
    return vec4<f32>(f32(arr[0]), f32(arr[1]), f32(arr[2]), f32(arr[3]));
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp2 = getColor_h4f(_storage0.testArr);
    (*_stageOut).sk_FragColor = _skTemp2;
    let _skTemp3 = unsizedInParameterA_ff(_storage0.testArr);
    let _skTemp4 = unsizedInParameterB_fS(_storage1.testArrStruct);
    let _skTemp5 = unsizedInParameterC_ff(_storage0.testArr);
    let _skTemp6 = unsizedInParameterD_fS(_storage1.testArrStruct);
    let _skTemp7 = unsizedInParameterE_ff(_storage0.testArr);
    let _skTemp8 = unsizedInParameterF_fS(_storage1.testArrStruct);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}

1 error
