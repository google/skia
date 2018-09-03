STRINGIFY(

// defines built-in interfaces supported by SkiaSL geometry shaders

layout(builtin=10002) in sk_PerVertex {
  layout(builtin=0) float4 sk_Position;
  layout(builtin=1) float sk_PointSize;
  layout(builtin=3) float sk_ClipDistance[1];
} sk_in[1];

layout(builtin=10007) out sk_PerVertex {
    layout(builtin=0) float4 sk_Position;
    layout(builtin=1) float sk_PointSize;
    layout(builtin=3) float sk_ClipDistance[1];
};

layout(builtin=8) in int sk_InvocationID;

sk_has_side_effects void EmitStreamVertex(int stream);
sk_has_side_effects void EndStreamPrimitive(int stream);
sk_has_side_effects void EmitVertex();
sk_has_side_effects void EndPrimitive();

)
