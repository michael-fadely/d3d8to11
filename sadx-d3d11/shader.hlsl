
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
	float4 vcolor   : COLOR;
#endif

#ifdef FVF_TEX1
	float2 tex      : TEXCOORD;
#endif
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;

#ifdef FVF_DIFFUSE
	float4 vcolor   : COLOR;
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
	result.vcolor = input.vcolor;
#endif

#ifdef FVF_TEX1
	#ifdef TCI_CAMERASPACENORMAL
		// don't actually need to transpose (T) because of the
		// multiplication order, but leaving T here for reasons.
		matrix wvMatrixInvT = mul(worldMatrix, viewMatrix);
		result.tex = (float2)mul(float4(input.normal, 1), wvMatrixInvT);
		result.tex = (float2)mul(textureTransform, float4(result.tex, 0, 1));
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
	result *= input.vcolor;
#endif

	return result;
}
