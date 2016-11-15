#pragma once

#include <GUObject.h>

class DXBlob;


class Triangle : public GUObject {

	ID3D11Buffer					*vertexBuffer = nullptr;
	ID3D11InputLayout				*inputLayout = nullptr;

public:

	Triangle(ID3D11Device *device, DXBlob *vsBytecode);
	~Triangle();

	void render(ID3D11DeviceContext *context);
};