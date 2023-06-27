### Compilation failed:

error: :4:1 error: structures must have at least one member
struct FSOut {
^^^^^^


struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
};
fn main() {
  {
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main();
  return _stageOut;
}

1 error
