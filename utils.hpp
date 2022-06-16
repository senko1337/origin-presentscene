#pragma once
#include <vector>

#define ORIGIN_HOOKFUNC "40 55 56 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 41 0F B6 F1 49 8B F8 48 8B EA 4D 85 C0 75 07"
#define ORIGIN_PRESENTHOOKDX11 "C3 CC CC CC CC 4C 8B DC 56 57 41 56 48 83 EC 60 49 C7 43 ? ? ? ? ?" // +5

uintptr_t sigscan(const char* module, const char* pattern)
{
    uintptr_t moduleAdress = 0;
    moduleAdress = (uintptr_t)GetModuleHandleA(module);

    static auto patternToByte = [](const char* pattern)
    {
        auto       bytes = std::vector<int>{};
        const auto start = const_cast<char*>(pattern);
        const auto end = const_cast<char*>(pattern) + strlen(pattern);

        for (auto current = start; current < end; ++current)
        {
            if (*current == '?')
            {
                ++current;
                if (*current == '?')
                    ++current;
                bytes.push_back(-1);
            }
            else { bytes.push_back(strtoul(current, &current, 16)); }
        }
        return bytes;
    };

    const auto dosHeader = (PIMAGE_DOS_HEADER)moduleAdress;
    const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)moduleAdress + dosHeader->e_lfanew);

    const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    auto       patternBytes = patternToByte(pattern);
    const auto scanBytes = reinterpret_cast<std::uint8_t*>(moduleAdress);

    const auto s = patternBytes.size();
    const auto d = patternBytes.data();

    for (auto i = 0ul; i < sizeOfImage - s; ++i)
    {
        bool found = true;
        for (auto j = 0ul; j < s; ++j)
        {
            if (scanBytes[i + j] != d[j] && d[j] != -1)
            {
                found = false;
                break;
            }
        }
        if (found) { return reinterpret_cast<uintptr_t>(&scanBytes[i]); }
    }
    return NULL;
}

void hook(__int64 addr, __int64 func, __int64* orig)
{
    static uintptr_t hook_addr;
    hook_addr = sigscan("igo64.dll", ORIGIN_HOOKFUNC);

    printf("hook %x\n", hook_addr);

    auto hook = ((__int64(__fastcall*)(const void* addr, __int64 func, __int64* orig, __int64 smthng))(hook_addr));
    hook((const void*)addr, (__int64)func, orig, (__int64)1);
}