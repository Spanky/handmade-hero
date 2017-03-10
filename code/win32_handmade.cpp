#include <windows.h>


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
		OutputDebugStringA("WM_SIZE\n");
	}
	break;

	case WM_DESTROY:
	{
		OutputDebugStringA("WM_DESTROY\n");
	}
	break;

	case WM_CLOSE:
	{
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
		if(operation == WHITENESS)
		{
			operation = BLACKNESS;
		}
		else
		{
			operation = WHITENESS;
		}

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
				if(messageResult >= 0)
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