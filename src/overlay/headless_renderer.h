#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdint>
#include <atomic>
#include "render_object_manager.h"

class headless_renderer {
public:
    static bool start(render_object_manager* mgr, HWND game_hwnd, uint32_t width, uint32_t height);
    static void stop();
    static bool is_ready();

private:
    static void render_thread();
    static void feed_input();
    static bool create_shared_texture();
    static void publish_handshake();

    inline static ID3D11Device* device_ = nullptr;
    inline static ID3D11DeviceContext* context_ = nullptr;
    inline static ID3D11Texture2D* shared_texture_ = nullptr;
    inline static ID3D11RenderTargetView* rtv_ = nullptr;
    inline static IDXGIKeyedMutex* mutex_ = nullptr;
    inline static HANDLE shared_handle_ = nullptr;

    inline static render_object_manager* render_mgr_ = nullptr;
    inline static HWND game_hwnd_ = nullptr;
    inline static uint32_t width_ = 0;
    inline static uint32_t height_ = 0;

    inline static std::atomic<bool> running_ = false;
    inline static std::atomic<bool> ready_ = false;
    inline static HANDLE render_thread_handle_ = nullptr;
};
