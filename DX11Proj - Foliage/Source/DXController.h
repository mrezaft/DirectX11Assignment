
//
// DXController.h
//

// DXController represents the controller tier of the MVC architecture pattern and references the main window (view), DirectX interfaces and scene variables (model)

#pragma once

#include <GUObject.h>
#include <Windows.h>
#include <buffers.h>
#include <Triangle.h>
#include <Box.h>
#include <Grid.h>
#include <Ocean.h>
#include <Particles.h>

class DXSystem;
class GUClock;
class DXModel;
class LookAtCamera;


// CBuffer struct
// Use 16byte aligned so can use optimised XMMathFunctions instead of setting _XM_NO_INNTRINSICS_ define when compiling for x86
__declspec(align(16)) struct CBufferExt  {  
	DirectX::XMMATRIX						WVPMatrix;
	DirectX::XMMATRIX						worldITMatrix; // Correctly transform normals to world space
	DirectX::XMMATRIX						worldMatrix;
	DirectX::XMFLOAT4						eyePos;
	DirectX::XMFLOAT4						windDir;
	// Simple single light source properties
	DirectX::XMFLOAT4						lightVec; // w=1: Vec represents position, w=0: Vec  represents direction.
	DirectX::XMFLOAT4						lightAmbient;
	DirectX::XMFLOAT4						lightDiffuse;
	DirectX::XMFLOAT4						lightSpecular; 
	FLOAT									Timer;
	FLOAT									grassHeight;

};


__declspec(align(16)) struct projMatrixStruct  {
	DirectX::XMMATRIX						projMatrix;
};
__declspec(align(16)) struct worldMatrixStruct  {
	DirectX::XMMATRIX						worldMatrix;
};
class DXController : public GUObject {

	HINSTANCE								hInst = NULL;
	HWND									wndHandle = NULL;

	// Strong reference to associated Direct3D device and rendering context.
	DXSystem								*dx = nullptr;

	// Default pipeline stage states
	ID3D11RasterizerState					*defaultRSstate = nullptr;
	ID3D11RasterizerState					*skyRSState = nullptr;
	ID3D11DepthStencilState					*defaultDSstate = nullptr;
	ID3D11BlendState						*defaultBlendState = nullptr;
	ID3D11BlendState						*treesBlendState = nullptr;
	ID3D11DepthStencilState					*fireDSstate = nullptr;
	ID3D11BlendState						*fireBlendState = nullptr;
	ID3D11VertexShader						*skyBoxVS = nullptr;
	ID3D11PixelShader						*skyBoxPS = nullptr;
	ID3D11VertexShader						*grassVS = nullptr;
	ID3D11PixelShader						*grassPS = nullptr;
	ID3D11VertexShader						*treeVS = nullptr;
	ID3D11PixelShader						*treePS = nullptr;
	ID3D11VertexShader						*oceanVS = nullptr;
	ID3D11PixelShader						*oceanPS = nullptr;
	ID3D11VertexShader						*reflectionMapVS = nullptr;
	ID3D11PixelShader						*reflectionMapPS = nullptr;
	ID3D11Buffer							*cBufferTree = nullptr;
	ID3D11Buffer							*cBufferSky = nullptr;
	ID3D11Buffer							*cBufferGrass = nullptr;
	ID3D11Buffer                            *cBufferWater = nullptr;
	ID3D11Buffer                            *cBufferCastle = nullptr;
	ID3D11VertexShader						*fireVS = nullptr;
	ID3D11PixelShader						*firePS = nullptr;
	ID3D11VertexShader						*perPixelLightingVS = nullptr;
	ID3D11PixelShader						*perPixelLightingPS = nullptr;
	ID3D11Buffer							*cBufferLogs = nullptr;
	ID3D11Buffer							*cBufferFire = nullptr;
	CBufferExt								*cBufferExtSrc = nullptr;

	// Main FPS clock
	GUClock									*mainClock = nullptr;

	
	LookAtCamera							*mainCamera = nullptr;
	projMatrixStruct 						*projMatrix = nullptr;
	worldMatrixStruct 						*treeInstance = nullptr;


