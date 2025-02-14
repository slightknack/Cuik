
//#include <stdbool.h>
//#include <stdint.h>

#define _X86_
#define _M_AMD64
#define _WIN64
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
 
LRESULT main_wnd_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam) {
	if (message == WM_DESTROY) {
		ExitProcess(0);
	}
	
	return DefWindowProcA(wnd, message, wparam, lparam);
}

int main(int argc, char** argv, char** env) {
	int a = (16 * 6) + 5;

	char name[4];
	name[0] = 84;
	name[1] = 83;
	name[2] = 84;
	name[3] = 0;

	WNDCLASSA wc;
	wc.hInstance = GetModuleHandleA(NULL);
	wc.lpfnWndProc = main_wnd_proc;
	wc.lpszClassName = name;
	RegisterClassA(&wc);
	
	HWND window = CreateWindowExA(262144, name,
									name, 0xCF0000,
									400, 400, 1600, 900,
								    NULL, NULL, NULL, NULL);

	GetLastError();

	// Display the window
	ShowWindow(window, SW_SHOWDEFAULT);
	SetFocus(window);
	
	while (1) {
		MSG msg;
		GetMessageA(&msg, NULL, 0, 0);
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return 0;
}

