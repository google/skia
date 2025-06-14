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

float4 main(float2 _80)
{
    scratchVar = 0;
    bool _86 = false;
    if (test_flat_b())
    {
        bool _85 = test_if_b();
        _86 = _85;
    }
    else
    {
        _86 = false;
    }
    bool _90 = false;
    if (_86)
    {
        _90 = test_else_b();
    }
    else
    {
        _90 = false;
    }
    bool _94 = false;
    if (_90)
    {
        bool _93 = test_loop_if_b();
        _94 = _93;
    }
    else
    {
        _94 = false;
    }
    float4 _95 = 0.0f.xxxx;
    if (_94)
    {
        _95 = _18_colorGreen;
    }
    else
    {
        _95 = _18_colorRed;
    }
    return _95;
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
