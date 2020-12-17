#ifndef _CAMERA_H_
#define _CAMERA_H_

/**
\file camera.h
*/

#include <iostream>
#include <fstream>

#include <GLEW/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iomanip>    

/**
\enum Camera_Movement
\brief Publically stores names for the allowed camera movement types. Used for camera API in other files.
*/
enum Camera_Movement {
	UP,
	DOWN,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};


/**
\class Camera
\brief An OpenGL camera object used to view the rendered scenes with dynamic movement.
*/
class Camera
{
private:
	//Camera's reference values - used to initialise and cap values.
	const glm::vec3 POS = glm::vec3(0.5f, 2.0f, 10.0f);		//!< Camera's initial position.
	const glm::vec3 VIEWUP = glm::vec3(0.0f, 1.0f, 0.0f);	//!< Camera's initial upwards view direction.
	const GLfloat YAW = 0.0f;								//!< Camera's initial rotation around the z-axis.
	const GLfloat PITCH = 0.0f;								//!< Camera's initial rotation around the x-axis.
	const GLfloat MOVE_SPEED = 5.0f;						//!< Default speed of camera movement from keypress.
	const GLfloat MOUSE_SENSITIVTY = 0.1f;					//!< Default speed of camera movement from mouse movement.
	const GLfloat ZOOM_SENSITIVTY = 2.0f;					//!< Default speed of camera zooming from mouse scrolling.
	const GLfloat MOUSE_ZOOM_MIN = 1.0f;					//!< Maximum zoom in.
	const GLfloat MOUSE_ZOOM_MAX = 80.0f;					//!< Maximum zoom out.
	const float PITCH_ANGLE_MAX = 89.0f;					//!< Maximum pitch rotation.

	//Camera's Vector Property Values:
	glm::vec3 m_forward;		//!< Instance's local relative forwards direction.
	glm::vec3 m_side;			//!< Instance's local relative side direction.
	glm::vec3 m_viewUp;			//!< Instance's local relative upwards direction
	glm::vec3 m_position;		//!< Instance's world position.

	//Camera's Rotation Property Values:
	GLfloat m_yawAngle;			//!< Instance's world yaw rotation.
	GLfloat m_pitchAngle;		//!< Instance's world pitch rotation.

	//Camera's Movement Property Values:
	GLfloat m_moveSpeed;		//!< Instance's panning movement speed.
	GLfloat m_mouseSensitivity; //!< Instance's rotation movement speed.
	GLfloat m_zoomSensitivity;  //!< Instance's zoom movement speed.
	GLfloat m_mouseZoom;		//!< Instnace's maximum outwards zoom distance.

	//! Constrains rotation back within its allowed ranges for pitch and yaw.
	void normalizeAngle()
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

	//! Updates the vectors for the camera's relative "forward" and "side" after a rotation.
	void updateCameraVectors()
	{
		//Calculate new forward direction from current rotation.
		glm::vec3 newForward;
		newForward.x = sin(glm::radians(m_yawAngle)) * cos(glm::radians(m_pitchAngle));
		newForward.y = sin(glm::radians(m_pitchAngle));
		newForward.z = -cos(glm::radians(m_yawAngle)) * cos(glm::radians(m_pitchAngle));
			//-cos(glm::radians(m_yawAngle)) * cos(glm::radians(m_pitchAngle));
		//Set newly calculated forward as the actual value.
		m_forward = glm::normalize(newForward);

		//Calculate new side direction from current rotation.
		glm::vec3 newSide;
		newSide.x = -cos(glm::radians(m_yawAngle));
		newSide.y = 0;
		newSide.z = sin(glm::radians(m_yawAngle));

		//Set newly calculated side as the actual value.
		m_side = glm::normalize(glm::cross(m_forward, VIEWUP));

		m_viewUp = glm::normalize(glm::cross(m_side, m_forward));
		//std::cout << "X: " << m_forward.x << " Y: " << m_forward.y << " Z: " << m_forward.z << std::endl;
	}
public:
	//! Default camera constructor which doesn't take parameters so just sets the instance's values to their defaults.
	Camera() : m_position(POS), m_viewUp(VIEWUP),  m_yawAngle(YAW), m_pitchAngle(PITCH), m_moveSpeed(MOVE_SPEED), m_mouseZoom(MOUSE_ZOOM_MAX/2), m_mouseSensitivity(MOUSE_SENSITIVTY), m_zoomSensitivity(ZOOM_SENSITIVTY)
	{
		updateCameraVectors();
	};

	//! Default camera constructor which only takes the position as a parameter and just sets the instance's values to their defaults.
	Camera(glm::vec3 pos) : m_position(pos), m_viewUp(VIEWUP), m_yawAngle(YAW), m_pitchAngle(PITCH), m_moveSpeed(MOVE_SPEED), m_mouseZoom(MOUSE_ZOOM_MAX/2), m_mouseSensitivity(MOUSE_SENSITIVTY), m_zoomSensitivity(ZOOM_SENSITIVTY)
	{
		updateCameraVectors();
	};

