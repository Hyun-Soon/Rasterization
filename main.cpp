#include <windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <iostream>
#include <wrl.h>
#include <d3dcompiler.h>
#include <vector>
#include <glm/glm.hpp>

std::ostream& operator<<(std::ostream& os, const glm::vec4& v)
{
	os << "(x, y, z): (" << v.x << ", " << v.y << ", " << v.z << ")";
	return os;
}

struct Vertex
{
	glm::vec4 pos;
	glm::vec2 uv;
};

struct Transformation
{
	float		thetaX = glm::radians(90.f);
	float		thetaY = 0 / 180 * 3.141592f;
	float		thetaZ = 0 / 180 * 3.141592f;
	float		scaleX = 1.0f;
	float		scaleY = 1.0f;
	float		scaleZ = 1.0f;
	glm::mat4x4 translation = {
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	glm::mat4x4 rotateX = {
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, cos(thetaX), -sin(thetaX), 0.0f },
		{ 0.0f, sin(thetaX), cos(thetaX), 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	glm::mat4x4 rotateY = {
		{ cos(thetaY), 0.0f, sin(thetaY), 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ -sin(thetaY), 0.0f, cos(thetaY), 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	glm::mat4x4 rotateZ = {
		{ cos(thetaZ), -sin(thetaZ), 0.0f, 0.0f },
		{ sin(thetaZ), cos(thetaZ), 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	glm::mat4x4 scale = {
		{ scaleX, 0.0f, 0.0f, 0.0f },
		{ 0.0f, scaleY, 0.0f, 0.0f },
		{ 0.0f, 0.0f, scaleZ, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
};

class Mesh
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<uint16_t> indices)
	{
		for (size_t i = 0; i < vertices.size(); i++)
		{
			this->vertices.push_back(vertices[i].pos);
			this->normals.push_back(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
		}

		this->indices = indices;
	};
	//~Mesh();
	// void initBox(std::vector<glm::vec3> vertices, std::vector<uint16_t> indices)
	//{
	//	for (size_t i = 0; i < indices.size(); i++)
	//	{
	//		this->vertices.push_back({ vertices[indices[i]], 1.0f });
	//		this->normals.push_back({ 0.0f, 0.0f, -1.0f, 0.0f });
	//	}
	// }

	std::vector<glm::vec4> vertices;
	std::vector<uint16_t>  indices;
	std::vector<glm::vec4> normals;
	Transformation		   transformation;
};

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
	const int	width = 800, height = 600;
	const float aspect_ratio = static_cast<float>(width) / height;

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

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* colorSampler;
	deviceComPtr->CreateSamplerState(&sampDesc, &colorSampler);

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.MipLevels = textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MiscFlags = 0;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureDesc.Width = width;
	textureDesc.Height = height;

	ID3D11Texture2D*		  canvasTexture;
	ID3D11ShaderResourceView* canvasTextureView;
	ID3D11RenderTargetView*	  canvasRenderTargetView;
	deviceComPtr->CreateTexture2D(&textureDesc, nullptr, &canvasTexture);
	if (canvasTexture)
	{
		deviceComPtr->CreateShaderResourceView(canvasTexture, nullptr,
			&canvasTextureView);

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		deviceComPtr->CreateRenderTargetView(canvasTexture, &renderTargetViewDesc,
			&canvasRenderTargetView);
	}
	else
	{
		std::cerr << "CreateRenderTargetView() failed." << std::endl;
	}

	// Create vertex buffer
	const std::vector<Vertex> vertices = {
		{
			{ -1.0f, -1.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f },
		},
		{
			{ -1.0f, 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f },
		},
		{
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f },
		},
		{
			{ 1.0f, -1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f },
		},
	};

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = UINT(sizeof(Vertex) * vertices.size());
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.StructureByteStride = sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA vertexBufferData = {
		0,
	};
	vertexBufferData.pSysMem = vertices.data();
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	ID3D11Buffer* vertexBuffer = nullptr;
	hr = deviceComPtr->CreateBuffer(&bufferDesc, &vertexBufferData, &vertexBuffer);
	if (FAILED(hr))
	{
		std::cout << "CreateBuffer() failed. " << std::hex << hr
				  << std::endl;
	};

	const std::vector<uint16_t> indices = {
		0,
		1,
		2,
		0,
		2,
		3,
	};

	UINT indexCount = UINT(indices.size());

	D3D11_BUFFER_DESC bufferDesc2 = {};
	bufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc2.ByteWidth = UINT(sizeof(uint16_t) * indices.size());
	bufferDesc2.BindFlags =
		D3D11_BIND_INDEX_BUFFER;
	bufferDesc2.CPUAccessFlags =
		D3D11_CPU_ACCESS_WRITE;
	bufferDesc2.StructureByteStride = sizeof(uint16_t);

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = indices.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	ID3D11Buffer* indexBuffer;
	deviceComPtr->CreateBuffer(&bufferDesc2, &indexBufferData, &indexBuffer);

	std::vector<glm::vec4> pixels;
	pixels.resize(width * height);

	std::shared_ptr<Mesh> object = std::make_shared<Mesh>(vertices, indices); // one object that consist of triangles

	float		camThetaY = 0 / 180 * 3.141592f;
	glm::mat4x4 camRotateY = {
		{ cos(-camThetaY), 0.0f, sin(-camThetaY), 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ -sin(-camThetaY), 0.0f, cos(-camThetaY), 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f }
	};
	const float distCamToScreen = 1.0f;

	// Render
	std::vector<std::shared_ptr<Mesh>> meshes; // objects
	meshes.push_back(object);
	while (1)
	{
		std::fill(pixels.begin(), pixels.end(),
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

		std::vector<float> depthBuffer;
		depthBuffer.resize(pixels.size());
		std::fill(depthBuffer.begin(), depthBuffer.end(), 10.0f);

		for (const auto& mesh : meshes)
		{
			for (size_t i = 0; i < mesh->indices.size(); i += 3)
			{
				const size_t i1 = mesh->indices[i];
				const size_t i2 = mesh->indices[i + 1];
				const size_t i3 = mesh->indices[i + 2];

				glm::vec4 vertex1 = mesh->vertices[i1];
				glm::vec4 vertex2 = mesh->vertices[i2];
				glm::vec4 vertex3 = mesh->vertices[i3];
				glm::vec4 normal1 = mesh->normals[i1];
				glm::vec4 normal2 = mesh->normals[i2];
				glm::vec4 normal3 = mesh->normals[i3];

				glm::mat4x4	 modelMat = mesh->transformation.rotateZ * mesh->transformation.rotateY * mesh->transformation.rotateX * mesh->transformation.translation * mesh->transformation.scale;
				glm ::mat4x4 invTranspose = modelMat;
				invTranspose[3] = { 0, 0, 0, 1 };
				invTranspose = glm::transpose(glm::inverse(invTranspose));
				vertex1 = camRotateY * modelMat * vertex1;
				vertex2 = camRotateY * modelMat * vertex2;
				vertex3 = camRotateY * modelMat * vertex3;
				normal1 = camRotateY * invTranspose * normal1;
				normal2 = camRotateY * invTranspose * normal2;
				normal3 = camRotateY * invTranspose * normal3;

				// debug
				/*std::cout << vertex1 << std::endl;
				std::cout << vertex2 << std::endl;
				std::cout << vertex3 << std::endl
						  << std::endl;*/

				// perpective projection
				const float projScale1 = distCamToScreen / (distCamToScreen + vertex1.z);
				const float projScale2 = distCamToScreen / (distCamToScreen + vertex2.z);
				const float projScale3 = distCamToScreen / (distCamToScreen + vertex3.z);

				const glm::vec2 pointProj1 = { vertex1.x * projScale1, vertex1.y * projScale1 };
				const glm::vec2 pointProj2 = { vertex2.x * projScale2, vertex2.y * projScale2 };
				const glm::vec2 pointProj3 = { vertex3.x * projScale3, vertex3.y * projScale3 };

				const glm::vec2 poinrtNDC1 = pointProj1 / aspect_ratio;
				const glm::vec2 poinrtNDC2 = pointProj2 / aspect_ratio;
				const glm::vec2 poinrtNDC3 = pointProj3 / aspect_ratio;
			}
		}
	}

	// std::cout << "Perfect Until Now!" << std::endl;
	return 0;
}
