
#include <stdafx.h>
#include <Grid.h>
#include <DXVertexExt.h>
#include <iostream>
#include <exception>
#include <DXBlob.h>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;





Grid::Grid(ID3D11Device *device, DXBlob *vsBytecode, ID3D11ShaderResourceView *tex_view) {
	diffuse = XMCOLOR(1.0f, 1.0f, 1.0f, 1.0f);	// BGRA
	spec = XMCOLOR(0.0f, 0.0f, 0.0f, 0.0f);// specular power = a * 1000.0
	try
	{

		// Setup grid vertex buffer

		//INITIALISE Verticies
		for (int i = 0; i<W_HEIGHT; i++)
		{
			for (int j = 0; j<W_WIDTH; j++)
			{
				vertices[(i*W_WIDTH) + j].pos.x = (j - (W_WIDTH / 2)) / 10.0;
				vertices[(i*W_WIDTH) + j].pos.z = (i - (W_HEIGHT / 2)) / 10.0;
				vertices[(i*W_WIDTH) + j].pos.y = 0;
				vertices[(i*W_WIDTH) + j].normal=XMFLOAT3(0, 1, 0);
				vertices[(i*W_WIDTH) + j].matDiffuse = diffuse;
				vertices[(i*W_WIDTH) + j].matSpecular = spec;
				vertices[(i*W_WIDTH) + j].texCoord.x = (float)j / W_WIDTH;
				vertices[(i*W_WIDTH) + j].texCoord.y = (float)i / W_HEIGHT;
			}
		}


		if (!device || !vsBytecode)
			throw exception("Invalid parameters for triangle model instantiation");

		// Setup vertex buffer
		D3D11_BUFFER_DESC vertexDesc;
		D3D11_SUBRESOURCE_DATA vertexData;

		ZeroMemory(&vertexDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));

		vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDesc.ByteWidth = sizeof(DXVertexExt) * W_WIDTH*W_HEIGHT;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexData.pSysMem = vertices;

		HRESULT hr = device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);

		if (!SUCCEEDED(hr))
			throw exception("Vertex buffer cannot be created");


		for (int i = 0; i<W_HEIGHT - 1; i++)
		{
			for (int j = 0; j<W_WIDTH - 1; j++)
			{
				indices[(i*(W_WIDTH - 1) * 2 * 3) + (j * 2 * 3) + 0] = (i*W_WIDTH) + j;
				indices[(i*(W_WIDTH - 1) * 2 * 3) + (j * 2 * 3) + 2] = (i*W_WIDTH) + j + 1;
				indices[(i*(W_WIDTH - 1) * 2 * 3) + (j * 2 * 3) + 1] = ((i + 1)*W_WIDTH) + j;

				indices[(i*(W_WIDTH - 1) * 2 * 3) + (j * 2 * 3) + 3] = (i*W_WIDTH) + j + 1;
				indices[(i*(W_WIDTH - 1) * 2 * 3) + (j * 2 * 3) + 5] = ((i + 1)*W_WIDTH) + j + 1;;
				indices[(i*(W_WIDTH - 1) * 2 * 3) + (j * 2 * 3) + 4] = ((i + 1)*W_WIDTH) + j;



			}
		}


		D3D11_BUFFER_DESC indexDesc;
		indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexDesc.ByteWidth = sizeof(UINT) * N_W_IND;
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDesc.CPUAccessFlags = 0;
		indexDesc.MiscFlags = 0;
		indexDesc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA indexData;
		indexData.pSysMem = indices;
		
		hr = device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);
		
		if (!SUCCEEDED(hr))
			throw exception("Vertex buffer cannot be created");


		// Build the vertex input layout - this is done here since each object may load it's data into the IA differently.  This requires the compiled vertex shader bytecode.
		hr = DXVertexExt::createInputLayout(device, vsBytecode, &inputLayout);

		if (!SUCCEEDED(hr))
			throw exception("Cannot create input layout interface");


		textureResourceView = tex_view;

		if (textureResourceView)
			textureResourceView->AddRef(); // We didnt create it here but dont want it deleted by the creator untill we have deconstructed

		D3D11_SAMPLER_DESC samplerDesc;

		ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;

		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy=16;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = 0.0f;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

		hr = device->CreateSamplerState(&samplerDesc, &linearSampler);

	


	}
	catch (exception& e)
	{
		cout << "Grid object could not be instantiated due to:\n";
		cout << e.what() << endl;

		if (vertexBuffer)
			vertexBuffer->Release();

		if (inputLayout)
			inputLayout->Release();

		vertexBuffer = nullptr;
		inputLayout = nullptr;
		indexBuffer = nullptr;
	}
}


Grid::~Grid() {

	if (vertexBuffer)
		vertexBuffer->Release();
	if (indexBuffer)
		indexBuffer->Release();
	if (inputLayout)
		inputLayout->Release();

	if (textureResourceView)
		textureResourceView->Release();

	if (linearSampler)
		linearSampler->Release();
}


void Grid::render(ID3D11DeviceContext *context) {

	// Validate object before rendering (see notes in constructor)
	if (!context || !vertexBuffer || !inputLayout)
		return;

	// Set vertex layout
	context->IASetInputLayout(inputLayout);

	// Set vertex and index buffers for IA
	ID3D11Buffer* vertexBuffers[] = { vertexBuffer };
	UINT vertexStrides[] = { sizeof(DXVertexExt) };
	UINT vertexOffsets[] = { 0 };

	context->IASetVertexBuffers(0, 1, vertexBuffers, vertexStrides, vertexOffsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology for IA
	
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	
	// Bind texture resource views and texture sampler objects to the PS stage of the pipeline
	if (textureResourceView && linearSampler) {

		context->PSSetShaderResources(0, 1, &textureResourceView);
		context->PSSetSamplers(0, 1, &linearSampler);

	}

	// Draw grid object using index buffer
	// 36 indices for the grid.
	context->DrawIndexed(N_W_IND, 0, 0);
}

