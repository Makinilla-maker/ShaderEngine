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
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraTarget2 = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraDirection = glm::normalize(cameraPos - cameraTarget);
	//Up axis' camera
	cameraUp = glm::vec3(0,1,0);
	
	//cameraForward = glm::normalize(glm::cross(cameraUp, cameraRight));
	cameraRight = glm::normalize(glm::cross({ 0,1,0 }, cameraDirection));
	cameraForward = glm::normalize(glm::cross({ 0,1,0 }, cameraRight));

	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	//Right axis' camera

	view = glm::mat4(1);
}

Camera::~Camera()
{
}
void Camera::Update(glm::vec2 displaySize, App* app)
{
	//if (app->input.mouseButtons[MouseButton::RIGHT] == BUTTON_PRESSED)
	{
		float speed = 20.0f;

		if (app->input.keys[Key::K_W] == ButtonState::BUTTON_PRESSED) cameraPos += cameraForward * app->deltaTime * speed;
		if (app->input.keys[Key::K_S] == ButtonState::BUTTON_PRESSED) cameraPos -= cameraForward * app->deltaTime * speed;
		if (app->input.keys[Key::K_A] == ButtonState::BUTTON_PRESSED)
		{
			//??why rotate
			cameraPos -= cameraRight * app->deltaTime * speed;
		}
		if (app->input.keys[Key::K_D] == ButtonState::BUTTON_PRESSED)
		{
			cameraPos += cameraRight * app->deltaTime * speed;
		}
		
		if (app->input.keys[Key::K_Q] == ButtonState::BUTTON_PRESSED) cameraPos += cameraUp * app->deltaTime * speed;
		if (app->input.keys[Key::K_E] == ButtonState::BUTTON_PRESSED) cameraPos -= cameraUp * app->deltaTime * speed;
	}
	if (app->input.mouseButtons[MouseButton::LEFT] == BUTTON_PRESSED)
	{
		float panSpeed = 0.9f * app->deltaTime;
		glm::vec3 right = glm::normalize(glm::cross(cameraTarget - cameraPos, cameraUp));
		glm::vec3 up = glm::normalize(glm::cross(right, cameraTarget - cameraPos));
		right = cameraRight;
		up = cameraUp;
		cameraPos -= right * app->input.mouseDelta.x * panSpeed * 5.f;
		cameraPos += up * app->input.mouseDelta.y * panSpeed * 5.f;
		//cameraTarget = cameraPos + glm::normalize(cameraForward - cameraPos);
	}
	/*if (app->input.mouseButtons[MouseButton::RIGHT] == BUTTON_PRESSED)
	{
		float speed = 20.0f;
		cameraPos -= cameraRight * app->deltaTime * speed * app->input.mouseDelta.x;
		cameraUp -= cameraRight * app->deltaTime * speed * app->input.mouseDelta.y;
		
	}*/

	RecalculateCamera(displaySize);
}
void Camera::RecalculateCamera(glm::vec2 displaySize)
{
	cameraDirection = glm::normalize(cameraPos - cameraTarget);
	cameraRight = glm::normalize(glm::cross({ 0,1,0 }, cameraDirection));

	cameraUp = glm::cross(cameraDirection, cameraRight);
	cameraForward = glm::normalize(glm::cross(cameraUp, cameraRight));

	aspectRatio = (float)displaySize.x / (float)displaySize.y;
	projection = glm::perspective(glm::radians(60.0f), aspectRatio, zNear, zFar);
	view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
}

