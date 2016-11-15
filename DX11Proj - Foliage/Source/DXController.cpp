
//
// DXController.cpp
//

#include <stdafx.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <DXController.h>
#include <DirectXMath.h>
#include <DXSystem.h>
#include <DirectXTK\DDSTextureLoader.h>
#include <DirectXTK\WICTextureLoader.h>
#include <GUClock.h>
#include <DXModel.h>
#include <LookAtCamera.h>
#define	NUM_TREES 10

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;
// Load the Compiled Shader Object (CSO) file 'filename' and return the bytecode in the blob object **bytecode.  This is used to create shader interfaces that require class linkage interfaces.
// Taken from DXShaderFactory by Paul Angel. This function has been included here for clarity.
void DXLoadCSO(const char *filename, DXBlob **bytecode)
{

	ifstream	*fp = nullptr;
	DXBlob		*memBlock = nullptr;
	

	try
	{
		// Validate parameters
		if (!filename || !bytecode)
			throw exception("loadCSO: Invalid parameters");

		// Open file
		fp = new ifstream(filename, ios::in | ios::binary);

		if (!fp->is_open())
			throw exception("loadCSO: Cannot open file");

		// Get file size
		fp->seekg(0, ios::end);
		uint32_t size = (uint32_t)fp->tellg();

		// Create blob object to store bytecode (exceptions propagate up if any occur)
		memBlock = new DXBlob(size);

		// Read binary data into blob object
		fp->seekg(0, ios::beg);
		fp->read((char*)(memBlock->getBufferPointer()), memBlock->getBufferSize());


		// Close file and release local resources
		fp->close();
		delete fp;

		// Return DXBlob - ownership implicity passed to caller
		*bytecode = memBlock;
	}
	catch (exception& e)
	{
		cout << e.what() << endl;

		// Cleanup local resources
		if (fp) {

			if (fp->is_open())
				fp->close();

			delete fp;
		}

		if (memBlock)
			delete memBlock;

		// Re-throw exception
		throw;
	}
}

// Helper Generates random number between -1.0 and +1.0
float randM1P1()
{	// use srand((unsigned int)time(NULL)); to seed rand()
	float r =(float) ((double)rand() / (double)(RAND_MAX)) * 2.0f - 1.0f;
	return r;
}
//
// Private interface implementation
//

// Private constructor
DXController::DXController(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc) {

	try
	{
		// 1. Register window class for main DirectX window
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_CROSS);
		wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = wndClassName;
		wcex.hIconSm = NULL;

		if (!RegisterClassEx(&wcex))
			throw exception("Cannot register window class for DXController HWND");

		
		// 2. Store instance handle in our global variable
		hInst = hInstance;


		// 3. Setup window rect and resize according to set styles
		RECT		windowRect;

		windowRect.left = 0;
		windowRect.right = _width;
		windowRect.top = 0;
		windowRect.bottom = _height;

		DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;

		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

		// 4. Create and validate the main window handle
		wndHandle = CreateWindowEx(dwExStyle, wndClassName, wndTitle, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 500, 500, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, hInst, this);

		if (!wndHandle)
			throw exception("Cannot create main window handle");

		ShowWindow(wndHandle, nCmdShow);
		UpdateWindow(wndHandle);
		SetFocus(wndHandle);


		// 5. Initialise render pipeline model (simply sets up an internal std::vector of pipeline objects)
	

		// 6. Create DirectX host environment (associated with main application wnd)
		dx = DXSystem::CreateDirectXSystem(wndHandle);

		if (!dx)
			throw exception("Cannot create Direct3D device and context model");

		// 7. Setup application-specific objects
		HRESULT hr = initialiseSceneResources();

		if (!SUCCEEDED(hr))
			throw exception("Cannot initalise scene resources");


		// 8. Create main clock / FPS timer (do this last with deferred start of 3 seconds so min FPS / SPF are not skewed by start-up events firing and taking CPU cycles).
		mainClock = GUClock::CreateClock(string("mainClock"), 3.0f);

		if (!mainClock)
			throw exception("Cannot create main clock / timer");

	}
	catch (exception &e)
	{
		cout << e.what() << endl;

		// Re-throw exception
		throw;
	}
	
}


