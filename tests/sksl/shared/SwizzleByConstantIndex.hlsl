cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 _RESERVED_IDENTIFIER_FIXUP_0_v = _11_testInputs;
    float _RESERVED_IDENTIFIER_FIXUP_1_x = _11_testInputs.x;
    float _RESERVED_IDENTIFIER_FIXUP_2_y = _11_testInputs.y;
    float _RESERVED_IDENTIFIER_FIXUP_3_z = _11_testInputs.z;
    float _RESERVED_IDENTIFIER_FIXUP_4_w = _11_testInputs.w;
    float4 _43 = float4(_11_testInputs);
    float4 a = _43;
    float _RESERVED_IDENTIFIER_FIXUP_9_x = _11_testInputs.x;
    float _RESERVED_IDENTIFIER_FIXUP_10_y = _11_testInputs.y;
    float _RESERVED_IDENTIFIER_FIXUP_11_z = _11_testInputs.z;
    float _RESERVED_IDENTIFIER_FIXUP_12_w = _11_testInputs.w;
    float4 _61 = float4(_11_testInputs.x, _11_testInputs.y, _11_testInputs.z, _11_testInputs.w);
    float4 b = _61;
    float4 c = float4(0.0f, 1.0f, 2.0f, 3.0f);
    bool _80 = false;
    if (all(bool4(_43.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, _43.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, _43.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, _43.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w)))
    {
        _80 = all(bool4(_61.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, _61.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, _61.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, _61.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w));
    }
    else
    {
        _80 = false;
    }
    bool _84 = false;
    if (_80)
    {
        _84 = true;
    }
    else
    {
        _84 = false;
    }
    float4 _85 = 0.0f.xxxx;
    if (_84)
    {
        _85 = _11_colorGreen;
    }
    else
    {
        _85 = _11_colorRed;
    }
    return _85;
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
