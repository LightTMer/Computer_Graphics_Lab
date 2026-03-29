cbuffer CB : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
};

struct VS_IN
{
    float3 pos : POSITION;
    float4 col : COLOR;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output;

    float4 p = float4(input.pos, 1.0f);

    p = mul(p, world);
    p = mul(p, view);
    p = mul(p, proj);

    output.pos = p;
    output.col = input.col;

    return output;
}

float4 PSMain(PS_IN input) : SV_TARGET
{
    return input.col;
}