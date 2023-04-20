#pragma once
#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include "platform.h"
#include <glad/glad.h>

class App;

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

u32 LoadModel(App* app, const char* filename);

#endif // MODEL_LOADER_H