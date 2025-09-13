cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float sumA = 0.0f;
    float sumB = 0.0f;
    float a = 0.0f;
    float b = 10.0f;
    for (;;)
    {
        bool _46 = false;
        if (a < 10.0f)
        {
            _46 = b > 0.0f;
        }
        else
        {
            _46 = false;
        }
        if (_46)
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
    bool _67 = false;
    if (sumA != 45.0f)
    {
        _67 = true;
    }
    else
    {
        _67 = sumB != 55.0f;
    }
    if (_67)
    {
        return _11_colorRed;
    }
    int sumC = 0;
    for (int c = 0; c < 10; c++)
    {
        sumC += c;
    }
    if (sumC != 45)
    {
        return _11_colorRed;
    }
    float sumE = 0.0f;
    float _103[2] = { 0.0f, 10.0f };
    float d[2] = _103;
    float _111[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float e[4] = _111;
    for (; d[0] < d[1]; d[0] += 1.0f)
    {
        sumE += e[0];
    }
    if (sumE != 10.0f)
    {
        return _11_colorRed;
    }
    for (;;)
    {
        break;
    }
    for (;;)
    {
        return _11_colorGreen;
    }
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
