cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorBlack : packoffset(c1);
    float4 _11_colorGreen : packoffset(c2);
    float4 _11_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 _RESERVED_IDENTIFIER_FIXUP_0_v = _11_testInputs;
    int4 _47 = int4(int(_11_colorBlack.x), int(_11_colorBlack.y), int(_11_colorBlack.z), int(_11_colorBlack.w));
    int4 _RESERVED_IDENTIFIER_FIXUP_1_i = _47;
    float _51 = _11_testInputs[_47.x];
    float _RESERVED_IDENTIFIER_FIXUP_2_x = _51;
    float _54 = _11_testInputs[_47.y];
    float _RESERVED_IDENTIFIER_FIXUP_3_y = _54;
    float _57 = _11_testInputs[_47.z];
    float _RESERVED_IDENTIFIER_FIXUP_4_z = _57;
    float _60 = _11_testInputs[_47.w];
    float _RESERVED_IDENTIFIER_FIXUP_5_w = _60;
    float4 _61 = float4(_51, _54, _57, _60);
    float4 _68 = 0.0f.xxxx;
    if (all(bool4(_61.x == float4(-1.25f, -1.25f, -1.25f, 0.0f).x, _61.y == float4(-1.25f, -1.25f, -1.25f, 0.0f).y, _61.z == float4(-1.25f, -1.25f, -1.25f, 0.0f).z, _61.w == float4(-1.25f, -1.25f, -1.25f, 0.0f).w)))
    {
        _68 = _11_colorGreen;
    }
    else
    {
        _68 = _11_colorRed;
    }
    return _68;
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
