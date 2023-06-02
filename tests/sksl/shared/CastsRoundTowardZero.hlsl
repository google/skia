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
    bool ok = true;
    return float4(bool4(true, true, true, true).x ? _10_colorGreen.x : _10_colorRed.x, bool4(true, true, true, true).y ? _10_colorGreen.y : _10_colorRed.y, bool4(true, true, true, true).z ? _10_colorGreen.z : _10_colorRed.z, bool4(true, true, true, true).w ? _10_colorGreen.w : _10_colorRed.w);
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
