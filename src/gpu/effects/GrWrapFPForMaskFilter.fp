in fragmentProcessor shader_fp;

void main() {
    half4 dst = process(shader_fp);
    sk_OutColor = sk_InColor * dst.a;
}
