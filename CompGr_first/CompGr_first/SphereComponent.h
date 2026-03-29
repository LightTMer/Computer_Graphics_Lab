#pragma once
#include "GameComponent.h"
#include <d3d11.h>
#include <SimpleMath.h>
#include <vector>

using namespace DirectX::SimpleMath;

class SphereComponent : public GameComponent
{
public:
    SphereComponent(Game* game, float radius = 0.5f, int slices = 20, int stacks = 20);
    ~SphereComponent() override;

    void Initialize() override;
    void Update(float dt) override;
    void Draw() override;
    void DestroyResources() override;

    //GameComponent* Parent = nullptr;

    float OrbitAngle = 0.0f;
    float OrbitRadius = 2.0f;
    Vector3 Center = Vector3(0, 0, 0);
    ID3D11RasterizerState* WireframeState = nullptr;

private:
    struct Vertex
    {
        Vector3 pos;
        Vector4 col;
    };

    struct CBData
    {
        Matrix world;
        Matrix view;
        Matrix proj;
    };

    float Radius;
    int SliceCount;
    int StackCount;

    Vector3 Position = Vector3(0, 0, 0);
    float Rotation = 0.0f;
    Matrix WorldMatrix;

    ID3D11Buffer* VertexBuffer = nullptr;
    ID3D11Buffer* IndexBuffer = nullptr;
    ID3D11Buffer* ConstantBuffer = nullptr;

    ID3D11VertexShader* VS = nullptr;
    ID3D11PixelShader* PS = nullptr;
    ID3D11InputLayout* Layout = nullptr;

    UINT IndexCount = 0;
};