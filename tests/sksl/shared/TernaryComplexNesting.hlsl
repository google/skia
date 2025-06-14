cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorWhite : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool IsEqual_bh4h4(float4 _28, float4 _29)
{
    return all(bool4(_28.x == _29.x, _28.y == _29.y, _28.z == _29.z, _28.w == _29.w));
}

float4 main(float2 _37)
{
    float4 _47 = float4(0.0f, 0.0f, _12_colorWhite.zw);
    float4 colorBlue = _47;
    float4 _55 = float4(0.0f, _12_colorWhite.y, 0.0f, _12_colorWhite.w);
    float4 colorGreen = _55;
    float4 _63 = float4(_12_colorWhite.x, 0.0f, 0.0f, _12_colorWhite.w);
    float4 colorRed = _63;
    float4 _68 = _12_colorWhite;
    float4 _69 = _47;
    float4 _71 = 0.0f.xxxx;
    if (!IsEqual_bh4h4(_68, _69))
    {
        float4 _75 = _55;
        float4 _76 = _63;
        float4 _78 = 0.0f.xxxx;
        if (IsEqual_bh4h4(_75, _76))
        {
            _78 = _63;
        }
        else
        {
            _78 = _55;
        }
        _71 = _78;
    }
    else
    {
        float4 _84 = _63;
        float4 _85 = _55;
        float4 _87 = 0.0f.xxxx;
        if (!IsEqual_bh4h4(_84, _85))
        {
            _87 = _47;
        }
        else
        {
            _87 = _12_colorWhite;
        }
        _71 = _87;
    }
    float4 result = _71;
    float4 _95 = _63;
    float4 _96 = _47;
    float4 _98 = 0.0f.xxxx;
    if (IsEqual_bh4h4(_95, _96))
    {
        _98 = _12_colorWhite;
    }
    else
    {
        float4 _105 = _63;
        float4 _106 = _55;
        float4 _108 = 0.0f.xxxx;
        if (!IsEqual_bh4h4(_105, _106))
        {
            _108 = _71;
        }
        else
        {
            float4 _112 = _63;
            float4 _115 = _12_colorWhite;
            float4 _117 = 0.0f.xxxx;
            if (IsEqual_bh4h4(_112, _115))
            {
                _117 = _47;
            }
            else
            {
                _117 = _63;
            }
            _108 = _117;
        }
        _98 = _108;
    }
    return _98;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
