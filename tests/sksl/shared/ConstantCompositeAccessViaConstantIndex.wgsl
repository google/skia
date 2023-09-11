### Compilation failed:

error: :8:14 error: uniform storage requires that array elements are aligned to 16 bytes, but array element of type 'f32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.
  testArray: array<f32, 5>,
             ^^^^^^^^^^^^^

:5:1 note: see layout of struct:
/*            align(16) size(64) */ struct _GlobalUniforms {
/* offset( 0) align(16) size(16) */   colorRed : vec4<f32>;
/* offset(16) align( 8) size(16) */   testMatrix2x2 : mat2x2<f32>;
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
  colorRed: vec4<f32>,
  testMatrix2x2: mat2x2<f32>,
  testArray: array<f32, 5>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
const globalArray: array<f32, 5> = array<f32, 5>(1.0, 1.0, 1.0, 1.0, 1.0);
const globalVector: vec2<f32> = vec2<f32>(1.0);
const globalMatrix: mat2x2<f32> = mat2x2<f32>(1.0, 1.0, 1.0, 1.0);
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    const localArray: array<f32, 5> = array<f32, 5>(0.0, 1.0, 2.0, 3.0, 4.0);
    const localVector: vec2<f32> = vec2<f32>(1.0);
    const localMatrix: mat2x2<f32> = mat2x2<f32>(0.0, 1.0, 2.0, 3.0);
    if ((((((globalArray[0] == _globalUniforms.testArray[0]) && (globalArray[1] == _globalUniforms.testArray[1]) && (globalArray[2] == _globalUniforms.testArray[2]) && (globalArray[3] == _globalUniforms.testArray[3]) && (globalArray[4] == _globalUniforms.testArray[4])) || all(globalVector == _globalUniforms.colorRed.xy)) || (all(globalMatrix[0] == _globalUniforms.testMatrix2x2[0]) && all(globalMatrix[1] == _globalUniforms.testMatrix2x2[1]))) || ((localArray[0] == _globalUniforms.testArray[0]) && (localArray[1] == _globalUniforms.testArray[1]) && (localArray[2] == _globalUniforms.testArray[2]) && (localArray[3] == _globalUniforms.testArray[3]) && (localArray[4] == _globalUniforms.testArray[4]))) || all(localVector == _globalUniforms.colorRed.xy)) || (all(localMatrix[0] == _globalUniforms.testMatrix2x2[0]) && all(localMatrix[1] == _globalUniforms.testMatrix2x2[1])) {
      {
        return _globalUniforms.colorRed;
      }
    }
    return vec4<f32>(0.0, 1.0, 0.0, 1.0);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
