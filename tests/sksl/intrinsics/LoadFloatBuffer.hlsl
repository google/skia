RWByteAddressBuffer _4 : register(u0, space0);

static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void avoidInline_vf(out float _19)
{
    _19 = asfloat(_4.Load(0));
}

float4 main()
{
    float f = 0.0f;
    float _31 = 0.0f;
    avoidInline_vf(_31);
    f = _31;
    return _31.xxxx;
}

void frag_main()
{
    sk_FragColor = main();
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
