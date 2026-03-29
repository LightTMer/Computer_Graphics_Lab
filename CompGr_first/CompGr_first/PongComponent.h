#pragma once

#include "GameComponent.h"
#include <SimpleMath.h>
#include <d3d11.h>
#include <DirectXCollision.h> 
#include "Ball.h" // м€чик



using namespace DirectX::SimpleMath;

class PongComponent : public GameComponent
{
public:
    PongComponent(Game* GameInstance);
    ~PongComponent() override;

    void Initialize() override;
    void Update(float DeltaTime) override;
    void Draw() override;
    void DestroyResources() override;

private:

    struct Vertex
    {
        DirectX::XMFLOAT4 pos;
        DirectX::XMFLOAT4 col;
    };

    // позиции ракеток
    Vector2 LeftPaddle;
    Vector2 RightPaddle;
    float PaddleSpeed;


    // DirectX
    ID3D11Buffer* VertexBuffer;
    ID3D11Buffer* IndexBuffer;
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* Layout;
    ID3D11Buffer* ConstantBuffer; // двигать ракетку
    ID3D11RasterizerState* RasterState;// дл€ экрана
    UINT IndexCount;

    struct CBData
    {
        DirectX::XMMATRIX transform;
    };

    //м€чик:
    Ball GameBall;
    int LeftScore = 0;
    int RightScore = 0;


    //// ЅќЌ”— 

    bool BonusActive = false;
    Vector2 BonusPosition;
    float BonusSize = 0.05f;

    // счет
    int TotalScore = 0;

    // эффект (“ќЋ№ ќ invisibility)
    bool InvisibleActive = false;
    float InvisibleTimer = 0.0f;
};