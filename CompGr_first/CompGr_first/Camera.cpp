#include "Camera.h"
#include <algorithm>
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;
using namespace std;

Camera::Camera()
{
    UpdateProjection();
    UpdateView();
}

void Camera::SwitchMode()
{
  /*  if (mode == CameraMode::FPS)
        mode = CameraMode::Orbit;
    else
        mode = CameraMode::FPS;*/
    if (mode == CameraMode::FPS)
    {
        mode = CameraMode::Orbit;
        // сбросить yaw/pitch, чтобы Orbit начинался корректно
        yaw = 0.0f;
        pitch = 0.0f;
    }
    else
    {
        mode = CameraMode::FPS;
    }
}

void Camera::SwitchProjection()
{
    switch (projectionType)
    {
    case ProjectionType::Perspective: projectionType = ProjectionType::WideFov; break;
    case ProjectionType::WideFov:    projectionType = ProjectionType::NarrowFov; break;
    case ProjectionType::NarrowFov:  projectionType = ProjectionType::Perspective; break;
    }
    UpdateProjection();
}

void Camera::ProcessMouseDelta(float dx, float dy)
{
    float sensitivity = 0.01f;
    yaw += dx * sensitivity;
    pitch += dy * sensitivity;
    pitch = std::clamp(pitch, -DirectX::XM_PIDIV2 + 0.01f, DirectX::XM_PIDIV2 - 0.01f);
}

void Camera::ProcessScroll(float delta)
{
    if (mode == CameraMode::Orbit)
    {
        orbitDistance -= delta;
        //orbitDistance = (orbitDistance < 1.0f) ? 1.0f : orbitDistance;
        orbitDistance = std::clamp(orbitDistance, 1.0f, 50.0f); //дабы камера не улетала
    }
}

void Camera::Update(float dt)
{
    // Для FPS мб сделать WASD
    // Здесь вращение мышью и орбиту сделаю так
    UpdateView();
}

void Camera::UpdateView()
{
    if (mode == CameraMode::FPS)
    {
        Vector3 forward = Vector3::Transform(Vector3::Forward, Matrix::CreateFromYawPitchRoll(yaw, pitch, 0));
        Vector3 lookAt = position + forward;
        view = Matrix::CreateLookAt(position, lookAt, Vector3::Up);
    }
    else if (mode == CameraMode::Orbit)
    {
        Vector3 camPos = Vector3(orbitDistance, 0, 0);
        Matrix rot = Matrix::CreateFromYawPitchRoll(yaw, pitch, 0);
        camPos = Vector3::Transform(camPos, rot) + lookAtPoint;
        view = Matrix::CreateLookAt(camPos, lookAtPoint, Vector3::Up);
    }
}

void Camera::UpdateProjection()
{
    switch (projectionType)
    {
    case ProjectionType::Perspective:
        fov = DirectX::XM_PIDIV4; break;
    case ProjectionType::WideFov:
        fov = DirectX::XM_PIDIV2; break;
    case ProjectionType::NarrowFov:
        fov = DirectX::XM_PIDIV4 / 2; break;
    }
    proj = Matrix::CreatePerspectiveFieldOfView(fov, aspect, nearPlane, farPlane);
}