	//! Camera constructor which allows the camera's values to be set by the user with the parameters, or their default values.
	/*!
	\param pos What to initialise the camera's position as.
	\param up What to initialise the camera's upwards reference as.
	\param yaw What to initialise the camera's yaw rotation as.
	\param pitch What to initialise the camera's pitch rotation as.
	*/
	Camera(glm::vec3 pos, glm::vec3 up, GLfloat yaw, GLfloat pitch) : m_position(pos), m_viewUp(up), m_yawAngle(yaw), m_pitchAngle(pitch), m_moveSpeed(MOVE_SPEED), m_mouseZoom(MOUSE_ZOOM_MAX/2), m_mouseSensitivity(MOUSE_SENSITIVTY), m_zoomSensitivity(ZOOM_SENSITIVTY)
	{
		updateCameraVectors();
	};

	//! Returns the camera's zoom value.
	float getZoom() { return m_mouseZoom; };
	//! Returns the camera's zoom value.
	glm::vec3 getPosition() { return m_position; };
	//! Returns the camera's forward view.
	glm::mat4 getViewMatrix() { return glm::lookAt(m_position, m_position + m_forward, m_viewUp); };	

	//! Defines how the camera should respond to specified input movement enumarators.
	/*!
	\param direction The associated direction of key input. (Which enumator was used for this input).
	\param deltaTime The application's current timestep.
	*/
	void handleKeyPress(Camera_Movement direction, GLfloat deltaTime)
	{
		//Calculate movement speed in context of the window update.
		GLfloat velocity = m_moveSpeed * deltaTime;

		//Move camera's position on the axis in the specified direction.
		switch (direction)
		{
		case UP:
			m_position += m_viewUp * velocity;
			break;
		case DOWN:
			m_position -= m_viewUp * velocity;
			break;
		case FORWARD:
			m_position += m_forward * velocity;
			break;
		case BACKWARD:
			m_position -= m_forward * velocity;
			break;
		case LEFT:
			m_position -= m_side * velocity;
			break;
		case RIGHT:
			m_position += m_side * velocity;
			break;
		default:
			break;
		}

		std::cout << "Position... X: " << m_position.x <<  " Y: " << m_position.y << " Z: " << m_position.z << std::endl;
		std::cout << "Yaw...: " << m_yawAngle << std::endl;
		std::cout << "Pitch...: " << m_pitchAngle << std::endl;
		std::cout << "Up... X: " << m_viewUp.x << " Y: " << m_viewUp.y << " Z: " << m_viewUp.z << std::endl;
		std::cout << "Forward... X: " << m_forward.x << " Y: " << m_forward.y << " Z: " << m_forward.z << std::endl << std::endl;
		
		
	}

	//! Defines how the camera should respond to mouse movement input.
	/*!
	\param xoffset Amount the mouse has moved since the last update on the x-axis.
	\param yoffset Amount the mouse has moved since the last update on the y-axis.
	*/
	void handleMouseMove(GLfloat xoffset, GLfloat yoffset)
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

	//! Defines how the camera should respond to the mouse wheel being scrolled.
	/*!
	\param yoffset Amount the scroll wheel has moved since the last update on the y-axis.
	*/
	void handleMouseScroll(GLfloat yoffset)
	{
		float zoom = m_mouseZoom - (ZOOM_SENSITIVTY * yoffset);

		//If the camera would not be taken outside its zoom range from doing so, zoom the camera to the amount scrolled.
		if (zoom < MOUSE_ZOOM_MIN) 
		{
			m_mouseZoom = MOUSE_ZOOM_MIN;
		}
		else if (zoom > MOUSE_ZOOM_MAX)
		{
			m_mouseZoom = MOUSE_ZOOM_MAX;
		}
		else 
		{
			m_mouseZoom = zoom;
		}

		//updateCameraVectors();

		std::cout << "Position... X: " << m_position.x << " Y: " << m_position.y << " Z: " << m_position.z << std::endl;
		std::cout << "Yaw...: " << m_yawAngle << std::endl;
		std::cout << "Pitch...: " << m_pitchAngle << std::endl;
		std::cout << "Up... X: " << m_viewUp.x << " Y: " << m_viewUp.y << " Z: " << m_viewUp.z << std::endl;
		std::cout << "Forward... X: " << m_forward.x << " Y: " << m_forward.y << " Z: " << m_forward.z << std::endl << std::endl;

		std::cout << "Get Zoom...: " << getZoom() << std::endl << std::endl;

	}

	//! Sets the camera's position/view values to their defaults.
	void reset()
	{
		m_position = POS;
		m_mouseZoom = MOUSE_ZOOM_MAX/2;
		m_pitchAngle = PITCH;
		m_yawAngle = YAW;
		m_viewUp = VIEWUP;

		updateCameraVectors();
	}
};
#endif