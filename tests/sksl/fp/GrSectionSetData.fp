uniform half calculated;
layout(key) in half provided;
@setData(varName) { varName.set1f(calculated, provided * 2); }
half4 main() {
    return half4(1);
}
