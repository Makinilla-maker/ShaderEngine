#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "platform.h"
#include <glad/glad.h>


class App;

class Camera
{
public:
	Camera();
	~Camera();
	void Update(App* app);
	
	float aspectRatio;

	float zNear;
	float zFar;
	glm::mat4 projection;

	glm::vec3 cameraPos;

	glm::vec3 cameraTarget;
	glm::vec3 cameraDirection;

	glm::vec3 up;
	glm::vec3 cameraRight;

	glm::vec3 cameraUp;
	glm::mat4 view;

	

private:

};



#endif // CAMERA_H