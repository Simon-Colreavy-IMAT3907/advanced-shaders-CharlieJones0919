/**
\file application.h
\mainpage Shaders Assignment Doxygen Report - P2423910  
*<EM> This program is an attempt to implement the shaders techniques of normal mapping, parallax/height mapping and model loading. </EM>
*<br> <br> 
*<B> Controls are as follows... </B> <br>
*	Mouse: Camera Rotation (e.g. Move up to rotate up, etc..) <br>
*	Scroll Wheel: Camera Zoom <br>
*<br>
*	W Key: Pan CameraUpwards <br>
*	A Key: Pan Camera Left <br>
*	S Key: Pan Camera Downwards <br>
*	D Key: Pan Camera Right <br>
*<br>
*	W + Left Shift Key: Move Camera Forwards <br>
*	S + Left Shift Key: Move Camera Backwards <br>
*<br>
*	P Key: Toggle Parallax Mapping On/Off <br>
*	N Key: Toggle Normal Mapping On/Off <br>
*<br>
*	R Key: Reset Camera <br>
*	Space Key: Stop Model Rotation <br>
*/
#define GLEW_STATIC
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

//Required Class' Header Files
#include "../../include/independent/shader.h"
#include "../../include/independent/camera.h"
#include "../../include/independent/texture.h"
#include "../../include/independent/model.h"

//Viewing Variables
Camera camera = Camera();
const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600; //!< The OpenGL window's width and height dimensions.

//Callback Functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods); //!< Calls correlating function[s] for according keyboard input.
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);				//!< Calls correlating function[s] for according mouse movement input.
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);		//!< Calls correlating function[s] for according scroll wheel input.

//User Input Variables
bool firstMouseMove = true;	  //!< Used to initialise the mouse movement's last position if there hasn't been a last position input.
GLfloat lastX;				  //!< The mouse/cursor's last position on screen's x-axis.
GLfloat lastY;				  //!< The mouse/cursor's last position on screen's y-axis.
							  
//Window Loop Variables		  
GLfloat deltaTime = 0.0f;	  //!< The window's current frame update.
GLfloat lastFrame = 0.0f;	  //!< The last frame since update.

//Shader Variables
glm::vec3 lightSrcPosition(0.25f, 1.75f, 2.0f); //!< Position of the lights.
bool bNormalMapping = true;   //!< Whether or not the model is being rendered with its normal map.
bool bParallaxMapping = true; //!< Whether or not the model is being rendered with parallax mapping.
bool bRotate = true;		  //!< Whether or not the model should rotate.
GLfloat fHeightScale = 0.1f;  //!< Parallax's height mapping height.
Model objectModel;			  //!< The model to be rendered.

//! A function to utalise the other classes to render a scene of model[s] on a loop while facilitating user input.
int main()
{
	//Check that GLFW could initialise.
	if (!glfwInit()) std::cout << "Error::GLFW could not initialize GLFW!" << std::endl;

	//Initialise GLFW version and properties.
	std::cout << "Start OpenGL core profile version 3.3" << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//Create window.
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "P2423910 Shaders Assignment", NULL, NULL);
	if (!window) std::cout << "Error::GLFW could not create winddow!" << std::endl; //Check window was created successfully.

	//Set GLFW's context the window and set its callbacks to functions.
	glfwMakeContextCurrent(window); //Set window.

	//Set callbacks to according functions.
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);

	//Disable cursor from view.
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Set glewExperimental to true so glewInit can be used. It obtains required information about supported extensions from the graphics driver.
	glewExperimental = GL_TRUE; 
	GLenum status = glewInit(); //Initialise GLEW.
	if (status != GLEW_OK) std::cout << "Error::GLEW glew version:" << glewGetString(GLEW_VERSION) << " error string:" << glewGetErrorString(status) << std::endl; //Check that GLEW was initialised successfully.

	//Set OpenGL viewport.
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	//Get model path from modelPath.txt file.
	std::ifstream modelPath("modelPath.txt");
	if (!modelPath) std::cout << "Error::could not read model path file." << std::endl; //Check model path was successfully loaded.

	//Load full file path from modelPath.
	std::string modelFilePath;
	std::getline(modelPath, modelFilePath);
	if (!objectModel.loadModel(modelFilePath)) std::cout << "Error::could not load model from file path." << std::endl; //Check model was successfully loaded.
	//Load shaders.
	Shader shader("resources/shaders/scene.vertex", "resources/shaders/scene.frag");

	//Enable depth test for 3D geometry.
	glEnable(GL_DEPTH_TEST);
	//Enable alpha transparancy in RGBA.
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Set window's clear colour to blue.
	glClearColor(0.0f, 0.5f, 0.75f, 1.0f);

	//Start application loop to run while the window hasn't been closed.
	while (!glfwWindowShouldClose(window))
	{
		//Clear window buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Set frame update data to new values.
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Check for input.
		glfwPollEvents(); 

		//Get current projection/zoom and view from camera.
		glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (GLfloat)(WINDOW_WIDTH / WINDOW_HEIGHT), 1.0f, 100.0f);
		glm::mat4 view = camera.getViewMatrix(); 

		//Use the shader set to shader.
		shader.use();

		//Get light uniform data locations from shader.
		GLint lightAmbientLoc = glGetUniformLocation(shader.programId, "light.ambient");
		GLint lightDiffuseLoc = glGetUniformLocation(shader.programId, "light.diffuse");
		GLint lightSpecularLoc = glGetUniformLocation(shader.programId, "light.specular");
		GLint lightPosLoc = glGetUniformLocation(shader.programId, "light.position");
		//Set light uniform data to the shader program's uniforms.
		glUniform3f(lightAmbientLoc, 0.3f, 0.3f, 0.3f);
		glUniform3f(lightDiffuseLoc, 0.6f, 0.6f, 0.6f);
		glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPosLoc, lightSrcPosition.x, lightSrcPosition.y, lightSrcPosition.z);
		
		//Get camera's uniform data location from shader.
		GLint viewPosLoc = glGetUniformLocation(shader.programId, "viewPos");
		glUniform3f(viewPosLoc, camera.getPosition().x, camera.getPosition().y, camera.getPosition().z); //Set camera uniform data to the shader program's uniforms.
		
		//Get light positions' uniform's location.
		lightPosLoc = glGetUniformLocation(shader.programId, "lightPos");
		glUniform3f(lightPosLoc, lightSrcPosition.x, lightSrcPosition.y, lightSrcPosition.z); //Set light position uniform.
		//Get and set data to camera projection and views' uniform locations.
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "projection"), 1, GL_FALSE, glm::value_ptr(projection)); //Set camera projection uniform.
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "view"), 1, GL_FALSE, glm::value_ptr(view)); //Set camera view uniform.
		
		glm::mat4 model; //Model to render.
		if (bRotate) model = glm::rotate(model, currentFrame -2, glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f))); 	//Rotate the model.

		//Get and set data to the model and normal mappings' uniform locations.
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"), 1, GL_FALSE, glm::value_ptr(model));
		//glUniform1i(glGetUniformLocation(shader.programId, "bParallaxMapping"), bParallaxMapping);
		//glUniform1f(glGetUniformLocation(shader.programId, "heightScale"), heightScale);
		glUniform1i(glGetUniformLocation(shader.programId, "normalMapping"), bNormalMapping);
		glUniform1i(glGetUniformLocation(shader.programId, "parallaxMapping"), bParallaxMapping);
		glUniform1f(glGetUniformLocation(shader.programId, "heightScale"), fHeightScale);

		//Draw the model.
		objectModel.draw(shader);

		//Bind Vertex Array.
		glBindVertexArray(0);
		//Set shader program.
		glUseProgram(0);

		//Swap window's buffers.
		glfwSwapBuffers(window);
	}

	//Termintate GLFW when window closes.
	glfwTerminate();
	return 0;
}

