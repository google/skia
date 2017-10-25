layout(ctype=SkPMColor) in half4 color0;
layout(ctype=SkPMColor) in half4 color1;
layout(ctype=SkPMColor) in half4 color2;
layout(ctype=SkPMColor) in half4 color3;
layout(ctype=SkPMColor) in half4 color4;
layout(ctype=SkPMColor) in half4 color5;

void main() {
    half alpha = 255.0 * sk_InColor.a;
    if (alpha < 0.5) {
        sk_OutColor = color0;
    } else if (alpha < 1.5) {
        sk_OutColor = color1;
    } else if (alpha < 2.5) {
        sk_OutColor = color2;
    } else if (alpha < 3.5) {
        sk_OutColor = color3;
    } else if (alpha < 4.5) {
        sk_OutColor = color4;
    } else {
        sk_OutColor = color5;
    }
}