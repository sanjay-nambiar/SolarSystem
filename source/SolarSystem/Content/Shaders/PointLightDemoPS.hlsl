cbuffer CBufferPerFrame
{
	float3 AmbientColor;
	float3 LightPosition;
	float3 LightColor;
};

cbuffer CBufferPerObject
{
	float LightingCoefficient;
}

Texture2D ColorMap;
SamplerState TextureSampler;

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float Attenuation : ATTENUATION;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;	
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float3 lightDirection = normalize(LightPosition - IN.WorldPosition);

	float3 normal = normalize(IN.Normal);
	float n_dot_l = dot(normal, lightDirection);

	float4 color = ColorMap.Sample(TextureSampler, IN.TextureCoordinate);

	float3 ambient = color.rgb * AmbientColor;
	float3 diffuse = color.rgb * n_dot_l * LightColor * IN.Attenuation;
	float3 textureColor = color.rgb * (1 - LightingCoefficient);

	return float4(saturate(ambient + diffuse + textureColor), color.a);
}