// Return TRUE if the window is in a minimised state, FALSE otherwise
BOOL DXController::isMinimised() {

	WINDOWPLACEMENT				wp;

	ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);

	return (GetWindowPlacement(wndHandle, &wp) != 0 && wp.showCmd == SW_SHOWMINIMIZED);
}



//
// Public interface implementation
//

// Factory method to create the main DXController instance (singleton)
DXController* DXController::CreateDXController(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc) {

	static bool _controller_created = false;

	DXController *dxController = nullptr;

	if (!_controller_created) {

		dxController = new DXController(_width, _height, wndClassName, wndTitle, nCmdShow, hInstance, WndProc);

		if (dxController)
			_controller_created = true;
	}

	return dxController;
}


// Destructor
DXController::~DXController() {

	
	//Clean Up- release local interfaces
	// Release RSstate
	defaultRSstate->Release();
	// Release RSstate
	skyRSState->Release();
	// Release dsState
	defaultDSstate->Release();
	// Release blendState
	defaultBlendState->Release();
	// Release VertexShader interface
	skyBoxVS->Release();
	// Release PixelShader interface
	skyBoxPS->Release();


	// Release VertexShader interface
	grassVS->Release();
	// Release PixelShader interface
	grassPS->Release();
	// Release VertexShader interface
	treeVS->Release();
	// Release PixelShader interface
	treePS->Release();
	// Release cBuffer
	cBufferTree->Release();
	// Release cBuffer
	cBufferSky->Release();

	oceanVS->Release(); 
	oceanPS->Release();
	fireVS->Release();
	// Release PixelShader interface
	firePS->Release();
	// Release VertexShader interface
	perPixelLightingVS->Release();
	// Release PixelShader interface
	perPixelLightingPS->Release();
	// Release cBuffer
	cBufferLogs->Release();

	if (cBufferExtSrc)
		_aligned_free(cBufferExtSrc);

	if (projMatrix)
		_aligned_free(projMatrix);
	if (treeInstance)
		_aligned_free(treeInstance);

	if (mainCamera)
		mainCamera->release();

	if (mainClock)
		mainClock->release();
	// Release skyBox
	
	if (floor)
		floor->release();
	if (tree)
		tree->release();
	if (logs)
		logs->release();
	if (castle)
        castle->release();

	if (water)
		water->release();
	

	// Release skyBox
	if (skyBox)
		skyBox->release();
	if (fire)
		fire->release();

	// Release Box
	

	if (dx) {

		dx->release();
		dx = nullptr;
	}

	if (wndHandle)
		DestroyWindow(wndHandle);
}


// Decouple the encapsulated HWND and call DestoryWindow on the HWND
void DXController::destoryWindow() {

	if (wndHandle != NULL) {

		HWND hWnd = wndHandle;

		wndHandle = NULL;
		DestroyWindow(hWnd);
	}
}


// Resize swap chain buffers and update pipeline viewport configurations in response to a window resize event
HRESULT DXController::resizeResources() {

	if (dx) {

		// Only process resize if the DXSystem *dx exists (on initial resize window creation this will not be the case so this branch is ignored)
		HRESULT hr = dx->resizeSwapChainBuffers(wndHandle);
		rebuildViewport();
		RECT clientRect;
		GetClientRect(wndHandle, &clientRect);


		if (!isMinimised())
			renderScene();
	}

	return S_OK;
}


// Helper function to call updateScene followed by renderScene
HRESULT DXController::updateAndRenderScene() {

	HRESULT hr = updateScene();

	if (SUCCEEDED(hr))
		hr = renderScene();

	return hr;
}


// Clock handling methods

void DXController::startClock() {

	mainClock->start();
}

void DXController::stopClock() {

	mainClock->stop();
}

void DXController::reportTimingData() {

	cout << "Actual time elapsed = " << mainClock->actualTimeElapsed() << endl;
	cout << "Game time elapsed = " << mainClock->gameTimeElapsed() << endl << endl;
	mainClock->reportTimingData();
}



//
// Event handling methods
//
// Process mouse move with the left button held down


void DXController::handleMouseLDrag(const POINT &disp) {

	mainCamera->rotateElevation((float)-disp.y * 0.01f);
	mainCamera->rotateOnYAxis((float)-disp.x * 0.01f);
}

// Process mouse wheel movement
void DXController::handleMouseWheel(const short zDelta) {

	if (zDelta<0)
		mainCamera->zoomCamera(1.2f);
	else if (zDelta>0)
		mainCamera->zoomCamera(0.9f);
}


