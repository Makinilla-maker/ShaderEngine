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
	//Right axis' camera
	//up = glm::vec3(0.0f, 1.0f, 0.0f);
	//cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	//Up axis' camera
	cameraUp = glm::vec3(0,1,0);
	view = glm::mat4(1);
}

Camera::~Camera()
{
}
void Camera::Update(glm::vec2 displaySize)
{
	aspectRatio = (float)displaySize.x / (float)displaySize.y;
	
	projection = glm::perspective(glm::radians(60.0f), aspectRatio, zNear, zFar);

	static float timer = 0;
	const float radius = 10.0f;
	float camX = sin(timer) * radius;
	float camZ = cos(timer) * radius;
	timer += 10;

	view = glm::lookAt(cameraPos,cameraTarget,cameraUp);
	//view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
}

