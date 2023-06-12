### Compilation failed:

error: :14:20 error: no matching call to min(vec4<f32>, abstract-float)

2 candidate functions:
  min(T, T) -> T  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  min(vecN<T>, vecN<T>) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32, u32 or f16

    let _skTemp0 = min(_globalUniforms.src + _globalUniforms.dst, 1.0);
                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  src: vec4<f32>,
  dst: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = min(_globalUniforms.src + _globalUniforms.dst, 1.0);
    (*_stageOut).sk_FragColor = _skTemp0;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
