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
    int _33 = int(_11_unknownInput);
    int i = _33;
    int4 i4 = _33.xxxx;
    i4 = int4(_33.xx, 0, 1);
    i4 = int4(0, _33, 1, 0);
    int4 _45 = int4(0, _33, 0, _33);
    i4 = _45;
    return float4(float(0), float(_45.y), float(_45.z), float(_45.w));
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
