#include "SphereComponent.h"
#include "Game.h"
#include <d3dcompiler.h>

SphereComponent::SphereComponent(Game* game, float radius, int slices, int stacks)
    : GameComponent(game), Radius(radius), SliceCount(slices), StackCount(stacks)
{
}

void SphereComponent::Initialize()
{
    auto device = OwningGame->GetDevice();

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // ¬≈–ÿ»Õ€
    for (int i = 0; i <= StackCount; ++i)
    {
        float phi = i * DirectX::XM_PI / StackCount;

        for (int j = 0; j <= SliceCount; ++j)
        {
            float theta = j * DirectX::XM_2PI / SliceCount;

            Vertex v;

            v.pos.x = Radius * sin(phi) * cos(theta);
            v.pos.y = Radius * cos(phi);
            v.pos.z = Radius * sin(phi) * sin(theta);

            v.col = Vector4(1, 1, 1, 1);

            vertices.push_back(v);
        }
    }

    // »Õƒ≈ —€
    for (int i = 0; i < StackCount; ++i)
    {
        for (int j = 0; j < SliceCount; ++j)
        {
            int first = i * (SliceCount + 1) + j;
            int second = first + SliceCount + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    IndexCount = (UINT)indices.size();

    // VB
    D3D11_BUFFER_DESC vb = {};
    vb.ByteWidth = (UINT)(vertices.size() * sizeof(Vertex));
    vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA data = { vertices.data() };
    device->CreateBuffer(&vb, &data, &VertexBuffer);

    // IB
    D3D11_BUFFER_DESC ib = {};
    ib.ByteWidth = (UINT)(indices.size() * sizeof(unsigned int));
    ib.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA idata = { indices.data() };
    device->CreateBuffer(&ib, &idata, &IndexBuffer);

    D3D11_BUFFER_DESC cb = {};
    cb.ByteWidth = sizeof(CBData);
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer(&cb, nullptr, &ConstantBuffer);

    ID3DBlob* vs = nullptr;
    ID3DBlob* ps = nullptr;

    D3DCompileFromFile(L"./Shaders/Basic3D.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vs, nullptr);
    ID3DBlob* error = nullptr;

    HRESULT hr = D3DCompileFromFile(
        L"./Shaders/Basic3D.hlsl",
        nullptr,
        nullptr,
        "PSMain",
        "ps_5_0",
        0,
        0,
        &ps,
        &error
    );

    if (FAILED(hr))
    {
        if (error)
        {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
    }

    device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &VS);
    device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &PS);   

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
    };

    device->CreateInputLayout(
        layoutDesc,
        2,
        vs->GetBufferPointer(),
        vs->GetBufferSize(),
        &Layout
    );

    //‡
    D3D11_RASTERIZER_DESC rs = {};
    rs.FillMode = D3D11_FILL_WIREFRAME; // ¬Œ“ ›“Œ ¬¿∆ÕŒ
    rs.CullMode = D3D11_CULL_NONE;

    device->CreateRasterizerState(&rs, &WireframeState);
}

void SphereComponent::Update(float dt)
{
    Rotation += dt;
    OrbitAngle += dt;

    Matrix local =
        Matrix::CreateRotationY(Rotation) *
        Matrix::CreateTranslation(OrbitRadius, 0, 0);

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

void SphereComponent::Draw()
{
    auto ctx = OwningGame->GetDeviceContext();

    ctx->RSSetState(WireframeState);//a

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    ctx->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    ctx->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    ctx->IASetInputLayout(Layout);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ctx->VSSetShader(VS, nullptr, 0);
    ctx->PSSetShader(PS, nullptr, 0);

    CBData cb;


   /* Matrix view = Matrix::CreateLookAt(
        Vector3(0, 5, -15),
        Vector3(0, 0, 0),
        Vector3::Up
    );

    Matrix proj = Matrix::CreatePerspectiveFieldOfView(
        DirectX::XM_PIDIV4,
        1.0f,
        0.1f,
        100.0f
    );*/

    cb.world = WorldMatrix.Transpose();
    //cb.view = view.Transpose();
    //cb.proj = proj.Transpose();
    cb.view = OwningGame->MainCamera.GetViewMatrix().Transpose();
    cb.proj = OwningGame->MainCamera.GetProjectionMatrix().Transpose();

    ctx->UpdateSubresource(ConstantBuffer, 0, nullptr, &cb, 0, 0);
    ctx->VSSetConstantBuffers(0, 1, &ConstantBuffer);

    ctx->DrawIndexed(IndexCount, 0, 0);
}

SphereComponent::~SphereComponent() = default;

void SphereComponent::DestroyResources()
{
    if (WireframeState) { WireframeState->Release(); WireframeState = nullptr; }
    if (VertexBuffer) VertexBuffer->Release();
    if (IndexBuffer) IndexBuffer->Release();
    if (ConstantBuffer) ConstantBuffer->Release();
    if (VS) VS->Release();
    if (PS) PS->Release();
    if (Layout) Layout->Release();
}