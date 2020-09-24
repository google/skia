#version 400
uniform vec4 sk_RTAdjust;
layout (points) in ;
layout (invocations = 2) in ;
layout (line_strip, max_vertices = 2) out ;
void main() {
    gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, float(gl_InvocationID));
    {
        gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
        EmitVertex();
    }
    gl_Position = gl_in[0].gl_Position + vec4(0.5, 0.0, 0.0, float(gl_InvocationID));
    {
        gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
        EmitVertex();
    }
    EndPrimitive();
}
