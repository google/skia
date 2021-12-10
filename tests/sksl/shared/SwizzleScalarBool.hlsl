cbuffer _UniformBuffer : register(b0, space0)
{
    float _10_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool b = _10_unknownInput != 0.0f;
    bool4 b4 = b.xxxx;
    b4 = bool4(b.xx, false, true);
    b4 = bool4(false, b, true, false);
    b4 = bool4(false, b, false, b);
    return float4(float(b4.x), float(b4.y), float(b4.z), float(b4.w));
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
