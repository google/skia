### Compilation failed:

error: :8:14 error: uniform storage requires that array elements are aligned to 16 bytes, but array element of type 'f32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.
  testArray: array<f32, 5>,
             ^^^^^^^^^^^^^

:5:1 note: see layout of struct:
/*            align(16) size(64) */ struct _GlobalUniforms {
/* offset( 0) align(16) size(16) */   colorGreen : vec4<f32>;
/* offset(16) align(16) size(16) */   colorRed : vec4<f32>;
/* offset(32) align( 4) size(20) */   testArray : array<f32, 5>;
/* offset(52) align( 1) size(12) */   // -- implicit struct size padding --;
/*                               */ };
struct _GlobalUniforms {
^^^^^^

:10:36 note: '_GlobalUniforms' used in address space 'uniform' here
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
                                   ^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testArray: array<f32, 5>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var one: f32 = _globalUniforms.testArray[0];
    var two: f32 = _globalUniforms.testArray[1];
    var three: f32 = _globalUniforms.testArray[2];
    var four: f32 = f32(_globalUniforms.testArray[3]);
    var five: f32 = f32(_globalUniforms.testArray[4]);
    let _skTemp0 = fma(one, two, three);
    let _skTemp1 = fma(f32(three), four, five);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((_skTemp0 == 5.0) && (_skTemp1 == 17.0)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
