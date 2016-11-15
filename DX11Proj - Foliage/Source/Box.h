#pragma once

#include <GUObject.h>

class DXBlob;


class Box : public GUObject {

	ID3D11Buffer					*vertexBuffer = nullptr;
	ID3D11Buffer					*indexBuffer = nullptr;
	ID3D11InputLayout				*inputLayout = nullptr;
	// Augment box with texture view
	ID3D11ShaderResourceView			*textureResourceView = nullptr;
	ID3D11SamplerState					*sampler = nullptr;
public:

	Box(ID3D11Device *device, DXBlob *vsBytecode, ID3D11ShaderResourceView *tex_view);
	~Box();

	void render(ID3D11DeviceContext *context);
};