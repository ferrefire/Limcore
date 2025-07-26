#pragma once

#include "point.hpp"
#include "matrix.hpp"

#include <iostream>

struct CameraConfig
{
	uint32_t width = 400;
	uint32_t height = 400;
	float near = 0.01;
	float far = 100;
	float fov = 60;
	float speed = 3;
	float sensitivity = 0.1;
};

class Camera
{
	private:
		CameraConfig config{};

		point3D position = point3D(0);
		point3D direction = point3D(0, 0, 1);
		point3D angles = point3D(0, 0, 0);
		point3D right = point3D(1, 0, 0);
		point3D up = point3D(0, 1, 0);

		mat4 view = mat4::Identity();
		mat4 projection = mat4::Identity();

		bool updated = false;

		void Mouse(double deltaX, double deltaY);

	public:
		Camera();
		~Camera();

		void Create(const CameraConfig& cameraConfig);

		const CameraConfig& GetConfig() const;
		const float GetAspect() const;

		const point3D& GetPosition() const;
		const point3D& GetDirection() const;
		const point3D& GetAngles() const;
		const point3D& GetRight() const;
		const point3D& GetUp() const;

		const mat4& GetView() const;
		const mat4& GetProjection() const;

		void UpdateView();
		void UpdateProjection();

		void Move(const point3D& translation);
		void Rotate(const point3D& rotation);

		void Frame();
};