cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorWhite : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool IsEqual_bh4h4(float4 _24, float4 _25)
{
    return all(bool4(_24.x == _25.x, _24.y == _25.y, _24.z == _25.z, _24.w == _25.w));
}

float4 main(float2 _33)
{
    float4 _44 = float4(0.0f, 0.0f, _8_colorWhite.zw);
    float4 colorBlue = _44;
    float4 _52 = float4(0.0f, _8_colorWhite.y, 0.0f, _8_colorWhite.w);
    float4 colorGreen = _52;
    float4 _60 = float4(_8_colorWhite.x, 0.0f, 0.0f, _8_colorWhite.w);
    float4 colorRed = _60;
    float4 _65 = _8_colorWhite;
    float4 _66 = _44;
    float4 _68 = 0.0f.xxxx;
    if (!IsEqual_bh4h4(_65, _66))
    {
        float4 _72 = _52;
        float4 _73 = _60;
        float4 _75 = 0.0f.xxxx;
        if (IsEqual_bh4h4(_72, _73))
        {
            _75 = _60;
        }
        else
        {
            _75 = _52;
        }
        _68 = _75;
    }
    else
    {
        float4 _81 = _60;
        float4 _82 = _52;
        float4 _84 = 0.0f.xxxx;
        if (!IsEqual_bh4h4(_81, _82))
        {
            _84 = _44;
        }
        else
        {
            _84 = _8_colorWhite;
        }
        _68 = _84;
    }
    float4 result = _68;
    float4 _92 = _60;
    float4 _93 = _44;
    float4 _95 = 0.0f.xxxx;
    if (IsEqual_bh4h4(_92, _93))
    {
        _95 = _8_colorWhite;
    }
    else
    {
        float4 _102 = _60;
        float4 _103 = _52;
        float4 _105 = 0.0f.xxxx;
        if (!IsEqual_bh4h4(_102, _103))
        {
            _105 = _68;
        }
        else
        {
            float4 _109 = _60;
            float4 _112 = _8_colorWhite;
            float4 _114 = 0.0f.xxxx;
            if (IsEqual_bh4h4(_109, _112))
            {
                _114 = _44;
            }
            else
            {
                _114 = _60;
            }
            _105 = _114;
        }
        _95 = _105;
    }
    return _95;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
