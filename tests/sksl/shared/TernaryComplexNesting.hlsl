cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorWhite : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool IsEqual_bh4h4(float4 _26, float4 _27)
{
    return all(bool4(_26.x == _27.x, _26.y == _27.y, _26.z == _27.z, _26.w == _27.w));
}

float4 main(float2 _35)
{
    float4 _46 = float4(0.0f, 0.0f, _11_colorWhite.zw);
    float4 colorBlue = _46;
    float4 _54 = float4(0.0f, _11_colorWhite.y, 0.0f, _11_colorWhite.w);
    float4 colorGreen = _54;
    float4 _62 = float4(_11_colorWhite.x, 0.0f, 0.0f, _11_colorWhite.w);
    float4 colorRed = _62;
    float4 _67 = _11_colorWhite;
    float4 _68 = _46;
    float4 _70 = 0.0f.xxxx;
    if (!IsEqual_bh4h4(_67, _68))
    {
        float4 _74 = _54;
        float4 _75 = _62;
        bool4 _77 = IsEqual_bh4h4(_74, _75).xxxx;
        _70 = float4(_77.x ? _62.x : _54.x, _77.y ? _62.y : _54.y, _77.z ? _62.z : _54.z, _77.w ? _62.w : _54.w);
    }
    else
    {
        float4 _80 = _62;
        float4 _81 = _54;
        bool4 _83 = (!IsEqual_bh4h4(_80, _81)).xxxx;
        _70 = float4(_83.x ? _46.x : _11_colorWhite.x, _83.y ? _46.y : _11_colorWhite.y, _83.z ? _46.z : _11_colorWhite.z, _83.w ? _46.w : _11_colorWhite.w);
    }
    float4 result = _70;
    float4 _88 = _62;
    float4 _89 = _46;
    float4 _91 = 0.0f.xxxx;
    if (IsEqual_bh4h4(_88, _89))
    {
        _91 = _11_colorWhite;
    }
    else
    {
        float4 _98 = _62;
        float4 _99 = _54;
        float4 _101 = 0.0f.xxxx;
        if (!IsEqual_bh4h4(_98, _99))
        {
            _101 = _70;
        }
        else
        {
            float4 _105 = _62;
            float4 _108 = _11_colorWhite;
            bool4 _110 = IsEqual_bh4h4(_105, _108).xxxx;
            _101 = float4(_110.x ? _46.x : _62.x, _110.y ? _46.y : _62.y, _110.z ? _46.z : _62.z, _110.w ? _46.w : _62.w);
        }
        _91 = _101;
    }
    return _91;
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
