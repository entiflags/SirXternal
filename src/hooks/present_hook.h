#pragma once
#include <cstdint>
#include <d3d11.h>
#include <dxgi.h>
#include <LiquidHookEx/Include.h>

namespace present_hook {

    struct hook_data : public LiquidHookEx::VTable::BaseHookData {
        // d3d11 state
        ID3D11Texture2D* overlay_tex = nullptr;
        ID3D11ShaderResourceView* overlay_srv = nullptr;
        IDXGIKeyedMutex* overlay_mutex = nullptr;

        ID3D11VertexShader* composite_vs = nullptr;
        ID3D11PixelShader* composite_ps = nullptr;
        ID3D11BlendState* composite_blend = nullptr;
        ID3D11SamplerState* composite_sampler = nullptr;
        ID3D11RasterizerState* composite_rs = nullptr;

        // pre-compiled shader blobs
        uint8_t vs_blob[4096]{};
        size_t vs_blob_size = 0;
        uint8_t ps_blob[4096]{};
        size_t ps_blob_size = 0;

        // device refs (obtained on first frame)
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* ctx = nullptr;

        // WinAPI function pointers
        decltype(&OpenFileMappingA) fn_open_file_mapping = nullptr;
        decltype(&MapViewOfFile) fn_map_view_of_file = nullptr;
        decltype(&UnmapViewOfFile) fn_unmap_view_of_file = nullptr;
        decltype(&CloseHandle) fn_close_handle = nullptr;

        // IIDs for QueryInterface
        IID iid_device = {};
        IID iid_texture2d = {};
        IID iid_keyed_mutex = {};

        // shared memory name
        char shared_mem_name[64]{};

        // blend constants
        float float_max = FLT_MAX;
        float float_one = 1.0f;

        // state
        bool init_done = false;
        bool overlay_ready = false;
    };

    bool install(uintptr_t swap_chain_ptr);
    bool uninstall();

} // namespace present_hook
