
#define D3DLIGHT_POINT 1
#define D3DLIGHT_SPOT 2
#define D3DLIGHT_DIRECTIONAL 3

struct Light
{
	uint   Type;         /* Type of light source */
	float4 Diffuse;      /* Diffuse color of light */
	float4 Specular;     /* Specular color of light */
	float4 Ambient;      /* Ambient color of light */
	float3 Position;     /* Position in world space */
	float3 Direction;    /* Direction in world space */
	float  Range;        /* Cutoff range */
	float  Falloff;      /* Falloff */
	float  Attenuation0; /* Constant attenuation */
	float  Attenuation1; /* Linear attenuation */
	float  Attenuation2; /* Quadratic attenuation */
	float  Theta;        /* Inner angle of spotlight cone */
	float  Phi;          /* Outer angle of spotlight cone */
};

struct VS_INPUT
{
#ifdef FVF_RHW
	float4 position : POSITIONT;
#else
	float4 position : POSITION;
#endif

#ifdef FVF_NORMAL
	float3 normal   : NORMAL;
#endif

#ifdef FVF_DIFFUSE
	float4 diffuse   : COLOR;
#endif

#ifdef FVF_TEX1
	float2 tex      : TEXCOORD;
#endif
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

#ifdef FVF_DIFFUSE
	float4 diffuse   : COLOR;
#endif

#ifdef FVF_TEX1
	float2 tex      : TEXCOORD;
#endif
};

static const matrix textureTransform = {
	-0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f
};

cbuffer PerSceneBuffer : register(b0)
{
	matrix viewMatrix;
	matrix projectionMatrix;
	float2 screenDimensions;
};

cbuffer PerModelBuffer : register(b1)
{
	matrix worldMatrix;
	Light light;
};

Texture2D<float4> DiffuseMap : register(t0);
SamplerState DiffuseSampler : register(s0);

VS_OUTPUT vs_main(VS_INPUT input)
{
	VS_OUTPUT result;

#ifdef FVF_RHW
	float4 p = input.position;
	float2 screen = screenDimensions - 0.5;

	p.x = ((p.x / screen.x) * 2) - 1;
	p.y = -(((p.y / screen.y) * 2) - 1);

	result.position = p;
#else
	input.position.w = 1;
	result.position = input.position;

	result.position = mul(worldMatrix, result.position);
	result.position = mul(viewMatrix, result.position);
	result.position = mul(projectionMatrix, result.position);
#endif

#ifdef FVF_DIFFUSE
	result.diffuse = input.diffuse;
#else
	result.diffuse = float4(1, 1, 1, 1);
#endif

#if defined(FVF_NORMAL) && !defined(FVF_RHW)
	float4 diff = result.diffuse;
	float3 n = mul((float3x3)worldMatrix, input.normal);
	float d = saturate(dot(normalize(light.Direction), n));

	diff.rgb *= saturate((d * light.Diffuse) + light.Ambient);
	diff.rgb = saturate(diff.rgb);

	result.diffuse = diff;
#endif

#ifdef FVF_TEX1
	#ifdef TCI_CAMERASPACENORMAL
		// this is kinda gross but the preshader should handle it
		// also this is producing incorrect output when the object rotates
		matrix wvMatrixInvT = mul(transpose(worldMatrix), transpose(viewMatrix));
		result.tex = (float2)mul(wvMatrixInvT, float4(input.normal, 1));
		result.tex = (float2)mul(float4(result.tex, 0, 1), textureTransform);
	#else
		result.tex = input.tex;
	#endif
#endif

	return result;
}

float4 ps_main(VS_OUTPUT input) : SV_TARGET
{
	float4 result;

#ifdef FVF_TEX1
	result = DiffuseMap.Sample(DiffuseSampler, input.tex);
#else
	result = float4(1, 1, 1, 1);
#endif

#ifdef FVF_DIFFUSE
	result *= input.diffuse;
#endif

	return result;
}
