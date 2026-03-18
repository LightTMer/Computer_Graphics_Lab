#pragma once

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

protected:
	Game* OwningGame;
};
