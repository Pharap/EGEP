#include "stdafx.h"
#include "EGEP.h"

constexpr std::size_t maxStringLength = 100;

//
// Globals
//

// Current instance
HINSTANCE instanceHandle;

// The title bar text
WCHAR titleString[maxStringLength];

// The main window class name
WCHAR windowClassName[maxStringLength];

int mouseX;
int mouseY;

bool running = false;

float positionX = 20;
float positionY = 20;

constexpr COLORREF darkGrey = RGB(63, 63, 63);
constexpr COLORREF lightGrey = RGB(127, 127, 127);
constexpr COLORREF lighterGrey = RGB(191, 191, 191);

COLORREF buttonColour = darkGrey;

//
// Utility functions
//

constexpr bool isKeyDown(SHORT keyState)
{
	return ((keyState & 0x8000) != 0);
}

// True if the key has been pressed since
constexpr bool wasKeyPressed(SHORT keyState)
{
	return ((keyState & 0x0001) != 0);
}


//
// Forward declarations
//

void setup();
void update(float deltaTime);
LRESULT paint(HWND, WPARAM, LPARAM);

ATOM RegisterWindowClass(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//
// Functions
//

// The setup step
void setup()
{
}

// The update step
void update(float deltaTime)
{
	constexpr float pixelsPerSecond = 32;

	const int previousX = static_cast<int>(positionX);
	const int previousY = static_cast<int>(positionY);

	if(isKeyDown(GetAsyncKeyState('W')))
	{
		positionY -= (pixelsPerSecond * deltaTime);
	}

	if(isKeyDown(GetAsyncKeyState('A')))
	{
		positionX -= (pixelsPerSecond * deltaTime);
	}

	if(isKeyDown(GetAsyncKeyState('S')))
	{
		positionY += (pixelsPerSecond * deltaTime);
	}

	if(isKeyDown(GetAsyncKeyState('D')))
	{
		positionX += (pixelsPerSecond * deltaTime);
	}

	const int currentX = static_cast<int>(positionX);
	const int currentY = static_cast<int>(positionY);

	if((currentX != previousX) || (currentY != previousY))
	{
		HWND windowHandle = GetActiveWindow();

		RECT oldButtonBounds { previousX, previousY, positionX + 100, positionY + 80 };
		InvalidateRect(windowHandle, &oldButtonBounds, FALSE);

		RECT newButtonBounds { positionX, positionY, positionX + 100, positionY + 80 };
		InvalidateRect(windowHandle, &newButtonBounds, FALSE);
	}
}

LRESULT paint(HWND windowHandle, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	
	PAINTSTRUCT paintStruct;
	HDC deviceContextHandle = BeginPaint(windowHandle, &paintStruct);

	FillRect(deviceContextHandle, &paintStruct.rcPaint, reinterpret_cast<HBRUSH>(6));

	HBRUSH buttonBrush = CreateSolidBrush(buttonColour);

	SelectObject(deviceContextHandle, buttonBrush);

	Rectangle(deviceContextHandle, positionX, positionY, positionX + 100, positionY + 80);

	DeleteObject(buttonBrush);

	EndPaint(windowHandle, &paintStruct);

	return 0;
}

// Processes messages for the main window.
LRESULT CALLBACK WndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_LBUTTONUP:
	case WM_MOUSEACTIVATE:
	case WM_MOUSEMOVE:
		{
			const WORD mouseX = LOWORD(lParam);
			const WORD mouseY = HIWORD(lParam);

			const WORD left = positionX;
			const WORD right = positionX + 100;
			const WORD top = positionY;
			const WORD bottom = positionY + 80;

			if((mouseX >= left) && (mouseX <= right) && (mouseY >= top) && (mouseY <= bottom))
			{
				buttonColour = lightGrey;
			}
			else
			{
				buttonColour = darkGrey;
			}

			const RECT buttonBounds { left, top, right, bottom };
			InvalidateRect(windowHandle, &buttonBounds, FALSE);

		}
		break;

	case WM_LBUTTONDOWN:
		{
			const WORD mouseX = LOWORD(lParam);
			const WORD mouseY = HIWORD(lParam);

			const WORD left = positionX;
			const WORD right = positionX + 100;
			const WORD top = positionY;
			const WORD bottom = positionY + 80;

			if((mouseX >= left) && (mouseX <= right) && (mouseY >= top) && (mouseY <= bottom))
			{
				buttonColour = lighterGrey;

				const RECT buttonBounds { left, top, right, bottom };
				InvalidateRect(windowHandle, &buttonBounds, FALSE);
			}

		}
		break;

	case WM_PAINT:
		return paint(windowHandle, wParam, lParam);

	default:
		return DefWindowProc(windowHandle, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE instanceHandle, _In_opt_ HINSTANCE previousInstanceHandle, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(previousInstanceHandle);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(instanceHandle, IDS_APP_TITLE, titleString, maxStringLength);
	LoadStringW(instanceHandle, IDC_EGEP, windowClassName, maxStringLength);
	RegisterWindowClass(instanceHandle);

	// Perform application initialisation:
	if (!InitInstance (instanceHandle, nCmdShow))
		return FALSE;

	// Load accelerator table
	HACCEL acceleratorTableHandle = LoadAccelerators(instanceHandle, MAKEINTRESOURCE(IDC_EGEP));

	running = true;

	setup();

	DWORD previousTicks = GetTickCount();

	MSG message;
	while(running)
	{
		// Calculate delta time
		DWORD currentTicks = GetTickCount();
		DWORD deltaTicks = (currentTicks - previousTicks);
		float deltaTime = (static_cast<float>(deltaTicks) / 1000.0f);
		previousTicks = currentTicks;
		
		// Main message loop:
		while(PeekMessage(&message, nullptr, 0, 0, PM_REMOVE) != 0)
		{
			if(!TranslateAccelerator(message.hwnd, acceleratorTableHandle, &message))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}

			if(message.message == WM_QUIT)
				running = false;
		}

		update(deltaTime);
	}

	return static_cast<int>(message.wParam);
}

// Registers the window class
ATOM RegisterWindowClass(HINSTANCE instanceHandle)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style		  = (CS_HREDRAW | CS_VREDRAW);
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra	 = 0;
	wcex.cbWndExtra	 = 0;
	wcex.hInstance	  = instanceHandle;
	wcex.hIcon		  = LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_EGEP));
	wcex.hCursor		= LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
	wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_EGEP);
	wcex.lpszClassName  = windowClassName;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

// Saves instance handle and creates main window
BOOL InitInstance(HINSTANCE instanceHandle, int nCmdShow)
{
	// Store instance handle in a global variable
   instanceHandle = instanceHandle;

   // Create window
   HWND windowHandle = CreateWindowW(windowClassName, titleString, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, instanceHandle, nullptr);

   // If window couldn't be created
   if (!windowHandle)
   {
		// Abort
		return FALSE;
   }

   // Display and update window
   ShowWindow(windowHandle, nCmdShow);
   UpdateWindow(windowHandle);

   return TRUE;
}