
//
// Grass effect - Modified a fur technique
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
	float4x4			worldITMatrix;				// Correctly transform normals to world space
	float4x4			worldMatrix;
	float4				eyePos;
	float4				windDir;
	float4				lightVec;					// w=1: Vec represents position, w=0: Vec  represents direction.
	float4				lightAmbient;
	float4				lightDiffuse;
	float4				lightSpecular;
	float				Timer;
	float				grassHeight;
};





//
// Textures
//

// Assumes texture bound to texture t0 and sampler bound to sampler s0
Texture2D myTexture : register(t0);
Texture2D grassAlpha: register(t1);
SamplerState anisotropicSampler : register(s0);




//-----------------------------------------------------------------
// Input / Output structures
//-----------------------------------------------------------------

// Input fragment - this is the per-fragment packet interpolated by the rasteriser stage
struct FragmentInputPacket {

	// Vertex in world coords
	float3				posW			: POSITION;
	// Normal in world coords
	float3				normalW			: NORMAL;
	float4				matDiffuse		: DIFFUSE; // a represents alpha.
	float4				matSpecular		: SPECULAR; // a represents specular power. 
	float2				texCoord		: TEXCOORD;
	float4				posH			: SV_POSITION;
};


struct FragmentOutputPacket {

	float4				fragmentColour : SV_TARGET;
};


//-----------------------------------------------------------------
// Pixel Shader - Lighting 
//-----------------------------------------------------------------

FragmentOutputPacket main(FragmentInputPacket v) {

	FragmentOutputPacket outputFragment;
	float tileRepeat = 8;
	float3 colour=float3(0.0, 0.0, 0.0);
	float alpha = 1.0;
	float3 N = normalize(v.normalW);
	colour = v.matDiffuse.xyz;
	colour *= myTexture.Sample(anisotropicSampler, v.texCoord);

	if (grassHeight > 0.0)
	{
		// Calculate the lambertian term (essentially the brightness of the surface point based on the dot product of the normal vector with the vector pointing from v to the light source's location)
		float3 lightDir = -lightVec.xyz; // Directional light
		if (lightVec.w == 1.0) lightDir = lightVec.xyz - v.posW; // Positional light
		lightDir = normalize(lightDir);
		colour += max(dot(lightDir, N), 0.0f) *colour * lightDiffuse;

		alpha = grassAlpha.Sample(anisotropicSampler, v.texCoord*tileRepeat).a;
		// Reduce alpha and increase illumination for tips of grass
		colour *= 3* alpha+(grassHeight*10);// *(1 - alpha) * 3;
		alpha = (alpha - grassHeight * 40);
	}
	else
		colour *= float3(0.8, 0.8, 0.8);

	outputFragment.fragmentColour = float4(colour, alpha);
	return outputFragment;

}
