layout(key) in bool test;
layout(ctype=SkPMColor4f, tracked, when=test) in uniform half4 color;
void main() {
  if (test) {
    sk_OutColor = color;
  } else {
    sk_OutColor = half4(1);
  }
}
