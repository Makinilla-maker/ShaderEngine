#include "Global.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
//#include "engine.h"


Camera::Camera()
{
	zNear = 0.1;
	zFar = 1000.0f;
	projection = glm::perspective(glm::radians(60.0f),aspectRatio,zNear,zFar);
	
	//Camera Position
	cameraPos = glm::vec3(-10.0f, 5.0f, -0.0f);
	//Camera Direction
	cameraTarget = glm::vec3(0.0f, 2.0f, 0.0f);
	cameraDirection = glm::normalize(cameraPos - cameraTarget);
	//Up axis' camera
	cameraUp = glm::vec3(0,1,0);
	//Right axis' camera
	cameraRight = glm::normalize(glm::cross({ 0,1,0 }, cameraDirection));

	cameraForward = glm::normalize(glm::cross(cameraUp, cameraRight));
	view = glm::mat4(1);
}

Camera::~Camera()
{
}
void Camera::Update(glm::vec2 displaySize, App* app)
{
	
	if (app->input.mouseButtons[MouseButton::RIGHT] == BUTTON_PRESSED)
	{
		float speed = 20.0f;

		if (app->input.keys[Key::K_W] == ButtonState::BUTTON_PRESSED)
		{
			cameraPos -= cameraForward * app->deltaTime * speed;
		}
		if (app->input.keys[Key::K_S] == ButtonState::BUTTON_PRESSED)
		{
			cameraPos += cameraForward * app->deltaTime * speed;
		}
		if (app->input.keys[Key::K_A] == ButtonState::BUTTON_PRESSED)
		{
			cameraPos -= cameraRight * app->deltaTime * speed;
		}
		if (app->input.keys[Key::K_D] == ButtonState::BUTTON_PRESSED)
		{
			cameraPos += cameraRight * app->deltaTime * speed;
		}
	}

	aspectRatio = (float)displaySize.x / (float)displaySize.y;
	
	projection = glm::perspective(glm::radians(60.0f), aspectRatio, zNear, zFar);

	RecalculateCamera();
}
void Camera::RecalculateCamera()
{
	cameraDirection = glm::normalize(cameraPos - cameraTarget);
	cameraRight = glm::normalize(glm::cross({ 0,1,0 }, cameraDirection));

	cameraUp = glm::cross(cameraDirection, cameraRight);
	cameraForward = glm::normalize(glm::cross(cameraUp, cameraRight));

	view = glm::lookAt(cameraPos,cameraTarget,cameraUp);

}

