
//
// Fire effect 
//

// Ensure matrices are row-major
#pragma pack_matrix(row_major)


//-----------------------------------------------------------------
// Structures and resources
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Globals
//-----------------------------------------------------------------

cbuffer basicCBuffer : register(b0) {

	float4x4			worldViewProjMatrix;
	float4x4			projIMatrix;				// Correctly transform normals to world space
	float4x4			worldMatrix;
	float4				eyePos;
	float4				windDir;
	float4				lightVec;					// w=1: Vec represents position, w=0: Vec  represents direction.
	float4				lightAmbient;
	float4				lightDiffuse;
	float4				lightSpecular;
	float				Timer;
};



//
// Textures
//

// Assumes texture bound to texture t0 and sampler bound to sampler s0
Texture2D fireTexture : register(t0);
Texture2D depth: register(t1);
SamplerState linearSampler : register(s0);




//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------

// Input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct FragmentInputPacket {

	float4 posH  : SV_POSITION;  // in clip space
	float2 texCoord  : TEXCOORD0;
	float alpha : ALPHA;
};


struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};


//-----------------------------------------------------------------
// Pixel Shader - Simple
//-----------------------------------------------------------------

FragmentOutputPacket main(FragmentInputPacket p) {

	FragmentOutputPacket outputFragment;

	float4 col = fireTexture.Sample(linearSampler, p.texCoord);
		outputFragment.fragmentColour = float4(col.xyz, p.alpha*(col.x + col.y + col.z) / 3);
	return outputFragment;
}
