Texture2D baseColorTexture : register(t0);
SamplerState baseColorSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput psInput) : SV_TARGET
{
    return baseColorTexture.Sample(baseColorSampler, psInput.uv);
}