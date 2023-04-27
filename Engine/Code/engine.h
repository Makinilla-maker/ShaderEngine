//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>
#include "ModelLoader.h"
#include "Camera.h"

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct OpenGlInfo
{
    std::string glVersion;
    std::string glRender;
    std::string glVendor;
    std::string glShadingVersion;

    std::vector<std::string> glExtension;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?
    VertexShaderLayout vertexInputLayout;
    GLsizei lenght;
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_Count
};

struct VertexV2V3
{
    glm::vec3 pos;
    glm::vec2 uv;
};

const VertexV2V3 vertices[] = {
    {glm::vec3(-0.5,-0.5,0.0),glm::vec2(0.0,0.0)},
    {glm::vec3(0.5,-0.5,0.0),glm::vec2(1.0,0.0)},
    {glm::vec3(0.5,0.5,0.0),glm::vec2(1.0,1.0)},
    {glm::vec3(-0.5,0.5,0.0),glm::vec2(0.0,1.0)},

};
const u16 indices[] =
{
    0,1,2,
    0,2,3
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize; //Alguns shaders

    std::vector<Texture>  textures; //Textures loaded
    std::vector<Program>  programs; //programms loaded
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Model> models;

    // program indices
    u32 texturedMeshProgramIdx; //Textures indefinides
    
    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    u32 modelPatrick;
    u8 texturedMeshProgram_uTexture;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    OpenGlInfo glInfo;

    Camera camera;

 };

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

u32 LoadTexture2D(App* app, const char* filepath);