//! The function to be called when the GLFW window detects keyboard input. Calls functions for assigned keys.
/*!
\param window The current GLFW window that was polled for this input.
\param key Code of the interacted with key.
\param scancode The key's platform specific code.
\param action Kind of interaction performed on this key.
\param mods Any modifiyer keys that are down. (e.g. Shift).
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case(GLFW_PRESS): //Check for key presses.
		switch (key)
		{
		case(GLFW_KEY_ESCAPE):
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case(GLFW_KEY_N):
			bNormalMapping = !bNormalMapping;
			std::cout << "Using Normal Mapping " << (bNormalMapping ? "True" : "False") << std::endl;
			break;
		case(GLFW_KEY_P):
			bParallaxMapping = !bParallaxMapping;
			std::cout << "Using Parallax Mapping " << (bParallaxMapping ? "True" : "False") << std::endl;
			break;
		case(GLFW_KEY_SPACE):
			bRotate = !bRotate;
			std::cout << "Rotating " << (bRotate ? "True" : "False") << std::endl;
			break;
		case(GLFW_KEY_R):
			camera.reset();
			break;
		}
		break;
	}

	//Check for movement key input.
	switch (key)
	{
	case(GLFW_KEY_W): //Callback is dependant on if shift key is down.
		switch (mods) {
			case(1): camera.handleKeyPress(FORWARD, deltaTime); break;
			default: camera.handleKeyPress(UP, deltaTime); break;
		} break;
	case(GLFW_KEY_A):
		camera.handleKeyPress(LEFT, deltaTime);
		break;
	case(GLFW_KEY_S): //Callback is dependant on if shift key is down.
		switch (mods) {
			case(1): camera.handleKeyPress(BACKWARD, deltaTime); break;
			default: camera.handleKeyPress(DOWN, deltaTime); break;
		} break;
	case(GLFW_KEY_D):
		camera.handleKeyPress(RIGHT, deltaTime);
		break;
	}
}

//! The function to be called when the GLFW window detects mouse movement. Rotates the camera.
/*!
\param window The current GLFW window that was polled for this input.
\param xpos The new screen position of the cursor on the x-axis.
\param xpos The new screen position of the cursor on the y-axis.
*/
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	//Set last offsets to the current offsets if this is the first call of this function.
	if (firstMouseMove) 
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	//Calculate the amount moved from the previous position for the offset.
	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	//Cache the cursor's screen position for the next movement call.
	lastX = xpos;
	lastY = ypos;

	//Rotate the camera to the degree/direction of the offset.
	camera.handleMouseMove(xoffset, yoffset);
}

//! The function to be called when the GLFW window detects scrolling. Zooms the camera.
/*!
\param window The current GLFW window that was polled for this input.
\param xoffset The amount scrolled on the x-axis.
\param yoffset The amount scrolled on the y-axis.
*/
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.handleMouseScroll(yoffset);
}