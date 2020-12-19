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
	//Camera reference values - used to initialise and cap values.
	const glm::vec3 POS = glm::vec3(0.0f, 1.5f, 10.0f);		//!< Camera's initial position.
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
	glm::vec3 m_position;		//!< Instance's world position.
	glm::vec3 m_viewUp;			//!< Instance's local relative upwards direction
	glm::vec3 m_forward;		//!< Instance's local relative forwards direction.
	glm::vec3 m_side;			//!< Instance's local relative side direction.

	//Camera's Rotation Property Values:
	GLfloat m_yawAngle;			//!< Instance's world yaw rotation.
	GLfloat m_pitchAngle;		//!< Instance's world pitch rotation.

	//Camera's Movement Property Values:
	GLfloat m_moveSpeed;		//!< Instance's panning movement speed.
	GLfloat m_mouseSensitivity; //!< Instance's rotation movement speed.
	GLfloat m_zoomSensitivity;  //!< Instance's zoom movement speed.
	GLfloat m_mouseZoom;		//!< Instnace's maximum outwards zoom distance.

	//Private Camera Functions
	void normalizeAngle();		//!< Constrains rotation back within its allowed ranges for pitch and yaw.
	void updateCameraVectors();	//!< Updates the vectors for the camera's relative "forward" and "side" after a rotation.
public:
	Camera();					//!< Default camera constructor which doesn't take parameters so just sets all the instance's values to their defaults.
	Camera(glm::vec3 pos);		//!< Default camera constructor which only takes the position as a parameter and sets the instance's remaining values to their defaults.
	Camera(glm::vec3 pos, glm::vec3 up, GLfloat yaw, GLfloat pitch); 	//!< Camera constructor which allows the camera's values to be set by the user with the parameters, or their default values.

	//! Returns the camera's zoom value.
	const inline float getZoom() { return m_mouseZoom; };
	//! Returns the camera's zoom value.
	const inline glm::vec3 getPosition() { return m_position; };
	//! Returns the camera's forward view.
	const inline glm::mat4 getViewMatrix() { return glm::lookAt(m_position, m_position + m_forward, m_viewUp); };

	void handleKeyPress(Camera_Movement direction, GLfloat deltaTime); //!< Defines how the camera should respond to specified input movement enumarators.
	void handleMouseMove(GLfloat xoffset, GLfloat yoffset);			   //!< Defines how the camera should respond to mouse movement input.
	void handleMouseScroll(GLfloat yoffset);						   //!< Defines how the camera should respond to the mouse wheel being scrolled.
	void reset();													   //!< Sets the camera's position/view values to their defaults.
};
#endif