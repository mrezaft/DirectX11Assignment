
//
// DXModel.h
//

// Version 1.  Encapsulate the mesh contents of a CGModel imported via CGImport3.  Currently supports obj, 3ds or gsf files.  md2, md3 and md5 (CGImport4) untested.  For version 1 a single texture and sampler interface are associated with the DXModel.


#pragma once

#include <d3d11_2.h>
#include <DXBaseModel.h>
#include <string>
#include <vector>
#include <cstdint>

class DXBlob;

class DXModel : public DXBaseModel {

	uint32_t							numMeshes = 0;
	std::vector<uint32_t>				indexCount;
	std::vector<uint32_t>				baseVertexOffset;
	
	ID3D11ShaderResourceView			*textureResourceView = nullptr;
	ID3D11SamplerState					*sampler = nullptr;

public:

	DXModel(ID3D11Device *device, DXBlob *vsBytecode, const std::wstring& filename, ID3D11ShaderResourceView *tex_view, DirectX::PackedVector::XMCOLOR diffuse, DirectX::PackedVector::XMCOLOR specular);
	~DXModel();

	void render(ID3D11DeviceContext *context);
};
