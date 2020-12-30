#include "Application.hpp"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR /*lpCmdLine*/, _In_ int nShowCmd)
{
	try
	{
		gdi::Application app{ hInstance, nShowCmd };
		return app.run();
	}
	catch (const std::exception& ex)
	{
		OutputDebugStringA(ex.what());
		return -1;
	}
}