cbuffer TransformBuffer : register(b0)
{
    matrix transform;
};

struct VS_IN
{
    float4 pos : POSITION;
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

    output.pos = mul(input.pos, transform);
    output.col = input.col;

    return output;
}

float4 PSMain(PS_IN input) : SV_TARGET
{
    return input.col;
}