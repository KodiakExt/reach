#pragma once

#include <windows.h>
#include <iostream>

class Terminal {
public:
    inline static void setConsoleFont(const std::wstring& fontName)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_FONT_INFOEX fontInfo;
        fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);

        GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

        wcscpy_s(fontInfo.FaceName, fontName.c_str());

        SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
    }

    inline static void setConsoleFontSize(int fontSize)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_FONT_INFOEX fontInfo;
        fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);

        GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

        fontInfo.dwFontSize.Y = fontSize;

        SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
    }

    inline static void clear() {
        system("cls");
    }

    inline static void resizeConsole(int columns, int rows) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        COORD bufferSize;
        bufferSize.X = columns;
        bufferSize.Y = rows;
        SetConsoleScreenBufferSize(hConsole, bufferSize);

        SMALL_RECT windowSize;
        windowSize.Left = 0;
        windowSize.Top = 0;
        windowSize.Right = static_cast<SHORT>(columns - 1);
        windowSize.Bottom = static_cast<SHORT>(rows - 1);
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
    }

    inline static void setConsoleBufferSize(int columns, int rows) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        COORD bufferSize;
        bufferSize.X = columns;
        bufferSize.Y = rows;
        SetConsoleScreenBufferSize(hConsole, bufferSize);

        SMALL_RECT windowSize;
        windowSize.Left = 0;
        windowSize.Top = 0;
        windowSize.Right = static_cast<SHORT>(columns - 1);
        windowSize.Bottom = static_cast<SHORT>(rows - 1);
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
    }

    inline static bool isUserAdministrator() {
        bool isAdmin = false;
        HANDLE hToken = nullptr;

        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            TOKEN_ELEVATION elevation;
            DWORD dwSize;

            if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
                isAdmin = elevation.TokenIsElevated != 0;
            }

            CloseHandle(hToken);
        }

        return isAdmin;
    }

    inline static void setup() {
        if(!isUserAdministrator())
        {
            relaunchAsAdmin();
            exit(0);
        }

        SetConsoleTitleA("Kodiak Reach");

        resizeConsole(600, 500);
        setConsoleBufferSize(80, 25);

        setConsoleFontSize(24);
        setConsoleFont(L"Cascadia Mono SemiBold");
    }

    inline static void relaunchAsAdmin() {
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);

        SHELLEXECUTEINFOA sei = { sizeof(sei) };
        sei.lpVerb = "runas";
        sei.lpFile = exePath;
        sei.nShow = SW_SHOWNORMAL;
        ShellExecuteExA(&sei);
    }
};
