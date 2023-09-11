cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
    float4 _7_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 _RESERVED_IDENTIFIER_FIXUP_0_v = _7_testInputs;
    float _RESERVED_IDENTIFIER_FIXUP_1_x = _7_testInputs.x;
    float _RESERVED_IDENTIFIER_FIXUP_2_y = _7_testInputs.y;
    float _RESERVED_IDENTIFIER_FIXUP_3_z = _7_testInputs.z;
    float _RESERVED_IDENTIFIER_FIXUP_4_w = _7_testInputs.w;
    float4 _40 = float4(_7_testInputs);
    float4 a = _40;
    float _RESERVED_IDENTIFIER_FIXUP_9_x = _7_testInputs.x;
    float _RESERVED_IDENTIFIER_FIXUP_10_y = _7_testInputs.y;
    float _RESERVED_IDENTIFIER_FIXUP_11_z = _7_testInputs.z;
    float _RESERVED_IDENTIFIER_FIXUP_12_w = _7_testInputs.w;
    float4 _58 = float4(_7_testInputs.x, _7_testInputs.y, _7_testInputs.z, _7_testInputs.w);
    float4 b = _58;
    float4 c = float4(0.0f, 1.0f, 2.0f, 3.0f);
    bool _77 = false;
    if (all(bool4(_40.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, _40.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, _40.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, _40.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w)))
    {
        _77 = all(bool4(_58.x == float4(-1.25f, 0.0f, 0.75f, 2.25f).x, _58.y == float4(-1.25f, 0.0f, 0.75f, 2.25f).y, _58.z == float4(-1.25f, 0.0f, 0.75f, 2.25f).z, _58.w == float4(-1.25f, 0.0f, 0.75f, 2.25f).w));
    }
    else
    {
        _77 = false;
    }
    bool _81 = false;
    if (_77)
    {
        _81 = true;
    }
    else
    {
        _81 = false;
    }
    float4 _82 = 0.0f.xxxx;
    if (_81)
    {
        _82 = _7_colorGreen;
    }
    else
    {
        _82 = _7_colorRed;
    }
    return _82;
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
