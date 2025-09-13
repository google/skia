cbuffer _UniformBuffer : register(b0, space0)
{
    float _11_unknownInput : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float _35 = 1.0f - _11_unknownInput;
    float r = _35;
    float g = _11_unknownInput;
    return float4(_35, _11_unknownInput, 0.0f, 1.0f);
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
