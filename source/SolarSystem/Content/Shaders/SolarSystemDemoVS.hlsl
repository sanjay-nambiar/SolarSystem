cbuffer CBufferPerFrame
{
	float3 LightPosition;
	float LightIntensity;
}

cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection;
	float4x4 World;
}

struct VS_INPUT
{
	float4 ObjectPosition: POSITION;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float3 WorldPosition : WORLDPOS;
	float Attenuation : ATTENUATION;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;	
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
	OUT.WorldPosition = mul(IN.ObjectPosition, World).xyz;
	OUT.TextureCoordinate = IN.TextureCoordinate;
	OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);

	float3 lightDirection = LightPosition - OUT.WorldPosition;
	float lightDistance = length(lightDirection);
	OUT.Attenuation = saturate(LightIntensity/(lightDistance*lightDistance));
	return OUT;
}