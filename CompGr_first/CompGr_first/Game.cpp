#include "Game.h"
#include "DisplayWin32.h"
#include "InputDevice.h"
#include "GameComponent.h"
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

Game::Game(LPCWSTR ApplicationName, int ScreenWidth, int ScreenHeight)
	: Name(ApplicationName)
	, ScreenWidth(ScreenWidth)
	, ScreenHeight(ScreenHeight)
	, Context(nullptr)
	, SwapChain(nullptr)
	, BackBuffer(nullptr)
	, RenderView(nullptr)
	, TotalTime(0.0f)
	, FrameCount(0)
	, IsExitRequested(false)
{
}

Game::~Game()
{
	DestroyResources();
}

void Game::Initialize()
{
	HINSTANCE InstanceHandle = GetModuleHandle(nullptr);

	std::srand((unsigned int)std::time(nullptr));//NOVOE


	Display = std::make_unique<DisplayWin32>(
		Name,
		InstanceHandle,
		ScreenWidth,
		ScreenHeight,
		[this](HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam) -> LRESULT
		{
			return this->MessageHandler(WindowHandle, Message, WParam, LParam);
		});

	Input = std::make_unique<InputDevice>(this);

	D3D_FEATURE_LEVEL FeatureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

	DXGI_SWAP_CHAIN_DESC SwapDescription = {};
	SwapDescription.BufferCount = 2;
	SwapDescription.BufferDesc.Width = ScreenWidth;
	SwapDescription.BufferDesc.Height = ScreenHeight;
	SwapDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapDescription.BufferDesc.RefreshRate.Numerator = 60;
	SwapDescription.BufferDesc.RefreshRate.Denominator = 1;
	SwapDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapDescription.OutputWindow = Display->GetWindowHandle();
	SwapDescription.Windowed = true;
	SwapDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapDescription.SampleDesc.Count = 1;
	SwapDescription.SampleDesc.Quality = 0;

	auto Result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		FeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&SwapDescription,
		&SwapChain,
		&Device,
		nullptr,
		&Context);

	if (FAILED(Result))
	{
		std::cerr << "Failed to create D3D11 device and swap chain." << std::endl;
		return;
	}

	CreateBackBuffer();
	PrepareResources();

	for (auto& Component : Components)
	{
		Component->Initialize();
	}
}

void Game::Run()
{
	StartTime = std::chrono::steady_clock::now();
	PreviousTime = StartTime;

	MSG Message = {};
	while (!IsExitRequested)
	{
		while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		if (Message.message == WM_QUIT)
		{
			IsExitRequested = true;
		}

		auto CurrentTime = std::chrono::steady_clock::now();
		float DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(CurrentTime - PreviousTime).count() / 1000000.0f;
		PreviousTime = CurrentTime;

		TotalTime += DeltaTime;
		FrameCount++;

		if (TotalTime > 1.0f)
		{
			float FramesPerSecond = FrameCount / TotalTime;
			TotalTime -= 1.0f;

			WCHAR TitleText[256];
			swprintf_s(TitleText, TEXT("FPS: %f"), FramesPerSecond);
			SetWindowText(Display->GetWindowHandle(), TitleText);

			FrameCount = 0;
		}

		PrepareFrame();
		Update(DeltaTime);
		Draw();
		SwapChain->Present(1, 0);
	}
}

void Game::PrepareFrame()
{
	Context->ClearState();
	RestoreTargets();

	D3D11_VIEWPORT Viewport = {};
	Viewport.Width = static_cast<float>(ScreenWidth);
	Viewport.Height = static_cast<float>(ScreenHeight);
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	Viewport.MinDepth = 0;
	Viewport.MaxDepth = 1.0f;

	Context->RSSetViewports(1, &Viewport);
}

void Game::Update(float DeltaTime)
{
	if (Input->IsKeyDown('C')) MainCamera.SwitchMode();       // FPS <-> Orbit
	if (Input->IsKeyDown('P')) MainCamera.SwitchProjection();
	MainCamera.Update(DeltaTime);//обновл€ю и камеру
	for (auto& Component : Components)
	{
		Component->Update(DeltaTime);
	}
}

void Game::Draw()
{
	float t = fmod(TotalTime, 5.0f) / 5.0f; // 0..1 циклично каждые 2 секунды
	float ClearColor[] = { 0.5f * t, 0.0f, 0.5f * (1.0f - t), 1.0f };
	Context->ClearRenderTargetView(RenderView, ClearColor);

	for (auto& Component : Components)
	{
		Component->Draw();
	}

	Context->OMSetRenderTargets(0, nullptr, nullptr);
}

void Game::PrepareResources()
{
}

void Game::CreateBackBuffer()
{
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer));
	Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderView);
}

void Game::RestoreTargets()
{
	Context->OMSetRenderTargets(1, &RenderView, nullptr);
}

void Game::DestroyResources()
{
	for (auto& Component : Components)
	{
		Component->DestroyResources();
	}

	if (RenderView)
	{
		RenderView->Release();
		RenderView = nullptr;
	}

	if (BackBuffer)
	{
		BackBuffer->Release();
		BackBuffer = nullptr;
	}

	if (SwapChain)
	{
		SwapChain->Release();
		SwapChain = nullptr;
	}

	if (Context)
	{
		Context->Release();
		Context = nullptr;
	}
}

ID3D11Device* Game::GetDevice() const
{
	return Device.Get();
}

ID3D11DeviceContext* Game::GetDeviceContext() const
{
	return Context;
}

IDXGISwapChain* Game::GetSwapChain() const
{
	return SwapChain;
}

DisplayWin32* Game::GetDisplay() const
{
	return Display.get();
}

InputDevice* Game::GetInputDevice() const
{
	return Input.get();
}

int Game::GetScreenWidth() const
{
	return ScreenWidth;
}

int Game::GetScreenHeight() const
{
	return ScreenHeight;
}

float Game::GetTotalTime() const
{
	return TotalTime;
}

void Game::AddComponent(std::unique_ptr<GameComponent> Component)
{
	Components.push_back(std::move(Component));
}

LRESULT Game::MessageHandler(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
	switch (Message)
	{
	case WM_KEYDOWN:
	{
		unsigned int KeyCode = static_cast<unsigned int>(WParam);
		std::cout << "Key: " << KeyCode << std::endl;

		if (Input)
		{
			Input->OnKeyDown(KeyCode);
		}

		if (KeyCode == 27)
		{
			PostQuitMessage(0);
		}

		return 0;
	}
	case WM_KEYUP:
	{
		if (Input)
		{
			Input->OnKeyUp(static_cast<unsigned int>(WParam));
		}
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		if (Input)
		{
			int PositionX = LOWORD(LParam);
			int PositionY = HIWORD(LParam);

			static int lastX = PositionX;
			static int lastY = PositionY;

			int dx = PositionX - lastX;
			int dy = PositionY - lastY;

			lastX = PositionX;
			lastY = PositionY;

			Input->OnMouseMove(PositionX, PositionY);
			MainCamera.ProcessMouseDelta((float)dx, (float)dy);

		}
		return 0;
	}

	case WM_MOUSEWHEEL://колесо
	{
		// прокрутка колЄсика мыши
		int delta = GET_WHEEL_DELTA_WPARAM(WParam);
		MainCamera.ProcessScroll((float)delta * 0.1f); // масштабируем скорость прокрутки
		return 0;
	}

	default:
	{
		return DefWindowProc(WindowHandle, Message, WParam, LParam);
	}
	}
}
