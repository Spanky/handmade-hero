#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool ourIsRunning;

global_variable BITMAPINFO ourBitmapInfo;
global_variable void* ourBitMapMemory;
global_variable int ourBitmapWidth;
global_variable int ourBitmapHeight;
global_variable int ourBytesPerPixel;

using uint8 = unsigned char;
using uint32 = unsigned int;

void RenderWeirdGradient(uint8 anXOffset, uint8 aYOffset)
{
	const auto pitch = ourBitmapWidth * ourBytesPerPixel;
	uint8* row = (uint8*)ourBitMapMemory;
	for (int y = 0; y < ourBitmapHeight; y++)
	{
		uint32* pixels = (uint32*)row;
		for (int x = 0; x < ourBitmapWidth; x++)
		{
			// Memory Layout: 00 00 00 00
			// Pixel Layout:  0xAARRGGBB
			// Due to little endian, the "little end" of BB is first in the memory layout
			// Memory Layout: BB GG RR AA

			uint8 g = (x + anXOffset);
			uint8 b = (y + aYOffset);
			
			(*pixels++) = ((g << 8) | (b));
		}

		row += pitch;
	}
}

internal void Win32ResizeDIBSection(int aWidth, int aHeight)
{
	// TODO(scarroll): Bulletproof this
	//		Maybe don't free first, free after, then free first if that fails.

	if (ourBitMapMemory)
	{
		VirtualFree(ourBitMapMemory, 0, MEM_RELEASE);
		ourBitMapMemory = nullptr;
	}
	
	ourBitmapInfo.bmiHeader.biSize = sizeof(ourBitmapInfo.bmiHeader);
	ourBitmapInfo.bmiHeader.biWidth = aWidth;
	ourBitmapInfo.bmiHeader.biHeight = -aHeight;
	ourBitmapInfo.bmiHeader.biPlanes = 1;
	ourBitmapInfo.bmiHeader.biBitCount = 32;
	ourBitmapInfo.bmiHeader.biCompression = BI_RGB;

	ourBytesPerPixel = ourBitmapInfo.bmiHeader.biBitCount / 8;
	const auto requiredMemorySize = aWidth * aHeight * ourBytesPerPixel;
	ourBitMapMemory = VirtualAlloc(nullptr, requiredMemorySize, MEM_COMMIT, PAGE_READWRITE);

	ourBitmapWidth = aWidth;
	ourBitmapHeight = aHeight;

	RenderWeirdGradient(0, 0);

}

internal void Win32UpdateWindow(HDC aDeviceContext, RECT* aClientRect)
{
	const auto windowWidth = aClientRect->right - aClientRect->left;
	const auto windowHeight = aClientRect->bottom - aClientRect->top;
	
	StretchDIBits(aDeviceContext,
		0, 0, windowWidth, windowHeight,
		0, 0, ourBitmapWidth, ourBitmapHeight,
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

		RECT clientRect;
		GetClientRect(aWindowHandle, &clientRect);
		Win32UpdateWindow(deviceContext, &clientRect);
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
	windowClass.hInstance = GetModuleHandle(nullptr);
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
			nullptr,
			nullptr,
			aInstance,
			nullptr
		);


		if(windowHandle)
		{
			int xOffset = 0;
			int yOffset = 0;
			ourIsRunning = true;

			while(ourIsRunning)
			{
				MSG message;
				while(PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
				{
					if(message.message == WM_QUIT)
					{
						ourIsRunning = false;
					}

					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				
				RenderWeirdGradient(xOffset, yOffset);

				HDC deviceContext = GetDC(windowHandle);
				RECT clientRect;
				GetClientRect(windowHandle, &clientRect);
				Win32UpdateWindow(deviceContext, &clientRect);
				ReleaseDC(windowHandle, deviceContext);

				xOffset += 1;
				yOffset += 1;
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