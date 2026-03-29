#pragma once

#include <SimpleMath.h>
#include <DirectXCollision.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

using namespace DirectX::SimpleMath;

class Ball
{
public:
    Ball(float radius = 0.05f);

    void Initialize(ID3D11Device* device, ID3D11Buffer* sharedCB = nullptr);
    void Update(float DeltaTime, const DirectX::BoundingBox& LeftBox, const DirectX::BoundingBox& RightBox,
        int& LeftScore, int& RightScore);
    void Draw(ID3D11DeviceContext* context, ID3D11Buffer* constantBuffer);

    Vector2 GetPosition() const { return Position; }
    float GetRadius() const { return Radius; }
    ~Ball();

private:
    Vector2 Position;
    Vector2 Velocity;
    float Radius;

    struct Vertex
    {
        DirectX::XMFLOAT4 pos;
        DirectX::XMFLOAT4 col;
    };

    ID3D11Buffer* VertexBuffer;
    ID3D11Buffer* IndexBuffer;
    UINT IndexCount;

    struct CBData
    {
        DirectX::XMMATRIX transform;
    };

    //sppe
    float Speed = 1.5f;
    float MaxSpeed = 3.5f;
    float SpeedIncrease = 1.1f;
};

