#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool ourIsRunning;

global_variable BITMAPINFO ourBitmapInfo;
global_variable void* ourBitMapMemory;
global_variable HBITMAP ourBitmapHandle;
global_variable HDC ourDeviceContext;

internal void Win32ResizeDIBSection(int aWidth, int aHeight)
{
	// TODO(scarroll): Bulletproof this
	//		Maybe don't free first, free after, then free first if that fails.

	if(ourBitmapHandle)
	{
		DeleteObject(ourBitmapHandle);
	}
	
	if(!ourDeviceContext)
	{
		// TODO(scarroll): Should we recreate these under special circumstances
		ourDeviceContext = CreateCompatibleDC(0);
	}

	ourBitmapInfo.bmiHeader.biSize = sizeof(ourBitmapInfo.bmiHeader);
	ourBitmapInfo.bmiHeader.biWidth = aWidth;
	ourBitmapInfo.bmiHeader.biHeight = aHeight;
	ourBitmapInfo.bmiHeader.biPlanes = 1;
	ourBitmapInfo.bmiHeader.biBitCount = 32;
	ourBitmapInfo.bmiHeader.biCompression = BI_RGB;\

	ourBitmapHandle = CreateDIBSection(
		ourDeviceContext, &ourBitmapInfo,
		DIB_RGB_COLORS,
		&ourBitMapMemory,
		0, 0);
}

internal void Win32UpdateWindow(HDC aDeviceContext, int anX, int aY,  int aWidth, int aHeight)
{
	StretchDIBits(aDeviceContext,
		anX, aY, aWidth, aHeight,
		anX, aY, aWidth, aHeight,
		ourBitMapMemory,
		&ourBitmapInfo,
		DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK WindowProc(
	HWND   aWindowHandle,
	UINT   aMessage,
	WPARAM aWParam,
	LPARAM aLParam)
{
	LRESULT result = 0;

	switch (aMessage)
	{
	case WM_SIZE:
	{
		RECT clientRect;
		GetClientRect(aWindowHandle, &clientRect);
		int width = clientRect.right - clientRect.left;
		int height = clientRect.bottom - clientRect.top;
		Win32ResizeDIBSection(width, height);
	}
	break;

	case WM_DESTROY:
	{
		OutputDebugStringA("WM_DESTROY\n");
	}
	break;

	case WM_CLOSE:
	{
		PostQuitMessage(0);
		OutputDebugStringA("WM_CLOSE\n");
	}
	break;

	case WM_ACTIVATEAPP:
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC deviceContext = BeginPaint(aWindowHandle, &paint);

		LONG width = paint.rcPaint.right - paint.rcPaint.left;
		LONG height = paint.rcPaint.bottom - paint.rcPaint.top;
		LONG x = paint.rcPaint.left;
		LONG y = paint.rcPaint.top;

		static DWORD operation = WHITENESS;
		PatBlt(deviceContext, x, y, width, height, operation);

		EndPaint(aWindowHandle, &paint);
	}
	break;

	default:
	{
		//OutputDebugStringA("default\n");
		result = DefWindowProc(aWindowHandle, aMessage, aWParam, aLParam);
	}
	break;
	}

	return result;
}


int CALLBACK WinMain(
	HINSTANCE aInstance,
	HINSTANCE aPrevInstance,
	LPSTR aCmdLine,
	int aCmdShow)
{
	WNDCLASS windowClass = {};
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = GetModuleHandle(0);
	windowClass.lpszClassName = L"HandmadeHeroWindowClass";

	if (RegisterClass(&windowClass))
	{
		HWND windowHandle = CreateWindowEx(
			0,
			windowClass.lpszClassName,
			L"HandMade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			aInstance,
			nullptr
		);


		if(windowHandle)
		{
			MSG message;

			for(;;)
			{
				BOOL messageResult = GetMessage(&message, 0, 0, 0);
				if(messageResult > 0)
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			// TODO: Handle a window failing to create
		}

	}
	else
	{
		// TODO: Handle a window class failing to register
	}

	return 0;
}