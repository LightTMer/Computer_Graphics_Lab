#include "PongComponent.h"
#include "Game.h"
#include "InputDevice.h"
#include "DisplayWin32.h"
#include <d3dcompiler.h>
#include <directxmath.h>
#include <DirectXCollision.h>
#include <iostream>

using namespace std;


PongComponent::PongComponent(Game* GameInstance)
	: GameComponent(GameInstance),
	VertexBuffer(nullptr),
	IndexBuffer(nullptr),
	VertexShader(nullptr),
	PixelShader(nullptr),
	Layout(nullptr),
	ConstantBuffer(nullptr),
	LeftScore(0), 
	RightScore(0),
	IndexCount(0)
{
	LeftPaddle = Vector2(-0.9f, 0.0f);
	RightPaddle = Vector2(0.9f, 0.0f);

	PaddleSpeed = 1.5f;
	

}

void PongComponent::Initialize()
{
	auto device = OwningGame->GetDevice();

	Vertex vertices[] =
	{
		{{-0.05f,  0.3f, 0, 1}, {1,1,1,1}},
		{{-0.05f, -0.3f, 0, 1}, {1,1,1,1}},
		{{ 0.05f, -0.3f, 0, 1}, {1,1,1,1}},
		{{ 0.05f,  0.3f, 0, 1}, {1,1,1,1}},
	};

	unsigned int indices[] = { 0,1,2, 0,2,3 };
	IndexCount = 6;

	// Vertex буфер
	D3D11_BUFFER_DESC vb = {};
	vb.Usage = D3D11_USAGE_DEFAULT;
	vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb.ByteWidth = sizeof(vertices);

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = vertices;

	device->CreateBuffer(&vb, &data, &VertexBuffer);

	// Index буфер
	D3D11_BUFFER_DESC ib = {};
	ib.Usage = D3D11_USAGE_DEFAULT;
	ib.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ib.ByteWidth = sizeof(indices);

	D3D11_SUBRESOURCE_DATA idata = {};
	idata.pSysMem = indices;

	device->CreateBuffer(&ib, &idata, &IndexBuffer);

	//константный7. Константный буфер содержит матрицу трансформации. Мы обновляем её перед отрисовкой каждой ракетки, что позволяет использовать один меш для всех объектов.
	D3D11_BUFFER_DESC cb = {}; // ДВЖИЕНИЕ РАКЕТКОЙ
	cb.Usage = D3D11_USAGE_DEFAULT;
	cb.ByteWidth = sizeof(CBData);
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 

	device->CreateBuffer(&cb, nullptr, &ConstantBuffer);

	// загрузка шейдера

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;

	D3DCompileFromFile(L"./Shaders/PongShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, nullptr); //вершинный
	D3DCompileFromFile(L"./Shaders/PongShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, nullptr); //пиксельный

	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &VertexShader);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &PixelShader);

	//мячик
	GameBall = Ball(0.03f);
	GameBall.Initialize(OwningGame->GetDevice());

	//layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &Layout);
	
	CD3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.FillMode = D3D11_FILL_SOLID; // Rasterizer State (отключение отсечения)
	rasterDesc.CullMode = D3D11_CULL_NONE;  // не отсекаем задние грани (back-face culling
	rasterDesc.FrontCounterClockwise = false; // ну и тупо по часовой стрелке порядок вершин 

	OwningGame->GetDevice()->CreateRasterizerState(&rasterDesc, &RasterState);
	
}

