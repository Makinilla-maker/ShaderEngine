//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include <glad/glad.h>
#include "Entities.h"


typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

class Camera;

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
    FORWARD,
    DEFERRED
};

struct VertexV2V3
{
    glm::vec3 pos;
    glm::vec2 uv;
};

const VertexV2V3 vertices[] = {
    {glm::vec3(-1.0,-1.0,0.0),glm::vec2(0.0,0.0)},
    {glm::vec3(1.0,-1.0,0.0),glm::vec2(1.0,0.0)},
    {glm::vec3(1.0,1.0,0.0),glm::vec2(1.0,1.0)},
    {glm::vec3(-1.0,1.0,0.0),glm::vec2(0.0,1.0)},

};
const u16 indices[] =
{
    0,1,2,
    0,2,3
};
class FrameBuffer
{
public:
    GLuint colorAttachmentHandle = -1;
    GLuint depthAttachmentHandle = -1;
    GLuint frameBufferHandle = -1;
    GLuint frameBufferStatus = -1;
    GLuint normalAttachment = -1;
    GLuint positionAttachment = -1;

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferHandle);
    }

    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void checkStatus()
    {
        frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            switch (frameBufferStatus)
            {
            case GL_FRAMEBUFFER_UNDEFINED:                      ELOG("GL_FRAMEBUFFER_UNDEFINED"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); break;
            case GL_FRAMEBUFFER_UNSUPPORTED:                    ELOG("GL_FRAMEBUFFER_UNSUPPORTED"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:         ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:       ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); break;
            default: ELOG("Unknown framebuffer status error");
            }
        }
    }
};
class WaterBuffer
{
public:
    GLuint rtReflection = 0;
    GLuint rtRefraction = 0;
    GLuint rtReflectionDepth = 0;
    GLuint rtRefractionDepth = 0;

    FrameBuffer* fboReflection = nullptr;
    FrameBuffer* fboRefraction = nullptr;

    //Vertex
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
    std::vector<Entity> entities;
    int selectedEntity;
    std::vector<Light> lights;

    u32 globalParamsOffset;
    u32 globalParamsSize;

    FrameBuffer frameBuffer;

    int depth;

    // program indices
    u32 texturedMeshProgramIdx; //Textures indefinides
    u32 frameBufferProgramIdx; 
    u32 forwardBufferProgramIdx; 
    
    //GLuint bufferHandle;

    Buffer uniformBuffer;
    Buffer lightBuffer;
    Buffer waterBuffer;
    WaterBuffer waterbuffer;
    GLint maxUniformBufferSize = 0;
    GLint uniformBlockAlignment;

    GLuint finalAttachment;
    u32 modelPatrick;
    u32 modelPatrick1;
    u32 modelPatrick2;
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

