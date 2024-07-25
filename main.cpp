#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <iostream>
#include <wrl.h>
#include <d3dcompiler.h>

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SIZE:
			// Reset and resize swapchain
			return 0;
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_MOUSEMOVE:
			// std::cout << "Mouse " << LOWORD(lParam) << " " << HIWORD(lParam) <<
			// std::endl;
			break;
		case WM_LBUTTONUP:
			// std::cout << "WM_LBUTTONUP Left mouse button" << std::endl;
			break;
		case WM_RBUTTONUP:
			// std::cout << "WM_RBUTTONUP Right mouse button" << std::endl;
			break;
		case WM_KEYDOWN:
			// std::cout << "WM_KEYDOWN " << (int)wParam << std::endl;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
	}

	return ::DefWindowProc(hWnd, msg, wParam, lParam);
};

int main()
{
	const int width = 800, height = 600;

	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		WndProc,
		0L,
		0L,
		GetModuleHandle(NULL),
		NULL,
		NULL,
		NULL,
		NULL,
		L"Rasterization",
		NULL
	};

	RegisterClassEx(&wc);

	RECT wr = { 0, 0, width, height };

	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd = CreateWindow(
		wc.lpszClassName,
		L"Rasterization",
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		wc.hInstance,
		NULL);

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage =
		DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.Flags =
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	Microsoft::WRL::ComPtr<ID3D11Device>		deviceComPtr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> contextComPtr;
	Microsoft::WRL::ComPtr<IDXGISwapChain>		swapChainComPtr;

	UINT					createDeviceFlags = 0;
	const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0 };
	if (FAILED(D3D11CreateDeviceAndSwapChain(
			NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
			featureLevelArray, 1, D3D11_SDK_VERSION, &swapChainDesc, swapChainComPtr.GetAddressOf(),
			deviceComPtr.GetAddressOf(), NULL, contextComPtr.GetAddressOf())))
	{
		std::cout << "D3D11CreateDeviceAndSwapChain() error" << std::endl;
		return 1;
	}

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetViewComPtr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>		   backBufferComPtr;
	swapChainComPtr->GetBuffer(0, IID_PPV_ARGS(backBufferComPtr.GetAddressOf()));
	if (backBufferComPtr)
	{
		deviceComPtr->CreateRenderTargetView(backBufferComPtr.Get(), NULL, renderTargetViewComPtr.GetAddressOf());
	}
	else
	{
		std::cerr << "CreateRenderTargetView() failed." << std::endl;
		return 1;
	}

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = float(width);
	viewport.Height = float(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	contextComPtr->RSSetViewports(1, &viewport);

	// Blob is not COM object
	ID3DBlob* vertexBlob = nullptr;
	ID3DBlob* pixelBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr;

	hr = D3DCompileFromFile(L"VertexShader.hlsl", 0, 0, "main", "vs_5_0", 0, 0, &vertexBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (hr & D3D11_ERROR_FILE_NOT_FOUND)
			std::cerr << "File not found." << std::endl;
		else if (errorBlob)
			std::cerr << "Vertex shader compile error\n"
					  << (char*)errorBlob->GetBufferPointer() << std::endl;
		return 1;
	}

	hr = D3DCompileFromFile(L"PixelShader.hlsl", 0, 0, "main", "ps_5_0", 0, 0, &pixelBlob, &errorBlob); // ps_5_0

	if (FAILED(hr))
	{
		if (hr & D3D11_ERROR_FILE_NOT_FOUND)
			std::cerr << "File not found." << std::endl;
		else if (errorBlob)
			std::cerr << "Pixel shader compile error\n"
					  << (char*)errorBlob->GetBufferPointer() << std::endl;
		return 1;
	}

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader*	pixelShader;
	deviceComPtr->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), NULL, &vertexShader);
	deviceComPtr->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), NULL, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ID3D11InputLayout* layout;
	deviceComPtr->CreateInputLayout(ied, 2, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &layout);
	contextComPtr->IASetInputLayout(layout);
	// TODO : implement after InitShaders();

	std::cout << "Perfect Until Now!" << std::endl;
	return 0;
}
