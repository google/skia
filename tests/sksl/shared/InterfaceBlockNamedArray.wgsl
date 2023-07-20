### Compilation failed:

error: :11:38 error: uniform storage requires that array elements are aligned to 16 bytes, but array element of type 'testBlock' has a stride of 4 bytes. Consider using the @size attribute on the last struct member.
@group(0) @binding(123) var<uniform> test : array<testBlock, 2>;
                                     ^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct testBlock {
  x: f32,
};
@group(0) @binding(123) var<uniform> test : array<testBlock, 2>;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(f32(test[1].x));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
