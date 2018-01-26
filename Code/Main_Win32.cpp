//Some code based off of code by Squirrel Eiserloh
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>

#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/App.hpp"

#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library


//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);


//-----------------------------------------------------------------------------------------------
const int OFFSET_FROM_WINDOWS_DESKTOP = 50;
const int WINDOW_PHYSICAL_WIDTH = (int) (SCREEN_RATIO_WIDTH);
const int WINDOW_PHYSICAL_HEIGHT = (int) (SCREEN_RATIO_HEIGHT);

HWND g_hWnd = nullptr;
HDC g_displayDeviceContext = nullptr;
HGLRC g_openGLRenderingContext = nullptr;
const char* APP_NAME = "Simple Miner";


//-----------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	unsigned char keyCode = (unsigned char)wParam;
	switch (wmMessageCode)
	{
		case WM_CLOSE:
		case WM_DESTROY:
		case WM_QUIT:
		{
			g_theApp->OnExitRequested();
			return 0;
		}

		case WM_SETFOCUS:
		{
			if (g_theInput)
				g_theInput->OnAppGainedFocus();
		}

		case WM_KILLFOCUS:
		{
			if (g_theInput)
				g_theInput->OnAppLostFocus();
		}

		case WM_KEYDOWN:
		{
			if (g_theInput)
				g_theInput->OnKeyDown(keyCode);

			break;
		}

		case WM_KEYUP:
		{
			if (g_theInput)
				g_theInput->OnKeyUp(keyCode);
			break;
		}

		case WM_LBUTTONDOWN:
		{
			if (g_theInput)
				g_theInput->OnKeyDown(MOUSE_LEFT);
			break;
		}

		case WM_LBUTTONUP:
		{
			if (g_theInput)
				g_theInput->OnKeyUp(MOUSE_LEFT);
			break;
		}

		case WM_RBUTTONDOWN:
		{
			if (g_theInput)
				g_theInput->OnKeyDown(MOUSE_RIGHT);
			break;
		}

		case WM_RBUTTONUP:
		{
			if (g_theInput)
				g_theInput->OnKeyUp(MOUSE_RIGHT);
			break;
		}
	}
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}


//-----------------------------------------------------------------------------------------------
void CreateOpenGLWindow(HINSTANCE applicationInstanceHandle)
{
	// Define a window class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Assign a win32 message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);

	RECT windowRect = { OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_WIDTH, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_HEIGHT };
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, APP_NAME, -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	g_hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL);

	ShowWindow(g_hWnd, SW_SHOW);
	SetForegroundWindow(g_hWnd);
	SetFocus(g_hWnd);

	g_displayDeviceContext = GetDC(g_hWnd);

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset(&pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor));
	pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	int pixelFormatCode = ChoosePixelFormat(g_displayDeviceContext, &pixelFormatDescriptor);
	SetPixelFormat(g_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor);
	g_openGLRenderingContext = wglCreateContext(g_displayDeviceContext);
	wglMakeCurrent(g_displayDeviceContext, g_openGLRenderingContext);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(2.f);
	glEnable(GL_LINE_SMOOTH);
}

//-----------------------------------------------------------------------------------------------
void Initialize(HINSTANCE applicationInstanceHandle)
{
	SetProcessDPIAware();
	CreateOpenGLWindow(applicationInstanceHandle);
	g_theApp = new App();
}

//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	delete g_theApp;
	g_theApp = nullptr;
}

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int)
{
	UNUSED(commandLineString);
	Initialize(applicationInstanceHandle);

	while (!g_theApp->IsQuitting() )
	{
		g_theApp->RunFrame();
		SwapBuffers(g_displayDeviceContext);
	}

	Shutdown();
	return 0;
}