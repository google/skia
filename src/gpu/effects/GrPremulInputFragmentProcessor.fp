@optimizationFlags {
    kPreservesOpaqueInput_OptimizationFlag | kConstantOutputForConstantInput_OptimizationFlag
}

void main() {
    sk_OutColor = sk_InColor;
    sk_OutColor.rgb *= sk_InColor.a;
}

@class {
    GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
        return input.premul();
    }
}