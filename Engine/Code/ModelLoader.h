#pragma once
#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <glad/glad.h>

class App;

#define PushData(buffer, data, size) PushAlignedData(buffer, data, size, 1)
#define PushUInt(buffer, value) { u32 v = value; PushAlignedData(buffer, &v, sizeof(v), 4); }
#define PushVec3(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushVec4(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushMat3(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushMat4(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))

struct VertexBufferAttribute
{
	u8 location;
	u8 componentCount;
	u8 offset;
};
struct VertexBufferLayout
{
	std::vector<VertexBufferAttribute> attributes;
	u8 stride;
};
struct VertexShaderAttribute
{
	u8 location;
	u8 component;
};
struct  VertexShaderLayout
{
	std::vector<VertexShaderAttribute> attributes;
};
struct  Model
{
	u32 meshIdx;
	std::vector<u32> materialIdx;
};

class Entity
{
public:
	std::string name;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::mat4 worldMatrix;
	glm::mat4 worldMatrixProjection;

	u32 modelIndex;
	u32 localParamsOffset;
	u32 localParamSize;
	std::vector<u32> materialIdx;

	glm::mat4 TransformScale(const glm::vec3& scaleFactors);
	glm::mat4 TransformPositionScale(const glm::vec3& pos, const glm::vec3& scaleFactors);

	Entity GetModelFromName(std::string name, App* app);
};

enum LightType
{
	DIRECTIONAL,
	POINT_LIGHT
};

class Light
{
public:
	Light() {}
	Light(LightType lightType, const glm::vec3& _position, const glm::vec3& _color, const float _intensity, const glm::vec3 d)
	{
		
			switch (lightType)
			{
				case LightType::DIRECTIONAL:
				{
					direction = glm::normalize(_position);
					color = _color;
					name = "Directional Light";
					type = lightType;
					intesity = _intensity;
					{

					};
					break;
				}
				case LightType::POINT_LIGHT:
				{
					name = "Point Light";
					direction = d;
					this->position = position;
					intesity = _intensity;
					type = lightType;
					break;
				}
			}
		
	}
	virtual ~Light()
	{

	}

public:
	std::string name;
	LightType type;
	glm::vec3 color;
	glm::vec3 direction;
	glm::vec3 position;
	float intesity;
};

struct Vao
{
	GLuint handle;
	GLuint programHandle;
};
struct Submesh
{
	VertexBufferLayout vertexBufferLayout;
	std::vector<float> vertices;
	std::vector<u32> indices;
	u32 vertexOffset;
	u32 indexOffset;

	std::vector<Vao> vaos;
};
struct Material
{
	std::string name;
	glm::vec3 albedo;
	glm::vec3 emissive;
	f32 smoothness;
	u32 albedoTextureIdx;
	u32 emissiveTextureIdx;
	u32 specularTextureIdx;
	u32 normalsTextureIdx;
	u32 bumpTextureIdx;
};
struct Mesh
{
	std::vector<Submesh> submeshes;
	GLuint vertexBufferHandle;
	GLuint indexBufferHandle;
};

u32 LoadModel(App* app, const char* filename, std::string name,glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

#endif // MODEL_LOADER_H