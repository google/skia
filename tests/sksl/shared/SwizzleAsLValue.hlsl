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
    color.x += ((_58.w <= 1.0f) ? _58.z : 0.0f);
    bool4 _72 = all(bool4(color.x == float4(1.0f, 1.0f, 0.25f, 1.0f).x, color.y == float4(1.0f, 1.0f, 0.25f, 1.0f).y, color.z == float4(1.0f, 1.0f, 0.25f, 1.0f).z, color.w == float4(1.0f, 1.0f, 0.25f, 1.0f).w)).xxxx;
    return float4(_72.x ? _10_colorGreen.x : _10_colorRed.x, _72.y ? _10_colorGreen.y : _10_colorRed.y, _72.z ? _10_colorGreen.z : _10_colorRed.z, _72.w ? _10_colorGreen.w : _10_colorRed.w);
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
