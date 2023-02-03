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
    color.y *= 4.0f;
    float3 _48 = color.yzw * 0.5f.xxx;
    color = float4(color.x, _48.x, _48.y, _48.z);
    float4 _51 = color;
    float4 _56 = _51.zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    float4 _57 = color;
    float4 _58 = float4(_56.w, _56.y, _56.x, _56.z);
    color = _58;
    float4 _64 = 0.0f.xxxx;
    if (all(bool4(_58.x == float4(0.75f, 1.0f, 0.25f, 1.0f).x, _58.y == float4(0.75f, 1.0f, 0.25f, 1.0f).y, _58.z == float4(0.75f, 1.0f, 0.25f, 1.0f).z, _58.w == float4(0.75f, 1.0f, 0.25f, 1.0f).w)))
    {
        _64 = _10_colorGreen;
    }
    else
    {
        _64 = _10_colorRed;
    }
    return _64;
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
