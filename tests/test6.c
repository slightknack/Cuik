
#include <stdint.h>
#include <stdbool.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void printf(const char* fmt, ...);

LRESULT main_wnd_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam) {
	if (message == WM_DESTROY) {
		ExitProcess(0);
	}
	
	return DefWindowProcA(wnd, message, wparam, lparam);
}

// NOTE(NeGate): this is where the actual logic goes, the entry point thread is where
// the window is going to be handled so that resize and move don't stall things.
static DWORD main_thread(LPVOID param) {
test_label:
	goto test_label;

	return 0;
}

int main(int argc, char** argv, char** env) {
	bool test_condition = true;

	char name[4];
	name[0] = 84;
	name[1] = 83;
	name[2] = 84;
	name[3] = 0;

	printf("Hello, World!");

	//uintptr_t name_as_int = (uintptr_t)&name;

	WNDCLASSA wc;
	wc.hInstance = GetModuleHandleA(NULL);
	wc.lpfnWndProc = main_wnd_proc;
	wc.lpszClassName = name;
	RegisterClassA(&wc);
	
	HWND window = CreateWindowExA(262144, name,
									name, 13565952,
									100, 100, 1600, 900,
								    NULL, NULL, wc.hInstance, NULL);

	DWORD main_thread_id;
	CreateThread(NULL, 0, main_thread, window, 0, &main_thread_id);

	// Display the window
	ShowWindow(window, SW_SHOWDEFAULT);
	SetFocus(window);
	
	while (1) {
		MSG msg;
		GetMessageA(&msg, NULL, 0, 0);
		TranslateMessage(&msg);

		if (msg.message == WM_QUIT ||
			msg.message == WM_CLOSE||
			msg.message == WM_SIZE ||
			msg.message == WM_CHAR ||
			msg.message == WM_KEYDOWN) {
			// forward the relevant messages
			PostThreadMessageA(main_thread_id, msg.message, msg.wParam, msg.lParam);
		} else {
			DispatchMessageA(&msg);
		}
	}

	return 0;
}
