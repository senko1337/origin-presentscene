#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <iostream>
#include <d3dx11.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"
#include "utils.hpp"
#include "menu.hpp"

ID3D11RenderTargetView* rendertarget;
ID3D11DeviceContext* context;
ID3D11Device* device;
HRESULT(*PresentOrig)(IDXGISwapChain* swapchain, UINT sync, UINT flags) = nullptr;
HWND hwnd;
int X, Y;

static bool firstinit = true;
HRESULT PresentHook(IDXGISwapChain* swapchain, UINT sync, UINT flags) {
    if (firstinit) {
        
        ID3D11Texture2D* renderTarget = 0;
        ID3D11Texture2D* backBuffer = 0;
        D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };

        swapchain->GetDevice(__uuidof(device), (PVOID*)&device);
        device->GetImmediateContext(&context);

        swapchain->GetBuffer(0, __uuidof(renderTarget), (PVOID*)&renderTarget);
        device->CreateRenderTargetView(renderTarget, nullptr, &rendertarget);
        renderTarget->Release();

        swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (PVOID*)&backBuffer);
        backBuffer->GetDesc(&backBufferDesc);

        X = backBufferDesc.Width;
        Y = backBufferDesc.Height;

        backBuffer->Release();

        hwnd = FindWindowA("UnrealWindow", 0);
        if (!hwnd)
            MessageBoxA(NULL, "render", "hwnd not found", MB_ICONWARNING | MB_OK);

        ImGui_ImplDX11_Init(hwnd, device, context);
        ImGui_ImplDX11_CreateDeviceObjects();

        firstinit = false;
    }
    context->OMSetRenderTargets(1, &rendertarget, nullptr);
    ImGui_ImplDX11_NewFrame();

    drawmenu();

    ImGui::Render();
    return PresentOrig(swapchain, sync, flags);
}

VOID Thread() {
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    if (!GetModuleHandleA("igo64.dll")) {
        MessageBoxA(NULL, "render", "igo64.dll not found", MB_ICONWARNING | MB_OK);
        exit(1);
    }

    uintptr_t PresentScene = sigscan("igo64.dll", ORIGIN_PRESENTHOOKDX11) + 5;
    if (!PresentScene) {
        MessageBoxA(NULL, "render", "presentscene not found", MB_ICONWARNING | MB_OK);
        exit(1);
    }

    printf("present scene %x\n", PresentScene);

    if (PresentScene)
        hook(PresentScene, (__int64)PresentHook, (__int64*)&PresentOrig);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_THREAD_ATTACH)
        Thread();
    return TRUE;
}

