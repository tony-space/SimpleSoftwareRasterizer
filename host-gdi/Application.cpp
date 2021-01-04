#include "Application.hpp"

namespace gdi {

Application* Application::s_instance{ nullptr };

Application::Application(HINSTANCE hInstance, int nShowCmd)
{
	HWND		hWnd;
	WNDCLASS	wndClass;

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = TEXT("SoftwareRendering");

	RegisterClass(&wndClass);

	hWnd = CreateWindow(
		TEXT("SoftwareRendering"),	// window class name
		TEXT("Software Rendering"),	// window caption
		WS_OVERLAPPEDWINDOW,		// window style
		CW_USEDEFAULT,				// initial x position
		CW_USEDEFAULT,				// initial y position
		800,						// initial x size
		600,						// initial y size
		NULL,						// parent window handle
		NULL,						// window menu handle
		hInstance,					// program instance handle
		NULL);						// creation parameters

	ShowWindow(hWnd, nShowCmd);
	s_instance = this;

	m_rasterizer.setTexture(loadTexture("../textures/Jupiter.bmp"));
	m_rasterizer.setMesh(rasterizer::Mesh::cube());
}

int Application::run()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return int(msg.wParam);
}

rasterizer::gamma_bgra_t* Application::draw(unsigned width, unsigned height)
{
	m_rasterizer.draw(width, height, m_framebuffer);
	return m_framebuffer.data();
}

LRESULT Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		RECT rect;
		GetClientRect(hWnd, &rect);

		auto rawData = s_instance->draw(rect.right, rect.bottom);

		PAINTSTRUCT ps;
		auto hDC = BeginPaint(hWnd, &ps);

		BITMAPINFO bitmapInfo;
		bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
		bitmapInfo.bmiHeader.biWidth = rect.right;
		bitmapInfo.bmiHeader.biHeight = rect.bottom;
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biBitCount = 32;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;
		bitmapInfo.bmiHeader.biSizeImage = 0;
		bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
		bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
		bitmapInfo.bmiHeader.biClrUsed = 0;
		bitmapInfo.bmiHeader.biClrImportant = 0;
		bitmapInfo.bmiColors[0] = {};

		auto hBitmap = CreateDIBitmap(hDC, &bitmapInfo.bmiHeader, CBM_INIT, rawData, &bitmapInfo, DIB_RGB_COLORS);
		auto hMemDC = CreateCompatibleDC(hDC);
		auto hOldObj = SelectObject(hMemDC, hBitmap);
		if (hOldObj)
		{
			auto success = SetMapMode(hMemDC, GetMapMode(hDC));
			success = BitBlt(hDC, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY);
			SelectObject(hMemDC, hOldObj);
		}
		DeleteDC(hMemDC);
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);

		static auto last = std::chrono::high_resolution_clock::now();
		static unsigned framesCounter = 0;
		
		auto cur = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(cur - last).count();
		framesCounter++;

		if (duration >= 2000)
		{
			SetWindowTextA(hWnd, std::to_string(float(framesCounter) / 2.0f).c_str());
			framesCounter = 0;
			last = cur;
		}

		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

rasterizer::Texture Application::loadTexture(std::filesystem::path path)
{
	auto hBitmap = (HBITMAP)LoadImageW(NULL, path.wstring().c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (hBitmap == NULL)
	{
		throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
	}

	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	std::vector<rasterizer::gamma_bgra_t> result(size_t(bitmap.bmWidth) * size_t(bitmap.bmHeight));

	BITMAPINFO bi = { {}, {} };
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = bitmap.bmWidth;
	bi.bmiHeader.biHeight = bitmap.bmHeight;
	bi.bmiHeader.biPlanes = bitmap.bmPlanes;
	bi.bmiHeader.biBitCount = bitmap.bmBitsPixel;
	bi.bmiHeader.biCompression = BI_RGB;

	GetDIBits(GetDC(NULL), hBitmap, 0, bitmap.bmHeight, result.data(), &bi, DIB_RGB_COLORS);
	DeleteObject(hBitmap);
	return rasterizer::Texture(bitmap.bmWidth, bitmap.bmHeight, result);
}

}