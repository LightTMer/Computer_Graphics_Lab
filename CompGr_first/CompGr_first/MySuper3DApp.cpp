#include "Game.h"
#include "TriangleComponent.h"
#include "PongComponent.h"
#include <memory>
#include "CubeComponent.h"
#include "SphereComponent.h"

int main()
{
	Game MyGame(L"My3DApp", 800, 800);

	//MyGame.AddComponent(std::make_unique<TriangleComponent>(&MyGame));
	//MyGame.AddComponent(std::make_unique<PongComponent>(&MyGame));

	//3тья лаба

	   // солнышко
	auto sun = std::make_unique<CubeComponent>(&MyGame);
	sun->OrbitRadius = 0.0f; // центр системы

	// ланеты
	std::vector<std::unique_ptr<GameComponent>> planets;

	float planetRadii[] = { 0.7f, 0.6f, 0.5f, 0.9f }; // размеры 
	float planetOrbits[] = { 3.0f, 5.0f, 7.0f, 9.0f }; // расстояние от солнца

	for (int i = 0; i < 4; ++i)
	{
		auto planet = std::make_unique<SphereComponent>(&MyGame, planetRadii[i]);
		planet->Parent = sun.get();     // вращение вокруг Солнца
		planet->OrbitRadius = planetOrbits[i];

		// луны для каждой планеты
		int moonCount = i + 1; // 1 луна для первой планеты, 2 для второй и т.д.
		for (int m = 0; m < moonCount; ++m)
		{
			float moonRadius = 0.2f + 0.05f * m;
			float moonOrbit = 0.5f + 0.5f * m;

			auto moon = std::make_unique<SphereComponent>(&MyGame, moonRadius);
			moon->Parent = planet.get();  // луна вращается вокруг планеты
			moon->OrbitRadius = moonOrbit;

			MyGame.AddComponent(std::move(moon));
		}

		MyGame.AddComponent(std::move(planet));
	}


	MyGame.AddComponent(std::move(sun));

	MyGame.Initialize();
	MyGame.Run();

	return 0;	
}