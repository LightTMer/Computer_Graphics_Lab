#include "CubeComponent.h"
#include "Game.h"
#include <d3dcompiler.h>

CubeComponent::CubeComponent(Game* game)
    : GameComponent(game)
{
}

void CubeComponent::Initialize()
{
    auto device = OwningGame->GetDevice();

	Vertex vertices[] =
	{
        {{-0.5f,-0.5f,-0.5f},{1,0,0,1}},
        {{-0.5f, 0.5f,-0.5f},{0,1,0,1}},
        {{ 0.5f, 0.5f,-0.5f},{0,0,1,1}},
        {{ 0.5f,-0.5f,-0.5f},{1,1,0,1}},

        {{-0.5f,-0.5f, 0.5f},{1,0,1,1}},
        {{-0.5f, 0.5f, 0.5f},{0,1,1,1}},
        {{ 0.5f, 0.5f, 0.5f},{1,1,1,1}},
        {{ 0.5f,-0.5f, 0.5f},{0,0,0,1}},
	};

    unsigned int indices[] = {
        0,1,2, 0,2,3,
        4,6,5,  4,7,6,
        4,5,1, 4,1,0,
        3,2,6, 3,6,7,
        1,5,6, 1,6,2,
        4,0,3, 4,3,7
    };

    IndexCount = _countof(indices);

    // VB
    D3D11_BUFFER_DESC vb = {};
    vb.ByteWidth = sizeof(vertices);
    vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA data = { vertices };
    device->CreateBuffer(&vb, &data, &VertexBuffer);

    // IB
    D3D11_BUFFER_DESC ib = {};
    ib.ByteWidth = sizeof(indices);
    ib.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA idata = { indices };
    device->CreateBuffer(&ib, &idata, &IndexBuffer);

    // CB
    D3D11_BUFFER_DESC cb = {};
    cb.ByteWidth = sizeof(CBData);
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;  
    device->CreateBuffer(&cb, nullptr, &ConstantBuffer);

    ID3DBlob* vs = nullptr;
    ID3DBlob* ps = nullptr;

    D3DCompileFromFile(L"./Shaders/Basic3D.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vs, nullptr);
    D3DCompileFromFile(L"./Shaders/Basic3D.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &ps, nullptr);


    device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &VS);
    device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &PS);

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
    };

    device->CreateInputLayout(layout, 2, vs->GetBufferPointer(), vs->GetBufferSize(), &Layout);

}


void CubeComponent::Update(float dt)
{
    Rotation += dt;
    OrbitAngle += dt;

    // локальное движение (вокруг себя + смещение)
    Matrix local =
        Matrix::CreateRotationY(Rotation) *
        Matrix::CreateTranslation(OrbitRadius, 0, 0);

    // вращение орбиты
    Matrix orbit = Matrix::CreateRotationY(OrbitAngle);

    if (Parent)
    {
        WorldMatrix = local * orbit * Parent->WorldMatrix;
    }
    else
    {
        WorldMatrix = local * orbit;
    }

}

void CubeComponent::Draw()
{
    auto ctx = OwningGame->GetDeviceContext();

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    ctx->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    ctx->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    ctx->IASetInputLayout(Layout);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ctx->VSSetShader(VS, nullptr, 0);
    ctx->PSSetShader(PS, nullptr, 0);

    CBData cb;

    // камера
    //Matrix view = Matrix::CreateLookAt(
    //    Vector3(0, 5, -15),   // позиция камеры
    //    Vector3(0, 0, 0),    // куда смотрим
    //    Vector3::Up
    //);

    //float aspect = 800.0f / 800.0f;

    //Matrix proj = Matrix::CreatePerspectiveFieldOfView(
    //    DirectX::XM_PIDIV4,
    //    aspect,
    //    0.1f,
    //    100.0f
    //);

    cb.world = WorldMatrix.Transpose();
 /*   cb.view = view.Transpose();
    cb.proj = proj.Transpose();*/
    cb.view = OwningGame->MainCamera.GetViewMatrix().Transpose();
    cb.proj = OwningGame->MainCamera.GetProjectionMatrix().Transpose();

    ctx->UpdateSubresource(ConstantBuffer, 0, nullptr, &cb, 0, 0);
    ctx->VSSetConstantBuffers(0, 1, &ConstantBuffer);
    ctx->DrawIndexed(IndexCount, 0, 0);
}

CubeComponent::~CubeComponent() = default;

void CubeComponent::DestroyResources()
{
    if (VertexBuffer) VertexBuffer->Release();
    if (IndexBuffer) IndexBuffer->Release();
    if (ConstantBuffer) ConstantBuffer->Release();
    if (VS) VS->Release();
    if (PS) PS->Release();
    if (Layout) Layout->Release();
}