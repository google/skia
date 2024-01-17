cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float sumA = 0.0f;
    float sumB = 0.0f;
    float a = 0.0f;
    float b = 10.0f;
    for (;;)
    {
        bool _42 = false;
        if (a < 10.0f)
        {
            _42 = b > 0.0f;
        }
        else
        {
            _42 = false;
        }
        if (_42)
        {
            sumA += a;
            sumB += b;
            a += 1.0f;
            b -= 1.0f;
            continue;
        }
        else
        {
            break;
        }
    }
    bool _63 = false;
    if (sumA != 45.0f)
    {
        _63 = true;
    }
    else
    {
        _63 = sumB != 55.0f;
    }
    if (_63)
    {
        return _7_colorRed;
    }
    int sumC = 0;
    for (int c = 0; c < 10; c++)
    {
        sumC += c;
    }
    if (sumC != 45)
    {
        return _7_colorRed;
    }
    float sumE = 0.0f;
    float _100[2] = { 0.0f, 10.0f };
    float d[2] = _100;
    float _108[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float e[4] = _108;
    for (; d[0] < d[1]; d[0] += 1.0f)
    {
        sumE += e[0];
    }
    if (sumE != 10.0f)
    {
        return _7_colorRed;
    }
    for (;;)
    {
        break;
    }
    for (;;)
    {
        return _7_colorGreen;
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
