
//
// DXVertexExt.h
//

// Extended vertex structure

#pragma once

#include <d3d11_2.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

class DXBlob;

struct DXVertexExt  {

	DirectX::XMFLOAT3					pos;
	DirectX::XMFLOAT3					normal;
	DirectX::PackedVector::XMCOLOR		matDiffuse;
	DirectX::PackedVector::XMCOLOR		matSpecular;
	DirectX::XMFLOAT2					texCoord;

	// Create an input layout object mapping the vertex structure to the vertex shader input defined in the shader bytecode *shaderBlob
	static HRESULT createInputLayout(ID3D11Device *device, DXBlob *shaderBlob, ID3D11InputLayout **layout);
};