	float									forestSize = 5.0f;
	float									grassLength = 0.005f;
	int										numGrassPasses = 40;
	// Direct3D scene objects
	Box										*skyBox = nullptr;
	Grid									*floor = nullptr;
	DXModel									*tree = nullptr;
	Ocean                                   *water = nullptr;
	Particles								*fire = nullptr;
	DXModel									*logs = nullptr;
	ID3D11SamplerState						*linearSampler = nullptr;
	DXModel                                  *castle = nullptr;



	// Instances of models that appear in the scene
	ID3D11Texture2D                          *CastleTexture = nullptr;
	ID3D11ShaderResourceView				*CastleTextureSRV = nullptr;

	ID3D11Texture2D							*treeTexture = nullptr;
	ID3D11ShaderResourceView				*treeTextureSRV = nullptr;
	ID3D11Texture2D							*cubeMapTexture = nullptr;
	ID3D11ShaderResourceView				*cubeMapTextureSRV = nullptr;
	ID3D11Texture2D							*grassDiffuseMap = nullptr;
	ID3D11ShaderResourceView				*grassDiffuseMapSRV = nullptr;
	ID3D11Texture2D							*grassAlphaMap = nullptr;
	ID3D11ShaderResourceView				*grassAlphaMapSRV = nullptr;


	ID3D11Texture2D							*grassHeightMap = nullptr;
	ID3D11ShaderResourceView				*grassHeightMapSRV = nullptr;
	ID3D11Texture2D							*grassNormalMap = nullptr;
	ID3D11ShaderResourceView				*grassNormalMapSRV = nullptr;



	ID3D11Texture2D							*waterNormalMap = nullptr;
	ID3D11ShaderResourceView				*waterNormalMapSRV = nullptr;
	
	
	//fire
	ID3D11Texture2D							*logsTexture = nullptr;
	ID3D11ShaderResourceView				*logsTextureSRV = nullptr;
	ID3D11Texture2D							*fireDiffuseMap = nullptr;
	ID3D11ShaderResourceView				*fireDiffuseMapSRV = nullptr;
	ID3D11Texture2D							*smokeDiffuseMap = nullptr;
	ID3D11ShaderResourceView				*smokeDiffuseMapSRV = nullptr;
	//
	// Private interface
	//

	// Private constructor
	DXController(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc);

	// Return TRUE if the window is in a minimised state, FALSE otherwise
	BOOL isMinimised();


public:

	//
	// Public interface
	//

	// Factory method to create the main DXController instance (singleton)
	static DXController* CreateDXController(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc);

	// Destructor
	~DXController();

	// Decouple the encapsulated HWND and call DestoryWindow on the HWND
	void destoryWindow();

	// Resize swap chain buffers and update pipeline viewport configurations in response to a window resize event
	HRESULT resizeResources();

	// Helper function to call updateScene followed by renderScene
	HRESULT updateAndRenderScene();
	HRESULT mapCbuffer(void *cBufferExtSrcL, ID3D11Buffer *cBufferExtL);
	// Clock handling methods
	void startClock();
	void stopClock();
	void reportTimingData();


	//
	// Event handling methods
	//

	// Process mouse move with the left button held down
	void handleMouseLDrag(const POINT &disp);

	// Process mouse wheel movement
	void handleMouseWheel(const short zDelta);

	// Process key down event.  keyCode indicates the key pressed while extKeyFlags indicates the extended key status at the time of the key down event (see http://msdn.microsoft.com/en-gb/library/windows/desktop/ms646280%28v=vs.85%29.aspx).
	void handleKeyDown(const WPARAM keyCode, const LPARAM extKeyFlags);
	
	// Process key up event.  keyCode indicates the key released while extKeyFlags indicates the extended key status at the time of the key up event (see http://msdn.microsoft.com/en-us/library/windows/desktop/ms646281%28v=vs.85%29.aspx).
	void handleKeyUp(const WPARAM keyCode, const LPARAM extKeyFlags);

	

	//
	// Methods to handle initialisation, update and rendering of the scene
	//
	HRESULT rebuildViewport();
	HRESULT initDefaultPipeline();
	HRESULT bindDefaultPipeline();
	HRESULT LoadShader(ID3D11Device *device, const char *filename, DXBlob **PSBytecode, ID3D11PixelShader **pixelShader);
	HRESULT LoadShader(ID3D11Device *device, const char *filename, DXBlob **VSBytecode, ID3D11VertexShader **vertexShader);
	HRESULT initialiseSceneResources();
	HRESULT updateScene();
	HRESULT renderScene();

};
