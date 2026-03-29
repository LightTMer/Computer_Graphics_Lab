#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include <vector>
using namespace DirectX::SimpleMath;
class Game;

class GameComponent
{
public:
	GameComponent(Game* GameInstance);
	virtual ~GameComponent();

	virtual void Initialize() = 0; //инициализация ресурсов
	virtual void Update(float DeltaTime) = 0; //обновление логики, типа - движения игрока (для pong и AI например)
	virtual void Draw() = 0; //отрисовка
	virtual void DestroyResources() = 0; //спасение ресурсов gpu
	GameComponent* Parent = nullptr;  // общий тип родителя
	Matrix WorldMatrix;

protected:
	Game* OwningGame;
};
