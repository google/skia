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
    int _31 = int(_7_colorGreen.y);
    int integerInput = _31;
    if (_31 == 0)
    {
        return 2.1400001049041748046875f.xxxx;
    }
    else
    {
        if (_31 == 1)
        {
            return _7_colorGreen;
        }
        else
        {
            if (_31 == 2)
            {
                return float4(1.0f, 0.20000000298023223876953125f, 2.1400001049041748046875f, 1.0f);
            }
            else
            {
                if (3.1400001049041748046875f < (_7_colorGreen.x * 3.1400001049041748046875f))
                {
                    return 3.1400001049041748046875f.xxxx;
                }
                else
                {
                    if (2.1400001049041748046875f >= (_7_colorGreen.x * 2.1400001049041748046875f))
                    {
                        return 0.0f.xxxx;
                    }
                    else
                    {
                        return float4(1.0f, 0.0f, 0.0f, 1.0f);
                    }
                }
            }
        }
    }
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
