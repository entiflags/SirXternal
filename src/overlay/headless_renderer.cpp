#include "overlay/headless_renderer.h"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

struct shared_overlay_handshake {
	HANDLE shared_texture_handle;
	UINT width;
	UINT height;
	DWORD producer_pid;
};

static HANDLE s_file_mapping = nullptr;

bool headless_renderer::start(render_object_manager* mgr, HWND game_hwnd, uint32_t width, uint32_t height) {
	render_mgr_ = mgr;
	game_hwnd_ = game_hwnd;
	width_ = width;
	height_ = height;
	running_ = true;

	render_thread_handle_ = CreateThread(nullptr, 0, [](LPVOID) -> DWORD {
		render_thread();
		return 0;
		}, nullptr, 0, nullptr);

	return render_thread_handle_ != nullptr;
}

void headless_renderer::stop() {
	running_ = false;
	if (render_thread_handle_) {
		WaitForSingleObject(render_thread_handle_, 3000);
		CloseHandle(render_thread_handle_);
		render_thread_handle_ = nullptr;
	}

	if (rtv_) { rtv_->Release(); rtv_ = nullptr; }
	if (mutex_) { mutex_->Release(); mutex_ = nullptr; }
	if (shared_texture_) { shared_texture_->Release(); shared_texture_ = nullptr; }
	if (context_) { context_->Release(); context_ = nullptr; }
	if (device_) { device_->Release(); device_ = nullptr; }

	if (s_file_mapping) {
		CloseHandle(s_file_mapping);
		s_file_mapping = nullptr;
	}

	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
	ready_ = false;
}

bool headless_renderer::is_ready() {
	return ready_;
}

bool headless_renderer::create_shared_texture() {
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width_;
	desc.Height = height_;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

	if (FAILED(device_->CreateTexture2D(&desc, nullptr, &shared_texture_)))
		return false;

	IDXGIResource* dxgi_resource = nullptr;
	shared_texture_->QueryInterface(__uuidof(IDXGIResource), (void**)&dxgi_resource);
	dxgi_resource->GetSharedHandle(&shared_handle_);
	dxgi_resource->Release();

	shared_texture_->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&mutex_);

	D3D11_RENDER_TARGET_VIEW_DESC rtv_desc{};
	rtv_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device_->CreateRenderTargetView(shared_texture_, &rtv_desc, &rtv_);

	return true;
}

void headless_renderer::publish_handshake() {
	s_file_mapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(shared_overlay_handshake), "Local\\SirXternalOverlayHandshake");

	auto* data = (shared_overlay_handshake*)MapViewOfFile(s_file_mapping, FILE_MAP_WRITE, 0, 0, sizeof(shared_overlay_handshake));

	data->shared_texture_handle = shared_handle_;
	data->width = width_;
	data->height = height_;
	data->producer_pid = GetCurrentProcessId();

	UnmapViewOfFile(data);
}

void headless_renderer::feed_input() {
	ImGuiIO& io = ImGui::GetIO();

	POINT cursor{};
	GetCursorPos(&cursor);
	ScreenToClient(game_hwnd_, &cursor);

	io.MousePos = ImVec2((float)cursor.x, (float)cursor.y);
	io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

	static DWORD last_time = GetTickCount();
	DWORD now = GetTickCount();
	io.DeltaTime = (now - last_time) / 1000.0f;
	if (io.DeltaTime <= 0.0f) io.DeltaTime = 0.001f;
	last_time = now;

	io.DisplaySize = ImVec2((float)width_, (float)height_);
	io.AddFocusEvent(true);
}

void headless_renderer::render_thread() {
	printf("[~] render_thread: creating d3d11 device...\n");
	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
		&feature_level, 1, D3D11_SDK_VERSION, &device_, nullptr, &context_);

	if (!device_ || !context_) {
		printf("[-] render_thread: failed to create device\n");
		return;
	}
	printf("[+] render_thread: device created\n");

	if (!create_shared_texture()) {
		printf("[-] render_thread: failed to create shared texture\n");
		return;
	}
	printf("[+] render_thread: shared texture created\n");

	publish_handshake();
	printf("[+] render_thread: handshake published\n");

	printf("[~] render_thread: init imgui...\n");
	ImGui::CreateContext();
	ImGui_ImplDX11_Init(device_, context_);
	printf("[+] render_thread: imgui initialized\n");

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)width_, (float)height_);

	// release initial ownership to ourselves (key 0)
	mutex_->ReleaseSync(0);

	ready_ = true;
	printf("[+] headless_renderer: ready\n");

	while (running_) {
		// acquire texture for rendering
		HRESULT hr = mutex_->AcquireSync(0, 32);
		if (FAILED(hr)) {
			Sleep(1);
			continue;
		}

		feed_input();

		// clear
		float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		context_->ClearRenderTargetView(rtv_, clear_color);
		context_->OMSetRenderTargets(1, &rtv_, nullptr);

		D3D11_VIEWPORT vp{};
		vp.Width = (float)width_;
		vp.Height = (float)height_;
		vp.MaxDepth = 1.0f;
		context_->RSSetViewports(1, &vp);

		// imgui frame
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		if (render_mgr_)
			render_mgr_->render_all(io.DisplaySize, io.MousePos, !io.MouseDown[0]);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// release to consumer (key 1)
		mutex_->ReleaseSync(1);

		Sleep(1); // ~1000fps cap, consumer controls actual framerate
	}
}