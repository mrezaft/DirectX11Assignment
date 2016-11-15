#pragma once

#include <GUObject.h>
#define W_WIDTH 20
#define W_HEIGHT 100
#define N_W_IND ((W_WIDTH-1)*2*3)*(W_HEIGHT-1)
class DXBlob;


class Ocean : public GUObject {
	DirectX::PackedVector::XMCOLOR		diffuse;
	DirectX::PackedVector::XMCOLOR		spec;	

	// Create Water vertex buffer
	DXVertexExt vertices[W_WIDTH*W_HEIGHT];

	// Create the indices
	UINT indices[N_W_IND];

	ID3D11Buffer					*vertexBuffer = nullptr;
	ID3D11Buffer					*indexBuffer = nullptr;
	ID3D11InputLayout				*inputLayout = nullptr;
	// Augment ocean with texture view
	ID3D11ShaderResourceView			*textureResourceView = nullptr;
	ID3D11SamplerState					*normalMapSampler = nullptr;
	ID3D11SamplerState					*cubeMapSampler = nullptr;
public:

	Ocean(ID3D11Device *device, DXBlob *vsBytecode, ID3D11ShaderResourceView *tex_view);
	~Ocean();

	void render(ID3D11DeviceContext *context);
};