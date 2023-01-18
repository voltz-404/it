#include "platform.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>

#include <SDL_syswm.h>
#include "internal.h"

std::string getSaveFileName()
{
	HANDLE hf;
	OPENFILENAME ofn;
	char filename[260] = { 0 };

	// Get Window HWND
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(Editor::getWindow(), &info);
	HWND hwnd = info.info.win.window;

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	// Display the Open dialog box. 

	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		hf = CreateFile(ofn.lpstrFile,
			GENERIC_READ,
			0,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL);
		return std::string(filename);
	}

	return std::string();
}

std::string getOpenFileName()
{
	OPENFILENAME ofn;
	char filename[260] =  { 0 };

	// Get Window HWND
	SDL_SysWMinfo info = { 0 };
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(Editor::getWindow(), &info);
	HWND hwnd = info.info.win.window;

	// Display the Open dialog box.
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		return std::string(filename);
	}

	return std::string();
}
#else
std::string openFileDialogBox()
{
	return std::string();
}

std::string getOpenFileName()
{
	return std::string();
}
#endif
