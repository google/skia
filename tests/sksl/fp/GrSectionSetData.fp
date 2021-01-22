uniform half calculated;
layout(key) in half provided;
@setData(varName) { varName.set1f(calculated, provided * 2); }
void main() {
    sk_OutColor = half4(1);
}
