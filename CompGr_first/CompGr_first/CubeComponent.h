#pragma once
#include "GameComponent.h"
#include <d3d11.h>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class CubeComponent : public GameComponent
{
public:
    CubeComponent(Game* game);
    ~CubeComponent() override;

    void Initialize() override;
    void Update(float dt) override;
    void Draw() override;
    void DestroyResources() override;

    void SetPosition(const Vector3& pos) { Position = pos; }

    //GameComponent* Parent = nullptr;
    float OrbitAngle = 0.0f;
    float OrbitRadius = 2.0f;
    Vector3 Center = Vector3(0, 0, 0);


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