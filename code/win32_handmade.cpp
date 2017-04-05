#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

using uint8 = unsigned char;
using uint32 = unsigned int;


struct win32_offsceen_buffer
{
	BITMAPINFO myInfo;
	void* myMemory;
	int myWidth;
	int myHeight;
	int myBytesPerPixel;
	int myPitch;
};

struct win32_window_dimension
{
	int myWidth;
	int myHeight;
};

global_variable bool ourIsRunning;
global_variable win32_offsceen_buffer GlobalBackBuffer;

internal win32_window_dimension Win32GetWindowDimension(HWND aWindow)
{
	win32_window_dimension windowDimension;
	
	RECT clientRect;
	GetClientRect(aWindow, &clientRect);
	windowDimension.myWidth = clientRect.right - clientRect.left;
	windowDimension.myHeight = clientRect.bottom - clientRect.top;

	return windowDimension;
}

internal void RenderWeirdGradient(const win32_offsceen_buffer* aBuffer, uint8 anXOffset, uint8 aYOffset)
{
	uint8* row = (uint8*)aBuffer->myMemory;
	for (int y = 0; y < aBuffer->myHeight; y++)
	{
		uint32* pixels = (uint32*)row;
		for (int x = 0; x < aBuffer->myWidth; x++)
		{
			// Memory Layout: 00 00 00 00
			// Pixel Layout:  0xAARRGGBB
			// Due to little endian, the "little end" of BB is first in the memory layout
			// Memory Layout: BB GG RR AA

			uint8 g = (x + anXOffset);
			uint8 b = (y + aYOffset);
			
			(*pixels++) = ((g << 8) | (b));
		}

		row += aBuffer->myPitch;
	}
}

internal void Win32ResizeDIBSection(win32_offsceen_buffer* aBuffer, int aWidth, int aHeight)
{
	// TODO(scarroll): Bulletproof this
	//		Maybe don't free first, free after, then free first if that fails.

	if (aBuffer->myMemory)
	{
		VirtualFree(aBuffer->myMemory, 0, MEM_RELEASE);
		aBuffer->myMemory = nullptr;
	}
	
	aBuffer->myInfo.bmiHeader.biSize = sizeof(aBuffer->myInfo.bmiHeader);
	aBuffer->myInfo.bmiHeader.biWidth = aWidth;
	aBuffer->myInfo.bmiHeader.biHeight = -aHeight;
	aBuffer->myInfo.bmiHeader.biPlanes = 1;
	aBuffer->myInfo.bmiHeader.biBitCount = 32;
	aBuffer->myInfo.bmiHeader.biCompression = BI_RGB;

	aBuffer->myBytesPerPixel = aBuffer->myInfo.bmiHeader.biBitCount / 8;
	const auto requiredMemorySize = aWidth * aHeight * aBuffer->myBytesPerPixel;
	aBuffer->myMemory = VirtualAlloc(nullptr, requiredMemorySize, MEM_COMMIT, PAGE_READWRITE);

	aBuffer->myWidth = aWidth;
	aBuffer->myHeight = aHeight;

	aBuffer->myPitch = aWidth * aBuffer->myBytesPerPixel;

	RenderWeirdGradient(aBuffer, 0, 0);

}

internal void Win32DisplayBufferInWindow(HDC aDeviceContext, const win32_offsceen_buffer* aBuffer, int aWidth, int aHeight)
{
	// TODO(scarroll): Aspect ratio correction
	StretchDIBits(aDeviceContext,
		0, 0, aWidth, aHeight,
		0, 0, aBuffer->myWidth, aBuffer->myHeight,
		aBuffer->myMemory,
		&aBuffer->myInfo,
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

		win32_window_dimension windowDimension = Win32GetWindowDimension(aWindowHandle);
		Win32DisplayBufferInWindow(deviceContext, &GlobalBackBuffer, windowDimension.myWidth, windowDimension.myHeight);
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
	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

	WNDCLASS windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
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
				
				RenderWeirdGradient(&GlobalBackBuffer, xOffset, yOffset);

				HDC deviceContext = GetDC(windowHandle);

				win32_window_dimension windowDimension = Win32GetWindowDimension(windowHandle);
				Win32DisplayBufferInWindow(deviceContext, &GlobalBackBuffer, windowDimension.myWidth, windowDimension.myHeight);
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