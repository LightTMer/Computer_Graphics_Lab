#pragma once
#include <SimpleMath.h>
#include <Windows.h>

using namespace DirectX::SimpleMath;

enum class CameraMode { FPS, Orbit };
enum class ProjectionType { Perspective, WideFov, NarrowFov };

class Camera
{
public:
    Camera();

    void Update(float dt);        // обновление позиции/вращения камеры
    void SwitchMode();            // переключение FPS <-> Orbit
    void SwitchProjection();      // переключение перспектив

    Matrix GetViewMatrix() const { return view; }
    Matrix GetProjectionMatrix() const { return proj; }

    // Параметры для управления мышью/клавишами
    void ProcessMouseDelta(float dx, float dy);
    void ProcessScroll(float delta);

private:
    CameraMode mode = CameraMode::Orbit;
    ProjectionType projectionType = ProjectionType::Perspective;

    // Положение и ориентация
    Vector3 position = Vector3(0, 2, -10);
    float yaw = 0.0f;
    float pitch = 0.0f;

    // Орбитальная камера
    Vector3 lookAtPoint = Vector3(0, 0, 0);
    float orbitDistance = 15.0f;

    // Проекция
    float fov = DirectX::XM_PIDIV4;
    float aspect = 1.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    Matrix view;
    Matrix proj;

    void UpdateView();
    void UpdateProjection();
}; 
