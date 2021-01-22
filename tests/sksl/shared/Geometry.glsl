#version 400
layout (points) in ;
layout (invocations = 2) in ;
layout (line_strip, max_vertices = 2) out ;
void main() {
    gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, float(gl_InvocationID));
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + vec4(0.5, 0.0, 0.0, float(gl_InvocationID));
    EmitVertex();
    EndPrimitive();
}
