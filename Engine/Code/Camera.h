#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>


class App;

//enum CameraMovement {
//	FORWARD,
//	BACKWARD,
//	LEFT,
//	RIGHT
//};

class Camera
{
public:
	Camera();
	~Camera();
	void Update(glm::vec2 displaySize, App* app);
	void RecalculateCamera(glm::vec2 displaySize);
	
	float aspectRatio;

	float zNear;
	float zFar;
	float yaw;
	float pitch;

	glm::mat4 projection;

	glm::vec3 cameraPos;
	glm::vec3 cameraDirection;
	glm::vec3 cameraUp;
	glm::vec3 cameraRight;
	glm::vec3 cameraForward;

	float movementSpeed = 0.5f;

	glm::vec3 cameraTarget;
	glm::vec3 cameraTarget2;


	glm::mat4 view;


private:

};



#endif // CAMERA_H