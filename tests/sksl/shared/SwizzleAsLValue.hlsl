cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 color = _10_colorGreen * 0.5f;
    color.w = 2.0f;
    color.y /= 0.25f;
    float3 _48 = color.yzw * 0.5f.xxx;
    color = float4(color.x, _48.x, _48.y, _48.z);
    float4 _51 = color;
    float4 _55 = _51.zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    float4 _56 = color;
    float4 _57 = float4(_55.w, _55.y, _55.x, _55.z);
    color = _57;
    float4 _63 = 0.0f.xxxx;
    if (all(bool4(_57.x == float4(0.75f, 1.0f, 0.25f, 1.0f).x, _57.y == float4(0.75f, 1.0f, 0.25f, 1.0f).y, _57.z == float4(0.75f, 1.0f, 0.25f, 1.0f).z, _57.w == float4(0.75f, 1.0f, 0.25f, 1.0f).w)))
    {
        _63 = _10_colorGreen;
    }
    else
    {
        _63 = _10_colorRed;
    }
    return _63;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
