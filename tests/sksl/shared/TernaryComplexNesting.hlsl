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
        float4 _77 = 0.0f.xxxx;
        if (IsEqual_bh4h4(_74, _75))
        {
            _77 = _62;
        }
        else
        {
            _77 = _54;
        }
        _70 = _77;
    }
    else
    {
        float4 _83 = _62;
        float4 _84 = _54;
        float4 _86 = 0.0f.xxxx;
        if (!IsEqual_bh4h4(_83, _84))
        {
            _86 = _46;
        }
        else
        {
            _86 = _11_colorWhite;
        }
        _70 = _86;
    }
    float4 result = _70;
    float4 _94 = _62;
    float4 _95 = _46;
    float4 _97 = 0.0f.xxxx;
    if (IsEqual_bh4h4(_94, _95))
    {
        _97 = _11_colorWhite;
    }
    else
    {
        float4 _104 = _62;
        float4 _105 = _54;
        float4 _107 = 0.0f.xxxx;
        if (!IsEqual_bh4h4(_104, _105))
        {
            _107 = _70;
        }
        else
        {
            float4 _111 = _62;
            float4 _114 = _11_colorWhite;
            float4 _116 = 0.0f.xxxx;
            if (IsEqual_bh4h4(_111, _114))
            {
                _116 = _46;
            }
            else
            {
                _116 = _62;
            }
            _107 = _116;
        }
        _97 = _107;
    }
    return _97;
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
