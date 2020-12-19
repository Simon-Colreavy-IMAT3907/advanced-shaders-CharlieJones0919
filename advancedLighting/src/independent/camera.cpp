/**
\file camera.cpp
*/
#include "../../include/independent/camera.h"

Camera::Camera() : m_position(POS), m_viewUp(VIEWUP), m_yawAngle(YAW), m_pitchAngle(PITCH), m_moveSpeed(MOVE_SPEED), m_mouseZoom(MOUSE_ZOOM_MAX / 2), m_mouseSensitivity(MOUSE_SENSITIVTY), m_zoomSensitivity(ZOOM_SENSITIVTY)
{
	updateCameraVectors();
}

/*!
\param pos What to initialise the camera's position as.
*/
Camera::Camera(glm::vec3 pos) : m_position(pos), m_viewUp(VIEWUP), m_yawAngle(YAW), m_pitchAngle(PITCH), m_moveSpeed(MOVE_SPEED), m_mouseZoom(MOUSE_ZOOM_MAX / 2), m_mouseSensitivity(MOUSE_SENSITIVTY), m_zoomSensitivity(ZOOM_SENSITIVTY)
{
	updateCameraVectors();
}

/*!
\param pos What to initialise the camera's position as.
\param up What to initialise the camera's upwards reference as.
\param yaw What to initialise the camera's yaw rotation as.
\param pitch What to initialise the camera's pitch rotation as.
*/
Camera::Camera(glm::vec3 pos, glm::vec3 up, GLfloat yaw, GLfloat pitch) : m_position(pos), m_viewUp(up), m_yawAngle(yaw), m_pitchAngle(pitch), m_moveSpeed(MOVE_SPEED), m_mouseZoom(MOUSE_ZOOM_MAX / 2), m_mouseSensitivity(MOUSE_SENSITIVTY), m_zoomSensitivity(ZOOM_SENSITIVTY)
{
	updateCameraVectors();
}

void Camera::normalizeAngle()
{
	//Constrain if over the maximum pitch rotation.
	if (m_pitchAngle > PITCH_ANGLE_MAX)
		m_pitchAngle = PITCH_ANGLE_MAX;
	//Constrain if under the minimum pitch rotation.
	if (m_pitchAngle < -PITCH_ANGLE_MAX)
		m_pitchAngle = -PITCH_ANGLE_MAX;

	//Constrain if under the minimum yaw rotation.
	if (m_yawAngle < 0.0f)
		m_yawAngle += 360.0f;
}

void Camera::updateCameraVectors()
{
	//Calculate new forward direction from current rotation.
	glm::vec3 newForward;
	newForward.x = sin(glm::radians(m_yawAngle)) * cos(glm::radians(m_pitchAngle));
	newForward.y = sin(glm::radians(m_pitchAngle));
	newForward.z = -cos(glm::radians(m_yawAngle)) * cos(glm::radians(m_pitchAngle));
	m_forward = glm::normalize(newForward);

	//Set new side value.
	m_side = glm::normalize(glm::cross(m_forward, VIEWUP));
	//Set new local upward value.
	m_viewUp = glm::normalize(glm::cross(m_side, m_forward));
}

/*!
\param direction The associated direction of key input. (Which enumator was used for this input).
\param deltaTime The application's current timestep.
*/
void Camera::handleKeyPress(Camera_Movement direction, GLfloat deltaTime)
{
	//Calculate movement speed in context of the window update.
	GLfloat velocity = m_moveSpeed * deltaTime;

	//Move camera's position on the axis in the specified direction.
	switch (direction)
	{
	case (UP):
		m_position += m_viewUp * velocity; break;
	case (DOWN):
		m_position -= m_viewUp * velocity; break;
	case (FORWARD):
		m_position += m_forward * velocity; break;
	case (BACKWARD):
		m_position -= m_forward * velocity; break;
	case (LEFT):
		m_position -= m_side * velocity; break;
	case (RIGHT):
		m_position += m_side * velocity; break;
	default: break;
	}
}

/*!
\param xoffset Amount the mouse has moved since the last update on the x-axis.
\param yoffset Amount the mouse has moved since the last update on the y-axis.
*/
void Camera::handleMouseMove(GLfloat xoffset, GLfloat yoffset)
{
	//Calculate rotation degree from amount the mouse has moved in conjunction with this camera's mouse movement speed.  
	float xRotation = xoffset * m_mouseSensitivity;
	float yRotation = yoffset * m_mouseSensitivity;

	//Add rotation to the camera's orientation.
	m_pitchAngle += yRotation;
	m_yawAngle += xRotation;

	//Constrain rotation and update relative direction vectors.
	normalizeAngle();
	updateCameraVectors();
}

/*!
\param yoffset Amount the scroll wheel has moved since the last update on the y-axis.
*/
void Camera::handleMouseScroll(GLfloat yoffset)
{
	float zoom = m_mouseZoom - (ZOOM_SENSITIVTY * yoffset);

	//If the camera would not be taken outside its zoom range from doing so, zoom the camera to the amount scrolled.
	if (zoom < MOUSE_ZOOM_MIN)
		m_mouseZoom = MOUSE_ZOOM_MIN;
	else if (zoom > MOUSE_ZOOM_MAX)
		m_mouseZoom = MOUSE_ZOOM_MAX;
	else
		m_mouseZoom = zoom;
}

void Camera::reset()
{
	//Reset viewing variables back to their defaults.
	m_position = POS;
	m_mouseZoom = MOUSE_ZOOM_MAX / 2;
	m_pitchAngle = PITCH;
	m_yawAngle = YAW;
	m_viewUp = VIEWUP;

	updateCameraVectors();
}