cbuffer _UniformBuffer : register(b0, space0)
{
    float _7_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float _32 = 1.0f - _7_unknownInput;
    float r = _32;
    float g = _7_unknownInput;
    return float4(_32, _7_unknownInput, 0.0f, 1.0f);
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
