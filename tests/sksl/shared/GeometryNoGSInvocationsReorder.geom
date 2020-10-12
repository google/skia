/*#pragma settings NoGSInvocationsSupport*/

layout(points) in;
layout(line_strip, max_vertices = 2) out;
layout(invocations = 2) in;

void test() {
    sk_Position = sk_in[0].sk_Position + float4(0.5, 0, 0, sk_InvocationID);
    EmitVertex();
}

void main() {
    test();
    sk_Position = sk_in[0].sk_Position + float4(-0.5, 0, 0, sk_InvocationID);
    EmitVertex();
}
