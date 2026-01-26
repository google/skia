diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
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
fn getColor_h4f(arr: ptr<storage, array<f32>, read>) -> vec4<f16> {
  {
    return vec4<f16>(f16((*arr)[0]), f16((*arr)[1]), f16((*arr)[2]), f16((*arr)[3]));
  }
}
fn getColor_helper_h4f(arr: ptr<storage, array<f32>, read>) -> vec4<f16> {
  {
    return getColor_h4f(arr);
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = getColor_helper_h4f(&(_storage0.testArr));
    unsizedInParameterA_ff(&(_storage0.testArr));
    unsizedInParameterB_fS(&(_storage1.testArrStruct));
    unsizedInParameterC_ff(&(_storage0.testArr));
    unsizedInParameterD_fS(&(_storage1.testArrStruct));
    unsizedInParameterE_ff(&(_storage0.testArr));
    unsizedInParameterF_fS(&(_storage1.testArrStruct));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
