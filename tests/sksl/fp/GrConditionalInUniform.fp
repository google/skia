layout(key) in bool test;
layout(ctype=SkPMColor4f, tracked, when=test) in uniform half4 color;
half4 main() {
  if (test) {
    return color;
  } else {
    return half4(1);
  }
}