void PongComponent::Update(float DeltaTime)
{
	auto input = OwningGame->GetInputDevice();

	float paddleHalfHeight = 0.3f; // половина высоты ракетки
	float paddleWidth = 0.1f;
	float paddleHeight = 0.6f;

	// Левая ракетка (W/S)
	if (input->IsKeyDown('W'))
		LeftPaddle.y += PaddleSpeed * DeltaTime;
	if (input->IsKeyDown('S'))
		LeftPaddle.y -= PaddleSpeed * DeltaTime;

	// Ограничение сверху и снизу
	if (LeftPaddle.y + paddleHalfHeight > 1.0f) LeftPaddle.y = 1.0f - paddleHalfHeight;
	if (LeftPaddle.y - paddleHalfHeight < -1.0f) LeftPaddle.y = -1.0f + paddleHalfHeight;

	// Правая ракетка (стрелки)
	if (input->IsKeyDown(VK_UP))
		RightPaddle.y += PaddleSpeed * DeltaTime;
	if (input->IsKeyDown(VK_DOWN))
		RightPaddle.y -= PaddleSpeed * DeltaTime;

	// Ограничение сверху и снизу
	if (RightPaddle.y + paddleHalfHeight > 1.0f) RightPaddle.y = 1.0f - paddleHalfHeight;
	if (RightPaddle.y - paddleHalfHeight < -1.0f) RightPaddle.y = -1.0f + paddleHalfHeight;

	DirectX::BoundingBox leftBox(
		DirectX::XMFLOAT3(LeftPaddle.x, LeftPaddle.y, 0),
		DirectX::XMFLOAT3(paddleWidth / 2, paddleHeight / 2, 0)
	);

	DirectX::BoundingBox rightBox(
		DirectX::XMFLOAT3(RightPaddle.x, RightPaddle.y, 0),
		DirectX::XMFLOAT3(paddleWidth / 2, paddleHeight / 2, 0)
	);

	GameBall.Update(DeltaTime, leftBox, rightBox, LeftScore, RightScore);
	std::cout << "\rScore: " << LeftScore << " - " << RightScore << " " << std::flush;


	//NOVOE
	//ПРОВЕРКА СЧЕТА 
	int newTotal = LeftScore + RightScore;

	if (newTotal != TotalScore)
	{
		TotalScore = newTotal;

		if (TotalScore % 5 == 0)
		{
			BonusActive = true;

			float x = ((rand() % 200) / 100.0f - 1.0f) * 0.8f;
			float y = ((rand() % 200) / 100.0f - 1.0f) * 0.8f;

			BonusPosition = Vector2(x, y);
		}
	}
	//  СТОЛКНОВЕНИЕ С БОНУСОМ 
	if (BonusActive)
	{
		Vector2 ballPos = GameBall.GetPosition();
		float r = GameBall.GetRadius();

		if (fabs(ballPos.x - BonusPosition.x) < (r + BonusSize) &&
			fabs(ballPos.y - BonusPosition.y) < (r + BonusSize))
		{
			BonusActive = false;

			// ВКЛЮЧАЕМ ЭФФЕКТ
			InvisibleActive = true;
			InvisibleTimer = 5.0f;
		}
	}

	//  ТАЙМЕР 
	if (InvisibleActive)
	{
		InvisibleTimer -= DeltaTime;

		if (InvisibleTimer <= 0.0f)
		{
			InvisibleActive = false;
		}
	}

}

void PongComponent::Draw()
{
	auto context = OwningGame->GetDeviceContext();
	context->RSSetState(RasterState); // layout

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(Layout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(VertexShader, nullptr, 0);
	context->PSSetShader(PixelShader, nullptr, 0);

	////  ЛЕВАЯ РАКЕТКА 
	//CBData cb;

	//cb.transform = DirectX::XMMatrixTranspose(
	//	DirectX::XMMatrixTranslation(LeftPaddle.x, LeftPaddle.y, 0.0f)
	//);

	//context->UpdateSubresource(ConstantBuffer, 0, nullptr, &cb, 0, 0);
	//context->VSSetConstantBuffers(0, 1, &ConstantBuffer);

	//context->DrawIndexed(IndexCount, 0, 0);

	//// ПРАВАЯ РАКЕТКА
	//cb.transform = DirectX::XMMatrixTranspose(
	//	DirectX::XMMatrixTranslation(RightPaddle.x, RightPaddle.y, 0.0f)
	//);

	//context->UpdateSubresource(ConstantBuffer, 0, nullptr, &cb, 0, 0);

	//context->DrawIndexed(IndexCount, 0, 0);

	// РАКЕТКИ NOVOE
	if (!InvisibleActive)
	{
		CBData cb;

		// ЛЕВАЯ
		cb.transform = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixTranslation(LeftPaddle.x, LeftPaddle.y, 0.0f)
		);

		context->UpdateSubresource(ConstantBuffer, 0, nullptr, &cb, 0, 0);
		context->VSSetConstantBuffers(0, 1, &ConstantBuffer);
		context->DrawIndexed(IndexCount, 0, 0);

		// ПРАВАЯ
		cb.transform = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixTranslation(RightPaddle.x, RightPaddle.y, 0.0f)
		);

		context->UpdateSubresource(ConstantBuffer, 0, nullptr, &cb, 0, 0);
		context->DrawIndexed(IndexCount, 0, 0);
	}

	//Мячч
	GameBall.Draw(context, ConstantBuffer);


	//NOVOE:
	// БОНУС
	if (BonusActive)
	{
		CBData cb;

		cb.transform = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixTranslation(BonusPosition.x, BonusPosition.y, 0.0f)
		);

		context->UpdateSubresource(ConstantBuffer, 0, nullptr, &cb, 0, 0);
		context->DrawIndexed(IndexCount, 0, 0);
	}
}

void PongComponent::DestroyResources()
{
	if (VertexBuffer) VertexBuffer->Release();
	if (IndexBuffer) IndexBuffer->Release();
	if (VertexShader) VertexShader->Release();
	if (PixelShader) PixelShader->Release();
	if (Layout) Layout->Release();
	if (ConstantBuffer) ConstantBuffer->Release();
	if (RasterState) { RasterState->Release(); RasterState = nullptr; }
}

PongComponent::~PongComponent() = default;