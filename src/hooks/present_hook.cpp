#include "hooks/present_hook.h"
#include <d3dcompiler.h>
#include <cstdio>
#include <cstring>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace present_hook {

    static LiquidHookEx::VTable s_hook("PresentHook");

    // globals for RIP slot patching
    static void* g_hook_data = nullptr;
    static void* g_original_present = nullptr;


    LH_START(".hkPresent")
#pragma strict_gs_check(off)

        __declspec(safebuffers)
        HRESULT __stdcall hk_present(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags) {
        volatile uintptr_t _dummy = reinterpret_cast<uintptr_t>(swap_chain);

        hook_data* data = (hook_data*)g_hook_data;
        typedef HRESULT(__stdcall* present_fn)(IDXGISwapChain*, UINT, UINT);
        present_fn original = (present_fn)g_original_present;

        // just call original for now — test that hook works without crashing
        return original(swap_chain, sync_interval, flags);
    }

    void hk_present_end() {}

    LH_END()

        static bool compile_shaders(hook_data& data) {
        const char* vs_src =
            "void VS(uint id:SV_VertexID, out float4 pos:SV_Position, out float2 uv:TEXCOORD0){"
            "uv=float2((id&1)?1.0:0.0,(id&2)?1.0:0.0);"
            "pos=float4(uv*float2(2,-2)+float2(-1,1),0,1);}";

        const char* ps_src =
            "Texture2D t:register(t0);SamplerState s:register(s0);"
            "float4 PS(float4 pos:SV_Position,float2 uv:TEXCOORD0):SV_Target{return t.Sample(s,uv);}";

        ID3DBlob* vs_blob = nullptr;
        ID3DBlob* ps_blob = nullptr;
        ID3DBlob* err = nullptr;

        if (FAILED(D3DCompile(vs_src, strlen(vs_src), nullptr, nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vs_blob, &err))) {
            if (err) { printf("[-] VS compile: %s\n", (char*)err->GetBufferPointer()); err->Release(); }
            return false;
        }
        if (FAILED(D3DCompile(ps_src, strlen(ps_src), nullptr, nullptr, nullptr, "PS", "ps_5_0", 0, 0, &ps_blob, &err))) {
            if (err) { printf("[-] PS compile: %s\n", (char*)err->GetBufferPointer()); err->Release(); }
            vs_blob->Release();
            return false;
        }

        memcpy(data.vs_blob, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize());
        data.vs_blob_size = vs_blob->GetBufferSize();
        memcpy(data.ps_blob, ps_blob->GetBufferPointer(), ps_blob->GetBufferSize());
        data.ps_blob_size = ps_blob->GetBufferSize();

        vs_blob->Release();
        ps_blob->Release();
        return true;
    }

    bool install(uintptr_t swap_chain_ptr) {
        if (!swap_chain_ptr) return false;

        uintptr_t present_addr = LiquidHookEx::proc->GetVTableFunction<8>(swap_chain_ptr);
        if (!present_addr) {
            printf("[-] present_hook: couldn't read vtable slot 8\n");
            return false;
        }

        auto* rendersys = LiquidHookEx::proc->GetRemoteModule("rendersystemdx11.dll");
        if (!rendersys || !rendersys->IsValid()) {
            printf("[-] present_hook: rendersystemdx11.dll not found\n");
            return false;
        }

        auto vtable_info = rendersys->FindVTableContainingFunction(present_addr);
        if (!vtable_info.vTableAddr) {
            auto* overlay_mod = LiquidHookEx::proc->GetRemoteModule("GameOverlayRenderer64.dll");
            if (overlay_mod && overlay_mod->IsValid())
                vtable_info = overlay_mod->FindVTableContainingFunction(present_addr);
        }

        // prepare hook data
        hook_data hk_data{};

        if (!compile_shaders(hk_data)) {
            printf("[-] present_hook: shader compilation failed\n");
            return false;
        }

        // resolve WinAPI functions
        HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
        hk_data.fn_open_file_mapping = (decltype(hk_data.fn_open_file_mapping))GetProcAddress(kernel32, "OpenFileMappingA");
        hk_data.fn_map_view_of_file = (decltype(hk_data.fn_map_view_of_file))GetProcAddress(kernel32, "MapViewOfFile");
        hk_data.fn_unmap_view_of_file = (decltype(hk_data.fn_unmap_view_of_file))GetProcAddress(kernel32, "UnmapViewOfFile");
        hk_data.fn_close_handle = (decltype(hk_data.fn_close_handle))GetProcAddress(kernel32, "CloseHandle");

        // IIDs
        hk_data.iid_device = __uuidof(ID3D11Device);
        hk_data.iid_texture2d = __uuidof(ID3D11Texture2D);
        hk_data.iid_keyed_mutex = __uuidof(IDXGIKeyedMutex);

        // shared memory name
        strcpy_s(hk_data.shared_mem_name, "Local\\SirXternalOverlayHandshake");

        // install via LiquidHookEx VTable hook using the swap chain address directly
        // Present is vtable index 8
        printf("[+] present_hook: installing on swap_chain @ 0x%llX (Present @ 0x%llX)\n",
            (uint64_t)swap_chain_ptr, (uint64_t)present_addr);

        bool result = s_hook.HookUsingAddr<hook_data>(
            present_addr,
            "DXGI.DLL",
            hk_data,
            (void*)hk_present,
            (void*)hk_present_end,
            {
                LiquidHookEx::VTable::RipSlot::Data(&g_hook_data),
                LiquidHookEx::VTable::RipSlot::Orig(&g_original_present),
            }
            );


        if (result)
            printf("[+] present_hook: installed\n");
        else
            printf("[-] present_hook: installation failed\n");

        return result;
    }

    bool uninstall() {
        return s_hook.Unhook();
    }

} // namespace present_hook
