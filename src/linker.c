#include "linker.h"

#if _WIN32
#include <windows.h>
#endif

#define LINKER_STRING_BUFFER_CAP 8192

bool linker_init(Linker* l) {
	// NOTE(NeGate): Windows has a max command line length of 32768 iirc,
	// so this seems reasonable
	*l = (Linker) {
		.input_file_buffer = malloc(LINKER_STRING_BUFFER_CAP),
		.libpaths_buffer = malloc(LINKER_STRING_BUFFER_CAP),
	};
	
#if _WIN32
	l->vswhere = MicrosoftCraziness_find_visual_studio_and_windows_sdk();
	
	return (l->vswhere.vs_exe_path != NULL);
#else
	return true;
#endif
}

void linker_deinit(Linker* l) {
#if _WIN32
	MicrosoftCraziness_free_resources(&l->vswhere);
#endif
	
	free(l->input_file_buffer);
	free(l->libpaths_buffer);
}

void linker_add_default_libpaths(Linker* l) {
#if _WIN32
	linker_add_libpath_wide(l, l->vswhere.vs_library_path);
	linker_add_libpath_wide(l, l->vswhere.windows_sdk_ucrt_library_path);
	linker_add_libpath_wide(l, l->vswhere.windows_sdk_um_library_path);
#endif
}

#if _WIN32
void linker_add_libpath_wide(Linker* l, const wchar_t* filepath) {
	assert(filepath);
	
	size_t filepath_len = wcslen(filepath) + 1;
	if (l->libpaths_top + filepath_len >= LINKER_STRING_BUFFER_CAP) abort();
	
	memcpy(&l->libpaths_buffer[l->libpaths_top], filepath, filepath_len * sizeof(wchar_t));
	l->libpaths_top += filepath_len;
	l->libpaths_count++;
}
#endif

void linker_add_libpath(Linker* l, const char filepath[]) {
#if _WIN32
	size_t remaining = LINKER_STRING_BUFFER_CAP - l->libpaths_top;
	OS_String output = &l->libpaths_buffer[l->libpaths_top];
	
	int number_of_wide_chars = MultiByteToWideChar(65001 /* UTF8 */, 0, filepath, -1,  output, remaining);
	l->libpaths_top += number_of_wide_chars;
	l->libpaths_count++;
#else
	size_t filepath_len = strlen(filepath) + 1;
	if (l->libpaths_top + filepath_len >= LINKER_STRING_BUFFER_CAP) abort();
	
	memcpy(&l->libpaths_buffer[l->libpaths_top], filepath, filepath_len * sizeof(char));
	l->libpaths_top += filepath_len;
	l->libpaths_count++;
#endif
}

void linker_add_input_file(Linker* l, const char filepath[]) {
#if _WIN32
	size_t remaining = LINKER_STRING_BUFFER_CAP - l->input_file_top;
	OS_String output = &l->input_file_buffer[l->input_file_top];
	
	int number_of_wide_chars = MultiByteToWideChar(65001 /* UTF8 */, 0, filepath, -1,  output, remaining);
	l->input_file_top += number_of_wide_chars;
	l->input_file_count++;
#else
	size_t filepath_len = strlen(filepath) + 1;
	if (l->input_file_top + filepath_len >= LINKER_STRING_BUFFER_CAP) abort();
	
	memcpy(&l->libpaths_buffer[l->input_file_top], filepath, filepath_len * sizeof(char));
	l->input_file_top += filepath_len;
	l->input_file_count++;
#endif
}

// TODO(NeGate): Do some testing to make sure this works with unicode input.
// Like im pretty sure %S doesn't do the UTF-8 conversion and im being lazy about it.
enum { CMD_LINE_MAX = 8192 };

bool linker_invoke(Linker* l, const char* filename, bool linked_with_crt) {
#if defined(_WIN32)
	wchar_t cmd_line[CMD_LINE_MAX];
	int cmd_line_len = swprintf(cmd_line, CMD_LINE_MAX,
								L"%s\\link.exe /nologo /machine:amd64 /subsystem:console"
								" /debug:none /entry:mainCRTStartup /pdb:%S.pdb /out:%S.exe ",
								l->vswhere.vs_exe_path, filename, filename);
	
	// Add all the libpaths
	{
		size_t i = l->libpaths_count;
		OS_String str = l->libpaths_buffer;
		while (i--) {
			cmd_line_len += swprintf(&cmd_line[cmd_line_len], CMD_LINE_MAX - cmd_line_len, L"/libpath:\"%s\" ", str);
			str += wcslen(str) + 1;
		}
	}
	
	if (linked_with_crt) {
		cmd_line_len += swprintf(&cmd_line[cmd_line_len], CMD_LINE_MAX - cmd_line_len, L"/defaultlib:libcmt ");
	}
	
	// Add all the input files
	{
		size_t i = l->input_file_count;
		OS_String str = l->input_file_buffer;
		while (i--) {
			cmd_line_len += swprintf(&cmd_line[cmd_line_len], CMD_LINE_MAX - cmd_line_len, L"%s ", str);
			str += wcslen(str) + 1;
		}
	}
	
	STARTUPINFOW si = {
		.cb = sizeof(STARTUPINFOW),
		.dwFlags = STARTF_USESTDHANDLES,
		.hStdInput = GetStdHandle(STD_INPUT_HANDLE),
		.hStdError = GetStdHandle(STD_ERROR_HANDLE),
		.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE)
	};
	PROCESS_INFORMATION pi = {};
	
	//printf("Linker command:\n%S %S\n", exe_path, cmd_line);
	if (!CreateProcessW(NULL, cmd_line, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		printf("Linker command could not be executed.");
		return false;
	}
	
	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);
	
	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
#elif defined(__unix__)
	return false;
#else
#error "Implement system linker on other platforms"
#endif
}
