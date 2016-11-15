
//
// main.cpp
//

#include <stdafx.h>
#include <iostream>
#include <exception>
#include <GUConsole.h>
#include <DXController.h>

using namespace std;


// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {

	GUConsole		*debugConsole = nullptr;
	DXController	*appController = nullptr;

#pragma region 1. Initialise application

	// 1.1 Tell Windows to terminate app if heap becomes corrupted
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	// 1.2 Initialise COM
	if (!SUCCEEDED(CoInitialize(NULL)))
		return 0;

	try
	{
		// 1.3 Initialise debug console
		debugConsole = GUConsole::CreateConsole(L"DirectX11 Debug Console");

		if (!debugConsole)
			throw exception("Cannot create debug console");

		// Since we have to create the console before any memory report can be generated we have +1 malloc error so compensate for this.  debugConsole must be released once the final memory report is given!
		compensate_free_count(1);

		cout << "Hello DirectX 11...\n\n";
		
		// 1.4 Create main application controller object (singleton)
		appController = DXController::CreateDXController(600, 600, L"DirectX 11", L"DirectX 11", nCmdShow, hInstance, WndProc);

		if (!appController)
			throw exception("Cannot create main application controller");
	}
	catch(exception& e)
	{
		cout << e.what() << endl;

		if (appController)
			appController->release();

		if (debugConsole)
			debugConsole->release();

		CoUninitialize();

		return 0;
	}

#pragma endregion

#pragma region 2. Main message loop

	while (1) {

		MSG msg;

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			if (WM_QUIT == msg.message)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {

			appController->updateAndRenderScene();
		}
	}

#pragma endregion

#pragma region 3. Tear-down and exit

	// 3.1 Report final timing data...
	appController->reportTimingData();
	
	// 3.2 Dispose of application resources
	appController->release();
	
	// 3.3 Final memory report
	cout << "\nFinal memory allocations:\n";
	gu_memory_report();

	// 3.4 Close debug console
	if (debugConsole)
		debugConsole->release();

	// 3.5 Shutdown COM
	CoUninitialize();

	return 0;

#pragma endregion

}


// Application event handler
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT				clientRect;
	static POINT		prevMousePos, currentMousePos;

	switch (message) {

	case WM_CREATE:
	{
		// Bind HWND to host DXController - this allows all relevant app data to be accessed when processing events related to the HWND.
		LPCREATESTRUCT createStruct = (LPCREATESTRUCT)lParam;
		DXController *appController = (DXController*)createStruct->lpCreateParams;

		SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(appController));

		break;
	}

	case WM_SIZE:
	{
		DXController *appController = (DXController*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

		if (appController)
			appController->resizeResources();

		break;
	}

	case WM_SIZING:
	{
		// Make sure window RECT does not fall below a minimum size
		LPRECT R = (LPRECT)lParam;

		if (R->bottom - R->top < 200) {

			// Determine edge being resized and modify RECT accordingly
			if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT) {

				// We're resizing from the bottom so set the bottom edge relative to the current top edge
				R->bottom = R->top + 200;
			}
			else if (wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT) {
			
				// We're resizing from the top so set the top edge relative to the current bottom edge
				R->top = R->bottom - 200;
			}
		}

		return TRUE;
	}

	case WM_ENTERSIZEMOVE:
	{
		// Stop the clock during (modal) window resize / move
		DXController *appController = (DXController*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

		if (appController)
			appController->stopClock();

		break;
	}

	case WM_EXITSIZEMOVE:
	{
		// Restart the clock once the window resize / move has completed
		DXController *appController = (DXController*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

		if (appController)
			appController->startClock();

		break;
	}

	case WM_PAINT:
	{
		// Don't do any painting - just validate window RECT
		GetClientRect(hWnd, &clientRect);
		ValidateRect(hWnd, &clientRect);
		break;
	}
	
	case WM_CLOSE:
	{
		// Window will close - decouple HWND from main controller
		DXController *appController = (DXController*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

		if (appController)
			appController->destoryWindow();
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	
#pragma region Mouse Input Handlers

	case WM_LBUTTONDOWN:
	{
		SetCapture(hWnd);
		GetCursorPos(&currentMousePos);
		break;
	}
	
	case WM_MOUSEMOVE:
	{
		// Only interested in mouse move if left button held down
		if (wParam & MK_LBUTTON) {

			prevMousePos = currentMousePos;
			GetCursorPos(&currentMousePos);

			DXController *appController = (DXController*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
			
			if (appController) {

				POINT disp;

				disp.x = currentMousePos.x - prevMousePos.x;
				disp.y = currentMousePos.y - prevMousePos.y;

				appController->handleMouseLDrag(disp);
				appController->updateAndRenderScene();
			}
		}
		break;
	}
	
	case WM_LBUTTONUP:
	{
		ReleaseCapture();
		break;
	}

	case WM_MOUSEWHEEL:
	{
		DXController *appController = (DXController*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		
		if (appController)
			appController->handleMouseWheel((short)HIWORD(wParam));
		
		break;
	}

	case WM_KEYDOWN:
	{
		DXController *appController = (DXController*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

		if (appController)
			appController->handleKeyDown(wParam, lParam);

		break;
	}

	case WM_KEYUP:
	{
		DXController *appController = (DXController*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

		if (appController)
			appController->handleKeyUp(wParam, lParam);

		break;
	}

#pragma endregion

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	return 0;
}
