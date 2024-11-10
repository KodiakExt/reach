#pragma once

#include <algorithm>
#include <string>
#include <tlhelp32.h>
#include <psapi.h>
#include <sstream>
#include <span>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <Windows.h>
#include <random>
#include <memory>
#include <atomic>
#include <future>

struct Pattern {
    std::vector<BYTE> bytes;
    std::string mask;
};

class GameUtil
{
public:
    static inline HMODULE module = nullptr;
    static inline HWND gameWindow;
    static inline DWORD proccessId;
    static inline HANDLE handle = nullptr;
    static inline uintptr_t baseAddress;
    static inline uintptr_t endAddress;

    static inline std::mutex resultsMutex;
    static inline std::vector<void*> foundAddresses;

    inline static boolean crashed()
    {
        return GameUtil::GetProcessByName("Minecraft.Windows.exe") == nullptr;
    }

    inline static HANDLE GetProcessByName(const PCSTR name)
    {
        DWORD pid = 0;

        // Create toolhelp snapshot.
        const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 process;
        ZeroMemory(&process, sizeof(process));
        process.dwSize = sizeof(process);

        // Walkthrough all processes.
        if (Process32First(snapshot, &process))
        {
            do
            {
                // Compare process.szExeFile based on format of name, i.e., trim file path
                // trim .exe if necessary, etc.
                if (std::string(process.szExeFile) == std::string(name))
                {
                    pid = process.th32ProcessID;
                    proccessId = pid;
                    break;
                }
            } while (Process32Next(snapshot, &process));
        }

        CloseHandle(snapshot);

        if (pid != 0)
        {
            return OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
        }

        return nullptr;
    }

    inline static bool close() {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) return false;

        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);
        bool processClosed = false;

        if (Process32First(hSnapshot, &processEntry)) {
            do {
                if (strcmp(processEntry.szExeFile, "Minecraft.Windows.exe") == 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processEntry.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                        processClosed = true;
                    }
                }
            } while (Process32Next(hSnapshot, &processEntry));
        }

        CloseHandle(hSnapshot);
        return processClosed;
    }

    inline static HMODULE GetModule(const HANDLE pHandle)
    {
        DWORD cbNeeded;

        if (HMODULE hMods[1024]; EnumProcessModules(pHandle, hMods, sizeof(hMods), &cbNeeded)) {
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
            {
                TCHAR szModName[MAX_PATH];
                if (GetModuleFileNameEx(pHandle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
                {
                    std::basic_string<TCHAR> wstrModName = szModName;
                    std::string wstrModContain = "Minecraft.Windows.exe";
                    TCHAR szModContain[22];
                    ua_tcscpy(szModContain, wstrModContain.c_str());
                    if (wstrModName.find(std::basic_string<TCHAR>(szModContain)) != std::string::npos)
                    {
                        //CloseHandle(pHandle);
                        return hMods[i];
                    }
                }
            }
        }
        return nullptr;
    }

    static void patchBytes(void* dst, void* src, unsigned int size) {
        DWORD oldprotect;
        VirtualProtectEx(handle, dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
        WriteProcessMemory(handle, dst, src, size, 0);
        VirtualProtectEx(handle, dst, size, oldprotect, &oldprotect);
    }

    inline static void readMemory(void* src, void* dst, unsigned int size) {
        DWORD oldprotect;
        VirtualProtectEx(handle, src, size, PAGE_EXECUTE_READWRITE, &oldprotect);
        ReadProcessMemory(handle, src, dst, size, nullptr);
        VirtualProtectEx(handle, src, size, oldprotect, &oldprotect);
    };

    inline static void nopBytes(void* dst, unsigned int size) {
        DWORD oldprotect;
        VirtualProtectEx(handle, dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
        std::vector<byte> bytes;
        for(unsigned int i = 0; i < size; i++) {
            bytes.push_back(0x90);
        }
        WriteProcessMemory(handle, dst, &bytes[0], size, nullptr);

        VirtualProtectEx(handle, dst, size, oldprotect, &oldprotect);
    };

    struct Pattern {
        std::vector<BYTE> bytes;
        std::string mask;
    };

    inline static void scanMemory(HANDLE handle, void* startAddress, SIZE_T regionSize, const Pattern& pattern)
    {
        constexpr SIZE_T chunkSize = 1024 * 2;
        SIZE_T bytesRead;
        std::unique_ptr<BYTE[]> pageData(new BYTE[chunkSize]);

        for (SIZE_T i = 0; i < regionSize; i += chunkSize) {
            SIZE_T toRead = std::min(chunkSize, regionSize - i);
            if (ReadProcessMemory(handle, reinterpret_cast<BYTE*>(startAddress) + i, pageData.get(), toRead, &bytesRead)) {
                const BYTE* patBytes = pattern.bytes.data();
                const char* patMask = pattern.mask.c_str();
                size_t bytesToCheck = bytesRead - pattern.bytes.size();

                for (size_t j = 0; j <= bytesToCheck; ++j) {
                    if (compareBytes(pageData.get() + j, patBytes, patMask)) {
                        std::lock_guard<std::mutex> lock(resultsMutex);
                        foundAddresses.push_back(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(startAddress) + i + j));
                    }
                }
            }
        }
    }

    inline static void* scanSig(const std::string& sig)
    {
        {
            std::lock_guard<std::mutex> lock(resultsMutex);
            foundAddresses.clear();
        }

        Pattern pattern;
        const char* str = sig.c_str();
        while (*str) {
            if (*str == ' ') {
                ++str;
                continue;
            }
            if (*str == '?') {
                pattern.bytes.push_back(0);
                pattern.mask.push_back('?');
                ++str;
                if (*str == '?') {
                    ++str;
                }
            } else {
                pattern.bytes.push_back(static_cast<BYTE>(std::strtoul(str, nullptr, 16)));
                pattern.mask.push_back('x');
                while (*str && *str != ' ') {
                    ++str;
                }
            }
        }

        MEMORY_BASIC_INFORMATION memInfo;
        void* currentAddress = nullptr;
        std::vector<std::thread> threads;

        while (VirtualQueryEx(handle, currentAddress, &memInfo, sizeof(memInfo)) != 0) {
            if (memInfo.State == MEM_COMMIT && memInfo.Protect != PAGE_NOACCESS) {
                threads.emplace_back(scanMemory, handle, currentAddress, memInfo.RegionSize, pattern);
            }
            currentAddress = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(currentAddress) + memInfo.RegionSize);
        }

        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        {
            std::lock_guard<std::mutex> lock(resultsMutex);
            if (!foundAddresses.empty()) {
                return foundAddresses.front();
            }
        }

        return nullptr;
    }

    inline static bool compareBytes(const BYTE* data, const BYTE* mask, const char* maskString) {
        for (; *maskString; ++maskString, ++data, ++mask) {
            if (*maskString == 'x' && *data != *mask) {
                return false;
            }
        }
        return (*maskString) == 0;
    }
};
