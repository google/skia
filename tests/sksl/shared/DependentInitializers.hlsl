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
    float x = 0.5f;
    float _31 = 0.5f * 2.0f;
    float y = _31;
    bool4 _35 = (_31 == 1.0f).xxxx;
    return float4(_35.x ? _10_colorGreen.x : _10_colorRed.x, _35.y ? _10_colorGreen.y : _10_colorRed.y, _35.z ? _10_colorGreen.z : _10_colorRed.z, _35.w ? _10_colorGreen.w : _10_colorRed.w);
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
