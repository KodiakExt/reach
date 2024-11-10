#pragma once

#include <iostream>
#include <string>
#include <thread>

#include <string>
#include <span>
#include <cstring>
#include <fstream>
#include <thread>
#include <Windows.h>
#include <random>

class LoggerUtil {
public:
    inline static void log(const std::string& str, std::chrono::milliseconds sleep)
    {
        for (std::size_t i = 0; i < str.length(); ++i)
        {
            std::cout << str[i];
            std::cout.flush();
            std::this_thread::sleep_for(sleep);
        }
        std::cout << std::endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    inline static void log2(const std::string& str, std::chrono::milliseconds sleep)
    {
        for (std::size_t i = 0; i < str.length(); ++i)
        {
            std::cout << str[i];
            std::cout.flush();
            std::this_thread::sleep_for(sleep);
        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    inline static void success(const std::string& str, std::chrono::milliseconds sleep)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        for (std::size_t i = 0; i < str.length(); ++i)
        {
            std::cout << str[i];
            std::cout.flush();
            std::this_thread::sleep_for(sleep);
        }
        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    inline static void error(const std::string& str, std::chrono::milliseconds sleep)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        for (std::size_t i = 0; i < str.length(); ++i)
        {
            std::cout << str[i];
            std::cout.flush();
            std::this_thread::sleep_for(sleep);
        }
        std::cout << std::endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    inline static void warning(const std::string& str, std::chrono::milliseconds sleep)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        for (std::size_t i = 0; i < str.length(); ++i)
        {
            std::cout << str[i];
            std::cout.flush();
            std::this_thread::sleep_for(sleep);
        }
        std::cout << std::endl;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    inline static void clearConsole() {
        // It's Minecraft "Windows" 10 Edition buddy :D
        system("cls");
    }
};