// Process key down event.  keyCode indicates the key pressed while extKeyFlags indicates the extended key status at the time of the key down event (see http://msdn.microsoft.com/en-gb/library/windows/desktop/ms646280%28v=vs.85%29.aspx).
void DXController::handleKeyDown(const WPARAM keyCode, const LPARAM extKeyFlags) {

	// Add key down handler here...
}


// Process key up event.  keyCode indicates the key released while extKeyFlags indicates the extended key status at the time of the key up event (see http://msdn.microsoft.com/en-us/library/windows/desktop/ms646281%28v=vs.85%29.aspx).
void DXController::handleKeyUp(const WPARAM keyCode, const LPARAM extKeyFlags) {

	// Add key up handler here...
}



//
// Methods to handle initialisation, update and rendering of the scene
//


HRESULT DXController::rebuildViewport(){
	// Binds the render target view and depth/stencil view to the pipeline.
	// Sets up viewport for the main window (wndHandle) 
	// Called at initialisation or in response to window resize


	ID3D11DeviceContext *context = dx->getDeviceContext();

	if ( !context)
		return E_FAIL;

	// Bind the render target view and depth/stencil view to the pipeline.
	ID3D11RenderTargetView* renderTargetView = dx->getBackBufferRTV();
	context->OMSetRenderTargets(1, &renderTargetView, dx->getDepthStencil());
	// Setup viewport for the main window (wndHandle)
	RECT clientRect;
	GetClientRect(wndHandle, &clientRect);
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(clientRect.right - clientRect.left);
	viewport.Height = static_cast<FLOAT>(clientRect.bottom - clientRect.top);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//Set Viewport
	context->RSSetViewports(1, &viewport);
	
	// Compute the projection matrix.
	projMatrix->projMatrix = XMMatrixPerspectiveFovLH(0.25f*3.14, viewport.Width / viewport.Height, 1.0f, 1000.0f);
	return S_OK;
}

HRESULT  DXController::bindDefaultPipeline(){

	ID3D11DeviceContext *context = dx->getDeviceContext();
	if (!context)
		return E_FAIL;
	// Apply RSState
	context->RSSetState(defaultRSstate);
	// Apply dsState
	context->OMSetDepthStencilState(defaultDSstate, 0);
	//Apply blendState
	FLOAT			blendFactor[4]; blendFactor[0] = blendFactor[1] = blendFactor[2] = blendFactor[3] = 1.0f;
	UINT			sampleMask = 0xFFFFFFFF; // Bitwise flags to determine which samples to process in an MSAA context
	context->OMSetBlendState(defaultBlendState, blendFactor, sampleMask);
	return S_OK;
}
HRESULT DXController::initDefaultPipeline(){
	
	ID3D11Device *device = dx->getDevice();
	if (!device)
		return E_FAIL;
	// Initialise default Rasteriser state object
	D3D11_RASTERIZER_DESC			RSdesc;

	ZeroMemory(&RSdesc, sizeof(D3D11_RASTERIZER_DESC));
	// Setup default rasteriser state 
	RSdesc.FillMode = D3D11_FILL_SOLID;
	RSdesc.CullMode = D3D11_CULL_NONE;
	RSdesc.FrontCounterClockwise = TRUE;
	RSdesc.DepthBias = 0;
	RSdesc.SlopeScaledDepthBias = 0.0f;
	RSdesc.DepthBiasClamp = 0.0f;
	RSdesc.DepthClipEnable = TRUE;
	RSdesc.ScissorEnable = FALSE;
	RSdesc.MultisampleEnable = TRUE;
	RSdesc.AntialiasedLineEnable = FALSE;
	HRESULT hr = device->CreateRasterizerState(&RSdesc, &defaultRSstate);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create Rasterise state interface");
	
	
	// Sky Box RSState
	RSdesc.CullMode = D3D11_CULL_NONE;
	hr = device->CreateRasterizerState(&RSdesc, &skyRSState);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create Rasterise state interface");
	

	// Output - Merger Stage

	// Initialise default depth-stencil state object
	D3D11_DEPTH_STENCIL_DESC	dsDesc;

	ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	// Setup default depth-stencil descriptor
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	// Initialise depth-stencil state object based on the given descriptor
	hr = device->CreateDepthStencilState(&dsDesc, &defaultDSstate);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create DepthStencil state interface");
	
	// Add Code Here (Disable Depth Writing for Fire)
	
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = device->CreateDepthStencilState(&dsDesc, &fireDSstate);

	// Add Code Here (Set Alpha Blending On)
	// Initialise default blend state object (Alpha Blending On)
	D3D11_BLEND_DESC	blendDesc;

	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	// Setup default blend state descriptor (no blend)
	blendDesc.AlphaToCoverageEnable = FALSE; // Use pixel coverage info from rasteriser (default FALSE)
	blendDesc.IndependentBlendEnable = FALSE; // The following array of render target blend properties uses the blend properties from RenderTarget[0] for ALL render targets
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	// Create blendState
	hr = device->CreateBlendState(&blendDesc, &defaultBlendState);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create Blend state interface");

	// create tree blend state
	blendDesc.AlphaToCoverageEnable = TRUE;
	RSdesc.MultisampleEnable = TRUE;
	// confirm blend description change
	hr = device->CreateBlendState(&blendDesc, &treesBlendState);

	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create Blend state interface");

	// Modify Code Here (Enable Alpha Blending for Fire)
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	hr = device->CreateBlendState(&blendDesc, &fireBlendState);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create Fire Blend state interface");
	

	return hr;
}

