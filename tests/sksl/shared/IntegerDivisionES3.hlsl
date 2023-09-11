cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    int _31 = int(_7_colorGreen.x);
    int zero = _31;
    int one = int(_7_colorGreen.y);
    for (int x = _31; x < 100; x++)
    {
        for (int y = one; y < 100; y++)
        {
            int _RESERVED_IDENTIFIER_FIXUP_0_x = x;
            int _RESERVED_IDENTIFIER_FIXUP_1_result = 0;
            while (_RESERVED_IDENTIFIER_FIXUP_0_x >= y)
            {
                _RESERVED_IDENTIFIER_FIXUP_1_result++;
                _RESERVED_IDENTIFIER_FIXUP_0_x -= y;
            }
            if ((x / y) != _RESERVED_IDENTIFIER_FIXUP_1_result)
            {
                return float4(1.0f, float(x) * 0.0039215688593685626983642578125f, float(y) * 0.0039215688593685626983642578125f, 1.0f);
            }
        }
    }
    return _7_colorGreen;
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
