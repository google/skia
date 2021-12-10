cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _18_colorGreen : packoffset(c0);
    float4 _18_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static int scratchVar = 0;

bool test_flat_b()
{
    return true;
}

bool test_if_b()
{
    if (_18_colorGreen.y > 0.0f)
    {
        return true;
    }
    else
    {
        scratchVar++;
    }
    scratchVar++;
    return false;
}

bool test_else_b()
{
    if (_18_colorGreen.y == 0.0f)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool test_loop_if_b()
{
    for (int x = 0; x <= 1; x++)
    {
        if (_18_colorGreen.y == 0.0f)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    scratchVar++;
    return true;
}

float4 main(float2 _79)
{
    scratchVar = 0;
    bool _85 = false;
    if (test_flat_b())
    {
        bool _84 = test_if_b();
        _85 = _84;
    }
    else
    {
        _85 = false;
    }
    bool _89 = false;
    if (_85)
    {
        _89 = test_else_b();
    }
    else
    {
        _89 = false;
    }
    bool _93 = false;
    if (_89)
    {
        bool _92 = test_loop_if_b();
        _93 = _92;
    }
    else
    {
        _93 = false;
    }
    float4 _94 = 0.0f.xxxx;
    if (_93)
    {
        _94 = _18_colorGreen;
    }
    else
    {
        _94 = _18_colorRed;
    }
    return _94;
}

void frag_main()
{
    float2 _28 = 0.0f.xx;
    float4 _30 = main(_28);
    sk_FragColor = _30;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
