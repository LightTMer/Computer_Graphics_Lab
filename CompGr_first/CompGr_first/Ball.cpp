#include "Ball.h"
#include <d3dcompiler.h>
#include <directxmath.h>
#include <cstdlib>
#include <ctime>

Ball::Ball(float radius)
    : Radius(radius), VertexBuffer(nullptr), IndexBuffer(nullptr), IndexCount(6)
{
    Position = Vector2(0.0f, 0.0f);

    // сразу горизонтальный компонент не ноль
    float angle = ((std::rand() % 120) - 60) * DirectX::XM_PI / 180.0f;
    Velocity = Vector2(cosf(angle), sinf(angle));
    Velocity.Normalize();
    Velocity *= Speed; //скорость
}

void Ball::Initialize(ID3D11Device* device, ID3D11Buffer* sharedCB)
{
    Vertex vertices[] =
    {
        {{-Radius,  Radius, 0,1}, {1,1,1,1}},
        {{-Radius, -Radius, 0,1}, {1,1,1,1}},
        {{ Radius, -Radius, 0,1}, {1,1,1,1}},
        {{ Radius,  Radius, 0,1}, {1,1,1,1}},
    };

    unsigned int indices[] = { 0,1,2, 0,2,3 };
    IndexCount = 6;

    D3D11_BUFFER_DESC vb = {};
    vb.Usage = D3D11_USAGE_DEFAULT;
    vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vb.ByteWidth = sizeof(vertices);
    D3D11_SUBRESOURCE_DATA data = { vertices };
    device->CreateBuffer(&vb, &data, &VertexBuffer);

    D3D11_BUFFER_DESC ib = {};
    ib.Usage = D3D11_USAGE_DEFAULT;
    ib.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ib.ByteWidth = sizeof(indices);
    D3D11_SUBRESOURCE_DATA idata = { indices };
    device->CreateBuffer(&ib, &idata, &IndexBuffer);
}

void Ball::Update(float DeltaTime, const DirectX::BoundingBox& LeftBox, const DirectX::BoundingBox& RightBox,
    int& LeftScore, int& RightScore) //обновл€ет константный буфер с матрицей трансформации (позици€ м€ча)
{
    Position += Velocity * DeltaTime;

    // —толкновение со стенами
    if (Position.y + Radius > 1.0f) { Position.y = 1.0f - Radius; Velocity.y = -Velocity.y; }
    if (Position.y - Radius < -1.0f) { Position.y = -1.0f + Radius; Velocity.y = -Velocity.y; }

    // BoundingBox дл€ м€ча
    DirectX::BoundingBox ballBox(
        DirectX::XMFLOAT3(Position.x, Position.y, 0),             // центр
        DirectX::XMFLOAT3(Radius, Radius, 0)                     // extents (половина размера)
    );

    if (ballBox.Intersects(LeftBox))
    {
        Velocity.x = fabs(Velocity.x); //отскакивает вправо
    
        Speed *= SpeedIncrease;
        if (Speed > MaxSpeed) Speed = MaxSpeed;

        Velocity.Normalize();
        Velocity *= Speed;

        Position.x = LeftBox.Center.x + (LeftBox.Extents.x + Radius);
    }
    else if (ballBox.Intersects(RightBox))
    {
        Velocity.x = -fabs(Velocity.x); // отскакиваем влево
        
        Speed *= SpeedIncrease;//увеличим скорость
        if (Speed > MaxSpeed) Speed = MaxSpeed;

        Velocity.Normalize();
        Velocity *= Speed;

        Position.x = RightBox.Center.x - (RightBox.Extents.x + Radius);
    }

    // √олы
    if (Position.x - Radius < -1.0f)
    {
        RightScore++;
        Position = Vector2(0, 0);
        float angle = ((std::rand() % 120) - 60) * DirectX::XM_PI / 180.0f;
        Velocity = Vector2(cosf(angle), sinf(angle));
        Velocity.Normalize();
        Velocity *= Speed;
    }
    else if (Position.x + Radius > 1.0f)
    {
        LeftScore++;
        Position = Vector2(0, 0);
        float angle = ((std::rand() % 120) + 120) * DirectX::XM_PI / 180.0f;
        Velocity = Vector2(cosf(angle), sinf(angle));
        Velocity.Normalize();
        Speed = 2.5f;
        Velocity.Normalize();
        Velocity *= Speed;
    }
}

void Ball::Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer)
{
    CBData cb;
    cb.transform = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(Position.x, Position.y, 0));
    context->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

    context->VSSetConstantBuffers(0, 1, &constantBuffer);//w

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(IndexCount, 0, 0);
}
Ball::~Ball() = default;