#pragma once

#include "point.hpp"
#include "matrix.hpp"

#include <iostream>

/**
 * @file camera.hpp
 * @brief Camera configuration and utility class for 3D rendering.
 *
 * @details
 * Provides a configuration struct and a Camera class that manages
 * view and projection matrices, position/orientation, and input-driven
 * updates for real-time rendering.
 */

/** @brief Configuration parameters for a Camera. */
struct CameraConfig
{
	uint32_t width = 400; /**< @brief Viewport width in pixels. */
	uint32_t height = 400; /**< @brief Viewport height in pixels. */
	float near = 0.01; /**< @brief Near clipping plane distance. */
	float far = 10000; /**< @brief Far clipping plane distance. */
	float fov = 60; /**< @brief Field of view (in degrees). */
	float speed = 3; /**< @brief Movement speed (units per second). */
	float sensitivity = 0.1; /**< @brief Mouse sensitivity for rotations. */
};

/**
 * @brief A 3D camera that manages view and projection matrices.
 *
 * @details
 * Encapsulates camera configuration, position/orientation state,
 * and provides methods to update matrices in response to input
 * or window resizing.
 * 
 * Typical usage:
 * - Construct and call @ref Create() with a @ref CameraConfig.
 * - On each frame, update input (move/rotate) and call @ref UpdateView() / @ref UpdateProjection() as needed.
 * - Use @ref GetView() and @ref GetProjection() for rendering.
 */
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
		void Scroll(double deltaX, double deltaY);

	public:
		Camera(); /**< @brief Constructs a camera with default configuration. */
		~Camera(); /**< @brief Destroys the camera. */

		/**
		 * @brief Initializes the camera with a configuration.
		 * @param cameraConfig Camera parameters (viewport, clipping planes, FOV, etc.).
		 */
		void Create(const CameraConfig& cameraConfig);

		/**
		 * @brief Gets the current configuration.
		 * @return Const reference to the active CameraConfig.
		 */
		const CameraConfig& GetConfig() const;

		void SetConfig(const CameraConfig& newConfig);

		/**
		 * @brief Gets the aspect ratio of the viewport.
		 * @return Aspect ratio (width / height).
		 */
		const float GetAspect() const;

		const point3D& GetPosition() const; /**< @brief Gets the camera position. */
		const point3D& GetDirection() const; /**< @brief Gets the forward direction vector. */
		const point3D& GetAngles() const; /**< @brief Gets the Euler angles (pitch, yaw, roll). */
		const point3D& GetRight() const; /**< @brief Gets the right vector. */
		const point3D& GetUp() const; /**< @brief Gets the up vector. */

		/**
		 * @brief Gets the current view matrix.
		 * @return Const reference to the view matrix.
		 */
		const mat4& GetView() const;

		/**
		 * @brief Gets the current projection matrix.
		 * @return Const reference to the projection matrix.
		 */
		const mat4& GetProjection() const;


		void UpdateView(); /**< @brief Updates the view matrix from the current position/orientation. */
		void UpdateProjection(); /**< @brief Updates the projection matrix from the current config (FOV, aspect, near/far). */

		/**
		 * @brief This will move the Camera in world space.
		 * @param translation The amount to move.
		 */
		void Move(const point3D& translation);

		/**
		 * @brief Rotates the camera.
		 * @param rotation Euler rotation vector (pitch, yaw, roll increments).
		 */
		void Rotate(const point3D& rotation);

		void Frame();

		void Resize(size_t width, size_t height);
};