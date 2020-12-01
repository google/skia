layout(set=0) uniform half zoom;

void main() {
    sk_Position = half4(1);
    if (zoom == 1) return;
    sk_Position *= zoom;
}
