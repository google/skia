### Compilation failed:

error: :5:1 error: structures must have at least one member
struct FSOut {
^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
};
fn main() -> vec3<i32> {
  {
    return vec3<i32>(1, 2, 3);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main();
  return _stageOut;
}

1 error
