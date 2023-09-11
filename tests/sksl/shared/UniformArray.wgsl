### Compilation failed:

error: :9:14 error: uniform storage requires that array elements are aligned to 16 bytes, but array element of type 'f32' has a stride of 4 bytes. Consider using a vector or struct as the element type instead.
  testArray: array<f32, 5>,
             ^^^^^^^^^^^^^

:8:1 note: see layout of struct:
/*            align(16) size(64) */ struct _GlobalUniforms {
/* offset( 0) align( 4) size(20) */   testArray : array<f32, 5>;
/* offset(20) align( 1) size(12) */   // -- implicit field alignment padding --;
/* offset(32) align(16) size(16) */   colorGreen : vec4<f32>;
/* offset(48) align(16) size(16) */   colorRed : vec4<f32>;
/*                               */ };
struct _GlobalUniforms {
^^^^^^

:13:36 note: '_GlobalUniforms' used in address space 'uniform' here
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
                                   ^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testArray: array<f32, 5>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    {
      var index: i32 = 0;
      loop {
        {
          if _globalUniforms.testArray[index] != f32(index + 1) {
            {
              return _globalUniforms.colorRed;
            }
          }
        }
        continuing {
          index = index + i32(1);
          break if index >= 5;
        }
      }
    }
    return _globalUniforms.colorGreen;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