HRESULT DXController::LoadShader(ID3D11Device *device, const char *filename, DXBlob **PSBytecode, ID3D11PixelShader **pixelShader){

	//Load the compiled shader byte code.
	DXLoadCSO(filename, PSBytecode);
	// Create shader objects
	HRESULT hr = device->CreatePixelShader((*PSBytecode)->getBufferPointer(), (*PSBytecode)->getBufferSize(), NULL, pixelShader);
	
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create PixelShader interface");
	return hr;
}


HRESULT DXController::LoadShader(ID3D11Device *device, const char *filename, DXBlob **VSBytecode, ID3D11VertexShader **vertexShader){


	//Load the compiled shader byte code.
	DXLoadCSO(filename, VSBytecode);
	HRESULT hr = device->CreateVertexShader((*VSBytecode)->getBufferPointer(), (*VSBytecode)->getBufferSize(), NULL, vertexShader);
	if (!SUCCEEDED(hr))
		throw std::exception("Cannot create VertexShader interface");
	return hr;
}



// Main resource setup for the application.  These are setup around a given Direct3D device.
HRESULT DXController::initialiseSceneResources() {
	//ID3D11DeviceContext *context = dx->getDeviceContext();
	ID3D11Device *device = dx->getDevice();
	if (!device)
		return E_FAIL;

	//
	// Setup main pipeline objects
	//

	// Setup objects for fixed function pipeline stages
	// Rasterizer Stage
	// Bind the render target view and depth/stencil view to the pipeline
	// and sets up viewport for the main window (wndHandle) 


	// Allocate the projection matrix (it is setup in rebuildViewport).
	projMatrix = (projMatrixStruct*)_aligned_malloc(sizeof(projMatrixStruct), 16);

	// Allocate the world matrices for the  tree instances
	treeInstance = (worldMatrixStruct*)_aligned_malloc(sizeof(worldMatrixStruct)*NUM_TREES, 16);
	// Setup tree instance positions

	for (int i = 0; i < NUM_TREES; i++)
	{
		// Translate and Rotate trees randomly
		// Modify code here (randomly rotate trees)
		treeInstance[i].worldMatrix = XMMatrixScaling(2.0, 2.0, 2.0)*XMMatrixTranslation(randM1P1()*forestSize, 1, randM1P1()*forestSize)*XMMatrixRotationY(0);
	}


	rebuildViewport();
	initDefaultPipeline();
	bindDefaultPipeline();

	// Setup objects for the programmable (shader) stages of the pipeline
	DXBlob *skyBoxVSBytecode = nullptr;
	DXBlob *skyBoxPSBytecode = nullptr;
	DXBlob *grassVSBytecode = nullptr;
	DXBlob *grassPSBytecode = nullptr;
	DXBlob *treeVSBytecode = nullptr;
	DXBlob *treePSBytecode = nullptr;
	DXBlob *oceanVSBytecode = nullptr;
	DXBlob *oceanPSBytecode = nullptr;
	DXBlob *reflectionMapVSBytecode = nullptr;
	DXBlob *reflectionMapPSBytecode = nullptr;


	DXBlob *fireVSBytecode = nullptr;
	DXBlob *firePSBytecode = nullptr;
	DXBlob *perPixelLightingVSBytecode = nullptr;
	DXBlob *perPixelLightingPSBytecode = nullptr;



	LoadShader(device, "Shaders\\cso\\sky_box_vs.cso", &skyBoxVSBytecode, &skyBoxVS);
	LoadShader(device, "Shaders\\cso\\sky_box_ps.cso", &skyBoxPSBytecode, &skyBoxPS);
	LoadShader(device, "Shaders\\cso\\grass_vs.cso", &grassVSBytecode, &grassVS);
	LoadShader(device, "Shaders\\cso\\grass_ps.cso", &grassPSBytecode, &grassPS);
	LoadShader(device, "Shaders\\cso\\tree_vs.cso", &treeVSBytecode, &treeVS);
	LoadShader(device, "Shaders\\cso\\tree_ps.cso", &treePSBytecode, &treePS);
	LoadShader(device, "Shaders\\cso\\ocean_vs.cso", &oceanVSBytecode, &oceanVS);
	LoadShader(device, "Shaders\\cso\\ocean_ps.cso", &oceanPSBytecode, &oceanPS);
	LoadShader(device, "Shaders\\cso\\reflection_map_vs.cso", &reflectionMapVSBytecode, &reflectionMapVS);
	LoadShader(device, "Shaders\\cso\\reflection_map_ps.cso", &reflectionMapPSBytecode, &reflectionMapPS);

	LoadShader(device, "Shaders\\cso\\fire_vs.cso", &fireVSBytecode, &fireVS);
	LoadShader(device, "Shaders\\cso\\fire_ps.cso", &firePSBytecode, &firePS);
	LoadShader(device, "Shaders\\cso\\per_pixel_lighting_vs.cso", &perPixelLightingVSBytecode, &perPixelLightingVS);
	LoadShader(device, "Shaders\\cso\\per_pixel_lighting_ps.cso", &perPixelLightingPSBytecode, &perPixelLightingPS);


	// Create main camera
	//
	mainCamera = new LookAtCamera();
	mainCamera->setPos(XMVectorSet(25, 1, -14.5, 1));

	// Setup tree CBuffer
	cBufferExtSrc = (CBufferExt*)_aligned_malloc(sizeof(CBufferExt), 16);
	// Initialise tree CBuffer
	cBufferExtSrc->worldMatrix = XMMatrixIdentity();
	cBufferExtSrc->worldITMatrix = XMMatrixIdentity();
	cBufferExtSrc->WVPMatrix = mainCamera->dxViewTransform()*projMatrix->projMatrix;
	cBufferExtSrc->lightVec = XMFLOAT4(-250.0, 130.0, 145.0, 1.0); // Positional light
	cBufferExtSrc->lightAmbient = XMFLOAT4(0.3, 0.3, 0.3, 1.0);
	cBufferExtSrc->lightDiffuse = XMFLOAT4(0.8, 0.8, 0.8, 1.0);
	cBufferExtSrc->lightSpecular = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	XMStoreFloat4(&cBufferExtSrc->eyePos, mainCamera->getCameraPos());// camera->pos;


	D3D11_BUFFER_DESC cbufferDesc;
	D3D11_SUBRESOURCE_DATA cbufferInitData;
	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&cbufferInitData, sizeof(D3D11_SUBRESOURCE_DATA));
	cbufferDesc.ByteWidth = sizeof(CBufferExt);
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferInitData.pSysMem = cBufferExtSrc;
	
	//Create tree CBuffer
	HRESULT hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferTree);
	//castle cBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferCastle);

	//Create floor CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferGrass);



	

	//create water buffer

	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferWater);

	//Create logs CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferLogs);

	//Create fire CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferFire);

	// Initialise skyBox CBuffer
	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferSky);

	
	//
	// Setup example objects
	//

	// Load texture
	ID3D11Resource *demoModelImageResource2 = static_cast<ID3D11Resource*>(CastleTexture);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\STRiq4k.jpg", &demoModelImageResource2, &CastleTextureSRV);
	ID3D11Resource *demoModelImageResource = static_cast<ID3D11Resource*>(logsTexture);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\logs.jpg", &demoModelImageResource, &logsTextureSRV);
	ID3D11Resource *demoModelImageResource1 = static_cast<ID3D11Resource*>(treeTexture);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\tree.tif", &demoModelImageResource, &treeTextureSRV);
	ID3D11Resource *cubeMapResource = static_cast<ID3D11Resource*>(cubeMapTexture);
	hr = CreateDDSTextureFromFile(device, L"Resources\\Textures\\grassenvmap1024.dds", &cubeMapResource, &cubeMapTextureSRV);
	ID3D11Resource *grassResource = static_cast<ID3D11Resource*>(grassDiffuseMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\grass.png", &grassResource, &grassDiffuseMapSRV);
	ID3D11Resource *grassAlphaResource = static_cast<ID3D11Resource*>(grassAlphaMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\grassAlpha.tif", &grassAlphaResource, &grassAlphaMapSRV);
	ID3D11Resource *waterResource = static_cast<ID3D11Resource*>(waterNormalMap);
	hr = CreateDDSTextureFromFile(device, L"Resources\\Textures\\Waves.dds", &waterResource, &waterNormalMapSRV);
	ID3D11Resource *fireResource = static_cast<ID3D11Resource*>(fireDiffuseMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\fire.tif", &fireResource, &fireDiffuseMapSRV);
	ID3D11Resource *smokeResource = static_cast<ID3D11Resource*>(smokeDiffuseMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\smoke.tif", &smokeResource, &smokeDiffuseMapSRV);


	ID3D11Resource *grassNormalResource = static_cast<ID3D11Resource*>(grassNormalMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\normalmap.bmp", &grassNormalResource, &grassNormalMapSRV);
	ID3D11Resource *grassHeightResource = static_cast<ID3D11Resource*>(grassAlphaMap);
	hr = CreateWICTextureFromFile(device, L"Resources\\Textures\\heightmapp.bmp", &grassHeightResource, &grassHeightMapSRV);


	dx->getDeviceContext()->PSSetShaderResources(1, 1, &grassAlphaMapSRV);
	dx->getDeviceContext()->VSSetShaderResources(1, 1, &grassNormalMapSRV);
	dx->getDeviceContext()->VSSetShaderResources(0, 1, &grassHeightMapSRV);
	
	dx->getDeviceContext()->PSSetShaderResources(2, 1, &cubeMapTextureSRV);

	castle = new DXModel(device, reflectionMapVSBytecode, wstring(L"Resources\\Models\\saintriqT3DS.obj"), CastleTextureSRV, XMCOLOR(1, 1, 1, 1), XMCOLOR(1, 1, 1, 0.5));
	tree = new DXModel(device, treeVSBytecode, wstring(L"Resources\\Models\\tree.3ds"), treeTextureSRV, XMCOLOR(1.0, 1.0, 1.0, 1.0), XMCOLOR(0, 0, 0.0, 0.0));
	//skyBox = new Box(device, skyBoxVSBytecode, cubeMapTextureSRV);
	floor = new Grid(device, grassVSBytecode, grassDiffuseMapSRV);
	water = new Ocean(device, oceanVSBytecode, waterNormalMapSRV);
	logs = new DXModel(device, perPixelLightingVSBytecode, wstring(L"Resources\\Models\\logs.obj"), logsTextureSRV, XMCOLOR(1.0, 1.0, 1.0, 1.0), XMCOLOR(0, 0, 0.0, 0.0));
	fire = new Particles(device, fireVSBytecode, fireDiffuseMapSRV);


	// Release PixelShader DXBlob
	skyBoxPSBytecode->release();
	// Release vertexShader DXBlob
	skyBoxVSBytecode->release();
	// Release vertexShader DXBlob
	grassVSBytecode->release();
	// Release PixelShader DXBlob
	grassPSBytecode->release();
	// Release vertexShader DXBlob
	treeVSBytecode->release();
	// Release PixelShader DXBlob
	treePSBytecode->release();
	
	reflectionMapVSBytecode->release();
	
	reflectionMapPSBytecode->release();

	oceanVSBytecode->release();
	oceanPSBytecode->release();

	
	fireVSBytecode->release();
	firePSBytecode->release();
	
	perPixelLightingVSBytecode->release();
	
	perPixelLightingPSBytecode->release();
	return S_OK;
}


// Update scene state (perform animations etc)
HRESULT DXController::updateScene() {

	ID3D11DeviceContext *context = dx->getDeviceContext();

	mainClock->tick();
	gu_seconds tDelta = mainClock->gameTimeElapsed();

	cBufferExtSrc->Timer = (FLOAT)tDelta;
	XMStoreFloat4(&cBufferExtSrc->eyePos, mainCamera->getCameraPos());

	
	// Update castle cBuffer
	cBufferExtSrc->worldMatrix = XMMatrixTranslation(-18.5, 1, -20);
	cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
	cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferCastle);


	// Update  skyBox cBuffer
	cBufferExtSrc->WVPMatrix = XMMatrixScaling(100, 100, 100)*mainCamera->dxViewTransform()*projMatrix->projMatrix;
	mapCbuffer(cBufferExtSrc, cBufferSky);



	return S_OK;
}

// Helper function to copy cbuffer data from cpu to gpu
HRESULT DXController::mapCbuffer(void *cBufferExtSrcL, ID3D11Buffer *cBufferExtL)
{
	ID3D11DeviceContext *context = dx->getDeviceContext();
	// Map cBuffer
	D3D11_MAPPED_SUBRESOURCE res;
	HRESULT hr = context->Map(cBufferExtL, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	if (SUCCEEDED(hr)) {
		memcpy(res.pData, cBufferExtSrcL, sizeof(CBufferExt));
		context->Unmap(cBufferExtL, 0);
	}
	return hr;
}

// Render scene
HRESULT DXController::renderScene() {

	ID3D11DeviceContext *context = dx->getDeviceContext();

	// Validate window and D3D context
	if (isMinimised() || !context)
		return E_FAIL;


	// Clear the screen
	static const FLOAT clearColor[4] = {0.0f, 0.0f, 0.3f, 1.0f };
	context->ClearRenderTargetView(dx->getBackBufferRTV(), clearColor);
	context->ClearDepthStencilView(dx->getDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	// set the  skyBox shader objects
	context->VSSetShader(skyBoxVS, 0, 0);
	context->PSSetShader(skyBoxPS, 0, 0);
	// Apply the skyBox cBuffer.
	context->VSSetConstantBuffers(0, 1, &cBufferSky);
	context->PSSetConstantBuffers(0, 1, &cBufferSky);

	// Apply  skyBox RSState
	context->RSSetState(skyRSState);

	// Draw triangle
	if( skyBox) {
		// Render
		skyBox->render(context);
	}


	// Return to default RSState
	context->RSSetState(defaultRSstate);


	// Set ground vertex and pixel shaders
	context->VSSetShader(grassVS, 0, 0);
	context->PSSetShader(grassPS, 0, 0);

	// Draw the Grass
	if (floor) {
		// Render

		// Update floor cBuffer
		// Scale and translate floor world matrix
		cBufferExtSrc->worldMatrix = XMMatrixScaling(5, 5, 5)*XMMatrixTranslation(0, 0, 0);
		cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
		cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;

		for (int i = 0; i < numGrassPasses; i++)
		{
			cBufferExtSrc->grassHeight = (grassLength / numGrassPasses)*i;
			mapCbuffer(cBufferExtSrc, cBufferGrass);
			//// Apply the cBuffer.
			context->VSSetConstantBuffers(0, 1, &cBufferGrass);
			context->PSSetConstantBuffers(0, 1, &cBufferGrass);
			floor->render(context);

		}
	}

	context->VSSetConstantBuffers(0, 1, &cBufferWater);
	context->PSSetConstantBuffers(0, 1, &cBufferWater);

	context->VSSetShader(oceanVS, 0, 0);
	context->PSSetShader(oceanPS, 0, 0);
	
	// draw water
	if (water){

		//update water cBuffer
		cBufferExtSrc->worldMatrix = XMMatrixScaling(5, 5, 5)*XMMatrixTranslation(10, 1, 0);
		//cBufferExtSrc->worldMatrix = XMMatrixScaling(4, 4, 4)*XMMatrixTranslation(26.5, 5, 10);
		cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
		cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
		mapCbuffer(cBufferExtSrc, cBufferWater);
		
		water->render(context);
	}
	

	context->VSSetShader(reflectionMapVS, 0, 0);
	context->PSSetShader(reflectionMapPS, 0, 0);

	// Apply the castle cBuffer.
	context->VSSetConstantBuffers(0, 1, &cBufferCastle);
	context->PSSetConstantBuffers(0, 1, &cBufferCastle);
	// Draw castle
	if (castle) {
		// Render
		castle->render(context);
	}

	// set  shaders for tree
	context->VSSetShader(treeVS, 0, 0);
	context->PSSetShader(treePS, 0, 0);
	//Apply blendState
	FLOAT			blendFactor[4]; blendFactor[0] = blendFactor[1] = blendFactor[2] = blendFactor[3] = 1.0f;
	UINT			sampleMask = 0xFFFFFFFF; // Bitwise flags to determine which samples to process in an MSAA context
	context->OMSetBlendState(treesBlendState, blendFactor, sampleMask);

	// Draw tree	

	if (tree) {
		// Render trees
		for (int i = 0; i < NUM_TREES; i++)
		{

			// Update tree cBuffer for each tree instance
			cBufferExtSrc->worldMatrix = treeInstance[i].worldMatrix;
			cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
			cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
			mapCbuffer(cBufferExtSrc, cBufferTree);
			// Apply the tree cBuffer.
			context->VSSetConstantBuffers(0, 1, &cBufferTree);
			context->PSSetConstantBuffers(0, 1, &cBufferTree);

			tree->render(context);
		}
	}
	context->OMSetBlendState(defaultBlendState, blendFactor, sampleMask);


	//// Apply the cBuffer.
	context->VSSetShader(perPixelLightingVS, 0, 0);
	context->PSSetShader(perPixelLightingPS, 0, 0);

	// Apply the logs cBuffer.
	context->VSSetConstantBuffers(0, 1, &cBufferLogs);
	context->PSSetConstantBuffers(0, 1, &cBufferLogs);
	// Draw logs
	if (logs) {
		// set  shaders for logs

		// Update logs cBuffer
		// Scale and translate logs world matrix
		cBufferExtSrc->worldMatrix = XMMatrixScaling(0.002, 0.002, 0.002)*XMMatrixTranslation(-15, 2, 1.5)*XMMatrixRotationX(XMConvertToRadians(-90));
		cBufferExtSrc->worldITMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, cBufferExtSrc->worldMatrix));
		cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
		mapCbuffer(cBufferExtSrc, cBufferLogs);

		// Render
			logs->render(context);

		}

	// Draw the Fire (Draw all transparent objects last)
	if (fire) {

		// Set fire vertex and pixel shaders
		context->VSSetShader(fireVS, 0, 0);
		context->PSSetShader(firePS, 0, 0);
		//Apply blendState
		FLOAT			blendFactor[] = { 1, 1, 1, 1 };
		context->OMSetBlendState(fireBlendState, blendFactor, 0xFFFFFFFF);
		context->OMSetDepthStencilState(fireDSstate, 0);

		// Apply the cBuffer.
		context->VSSetConstantBuffers(0, 1, &cBufferFire);
		context->PSSetConstantBuffers(0, 1, &cBufferFire);

		// Update fire and smoke cBuffer
		cBufferExtSrc->Timer = cBufferExtSrc->Timer / 4;// smoke changes more slowly than flames
		// Scale and translate smoke world matrix
		cBufferExtSrc->worldMatrix = XMMatrixScaling(0.25, 0.25, 0.25)*XMMatrixTranslation(-15, 2, -2);
		cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
		mapCbuffer(cBufferExtSrc, cBufferFire);
		fire->setTexture(smokeDiffuseMapSRV);
		// Render Smoke
		fire->render(context);

		cBufferExtSrc->Timer = cBufferExtSrc->Timer * 6;// fire changes more quickly than flames
		// Scale and translate fire world matrix
		cBufferExtSrc->worldMatrix = XMMatrixScaling(0.5, 0.5, 0.5)*XMMatrixTranslation(-15, 2,-2);
		cBufferExtSrc->WVPMatrix = cBufferExtSrc->worldMatrix*mainCamera->dxViewTransform() * projMatrix->projMatrix;
		mapCbuffer(cBufferExtSrc, cBufferFire);
		fire->setTexture(fireDiffuseMapSRV);
		// Render Fire
		fire->render(context);


		context->OMSetBlendState(defaultBlendState, blendFactor, 0xFFFFFFFF);
		context->OMSetDepthStencilState(defaultDSstate, 0);
	
	}	

	// Present current frame to the screen
	HRESULT hr = dx->presentBackBuffer();

	return S_OK;
}
