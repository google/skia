cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorBlack : packoffset(c1);
    float4 _7_colorGreen : packoffset(c2);
    float4 _7_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 _RESERVED_IDENTIFIER_FIXUP_0_v = _7_testInputs;
    int4 _44 = int4(int(_7_colorBlack.x), int(_7_colorBlack.y), int(_7_colorBlack.z), int(_7_colorBlack.w));
    int4 _RESERVED_IDENTIFIER_FIXUP_1_i = _44;
    float _48 = _7_testInputs[_44.x];
    float _RESERVED_IDENTIFIER_FIXUP_2_x = _48;
    float _51 = _7_testInputs[_44.y];
    float _RESERVED_IDENTIFIER_FIXUP_3_y = _51;
    float _54 = _7_testInputs[_44.z];
    float _RESERVED_IDENTIFIER_FIXUP_4_z = _54;
    float _57 = _7_testInputs[_44.w];
    float _RESERVED_IDENTIFIER_FIXUP_5_w = _57;
    float4 _58 = float4(_48, _51, _54, _57);
    float4 _65 = 0.0f.xxxx;
    if (all(bool4(_58.x == float4(-1.25f, -1.25f, -1.25f, 0.0f).x, _58.y == float4(-1.25f, -1.25f, -1.25f, 0.0f).y, _58.z == float4(-1.25f, -1.25f, -1.25f, 0.0f).z, _58.w == float4(-1.25f, -1.25f, -1.25f, 0.0f).w)))
    {
        _65 = _7_colorGreen;
    }
    else
    {
        _65 = _7_colorRed;
    }
    return _65;
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
