cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_sk_RTAdjust : packoffset(c0);
};


static float4 gl_Position;
struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = 1.0f.xxxx;
    gl_Position = float4((gl_Position.xy * _8_sk_RTAdjust.xz) + (gl_Position.ww * _8_sk_RTAdjust.yw), 0.0f, gl_Position.w);
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
