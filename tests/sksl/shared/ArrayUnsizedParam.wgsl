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
fn unsizedInParameterA_ff(x: ptr<storage, array<f32>, read>) -> f32 {
  {
    return (*x)[0];
  }
}
fn unsizedInParameterB_fS(x: ptr<storage, array<S>, read>) -> f32 {
  {
    return (*x)[0].y;
  }
}
fn unsizedInParameterC_ff(x: ptr<storage, array<f32>, read>) -> f32 {
  {
    return (*x)[0];
  }
}
fn unsizedInParameterD_fS(x: ptr<storage, array<S>, read>) -> f32 {
  {
    return (*x)[0].y;
  }
}
fn unsizedInParameterE_ff(_skParam0: ptr<storage, array<f32>, read>) -> f32 {
  {
    return 0.0;
  }
}
fn unsizedInParameterF_fS(_skParam0: ptr<storage, array<S>, read>) -> f32 {
  {
    return 0.0;
  }
}
fn getColor_h4f(arr: ptr<storage, array<f32>, read>) -> vec4<f32> {
  {
    return vec4<f32>(f32((*arr)[0]), f32((*arr)[1]), f32((*arr)[2]), f32((*arr)[3]));
  }
}
fn getColor_helper_h4f(arr: ptr<storage, array<f32>, read>) -> vec4<f32> {
  {
    let _skTemp2 = getColor_h4f(arr);
    return _skTemp2;
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp3 = getColor_helper_h4f(&(_storage0.testArr));
    (*_stageOut).sk_FragColor = _skTemp3;
    let _skTemp4 = unsizedInParameterA_ff(&(_storage0.testArr));
    let _skTemp5 = unsizedInParameterB_fS(&(_storage1.testArrStruct));
    let _skTemp6 = unsizedInParameterC_ff(&(_storage0.testArr));
    let _skTemp7 = unsizedInParameterD_fS(&(_storage1.testArrStruct));
    let _skTemp8 = unsizedInParameterE_ff(&(_storage0.testArr));
    let _skTemp9 = unsizedInParameterF_fS(&(_storage1.testArrStruct));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
