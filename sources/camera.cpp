#include "camera.hpp"

#include "input.hpp"
#include "time.hpp"

Camera::Camera()
{

}

Camera::~Camera()
{

}



void Camera::Create(const CameraConfig& cameraConfig)
{
	config = cameraConfig;

	updated = false;

	UpdateView();
	UpdateProjection();

	Input::RegisterMouseCallback([&](double xpos, double ypos) { Mouse(xpos, ypos); });
}

const CameraConfig& Camera::GetConfig() const
{
	return (config);
}

const float Camera::GetAspect() const
{
	return (static_cast<float>(config.width) / static_cast<float>(config.height));
}

const point3D& Camera::GetPosition() const
{
	return (position);
}

const point3D& Camera::GetDirection() const
{
	return (direction);
}

const point3D& Camera::GetAngles() const
{
	return (angles);
}

const point3D& Camera::GetRight() const
{
	return (right);
}

const point3D& Camera::GetUp() const
{
	return (up);
}

const mat4& Camera::GetView() const
{
	return (view);
}

const mat4& Camera::GetProjection() const
{
	return (projection);
}

void Camera::UpdateView()
{
	if (updated) return;

	//view = mat4::View(angles, position);
	view = mat4::Look(position, position + direction, point3D(0, 1, 0));

	updated = true;
}

void Camera::UpdateProjection()
{
	projection = mat4::Projection(config.fov, GetAspect(), config.near, config.far);
}

void Camera::Move(const point3D& translation)
{
	position += translation;

	updated = false;
}

void Camera::Rotate(const point3D& rotation)
{
	angles += rotation;

	if (angles.x() > 89) angles.x() = 89;
	if (angles.x() < -89) angles.x() = -89;

	direction = point3D(0, 0, 1);
	direction.Rotate(angles);
	direction.Unitize();

	right = point3D::Cross(point3D(0, 1, 0), direction).Unitized();
	up = point3D::Cross(direction, right);

	updated = false;
}

void Camera::Mouse(double deltaX, double deltaY)
{
	Rotate(point3D(deltaY * config.sensitivity, deltaX * config.sensitivity));
}

void Camera::Frame()
{
	if (Input::GetKey(GLFW_KEY_W).down) Move(direction * Time::deltaTime * config.speed);
	if (Input::GetKey(GLFW_KEY_S).down) Move(direction * Time::deltaTime * -config.speed);
	if (Input::GetKey(GLFW_KEY_D).down) Move(right * Time::deltaTime * config.speed);
	if (Input::GetKey(GLFW_KEY_A).down) Move(right * Time::deltaTime * -config.speed);
	if (Input::GetKey(GLFW_KEY_SPACE).down) Move(up * Time::deltaTime * config.speed);
	if (Input::GetKey(GLFW_KEY_LEFT_CONTROL).down) Move(up * Time::deltaTime * -config.speed);
}