//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Global.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(vertexShaderDefine),
        (GLint)programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(fragmentShaderDefine),
        (GLint)programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
        assert(success);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat = GL_RGB;
    GLenum dataType = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
    case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
    case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
    default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

void Init(App* app)
{
    // TODO: Initialize your resources here!
    // - vertex buffers
    // - element/index buffers
    // - vaos
    // - programs (and retrieve uniform indices)
    // - textures


    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV2V3), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV2V3), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBindVertexArray(0);

    app->lights.emplace_back(Light(LightType::DIRECTIONAL, { 1, 1, 1 }, { 1 ,1, 1 }));

    app->texturedMeshProgramIdx = LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
    Program& textureMeshProgram = app->programs[app->texturedMeshProgramIdx];
    //textureMeshProgram.vertexInputLayout.attributes.push_back({0, 3});
    //textureMeshProgram.vertexInputLayout.attributes.push_back({2, 3});
    glGetProgramiv(textureMeshProgram.handle, GL_ACTIVE_ATTRIBUTES, &textureMeshProgram.lenght);
    
    GLsizei length;
    GLint size;
    GLenum type;
    GLchar name[128];

    for (u32 i = 0; i < textureMeshProgram.lenght; ++i)
    {
        glGetActiveAttrib(textureMeshProgram.handle, i, ARRAY_COUNT(name), &length, &size, &type, name);
        GLuint attributeLocation = glGetAttribLocation(textureMeshProgram.handle, name);

        u8 test = sizeof(type);

        textureMeshProgram.vertexInputLayout.attributes.push_back({ (u8)attributeLocation, (u8)size });
    }
    

    //app->diceTexIdx = LoadTexture2D(app, "dice.png");
    //app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    //app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    //app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    //app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");


    app->glInfo.glVersion = reinterpret_cast<const char*> (glGetString(GL_VERSION));
    app->glInfo.glRender = reinterpret_cast<const char*> (glGetString(GL_RENDERER));
    app->glInfo.glVendor = reinterpret_cast<const char*> (glGetString(GL_VENDOR));
    app->glInfo.glShadingVersion = reinterpret_cast<const char*> (glGetString(GL_SHADING_LANGUAGE_VERSION));

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);

    app->uniformBuffer = CreateBuffer(app->maxUniformBufferSize, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);

    app->lightBuffer = CreateBuffer(app->maxUniformBufferSize, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);

    app->modelPatrick = LoadModel(app,"Patrick/Patrick.obj");
    ///////////////////////////////////////////FrameBuffer///////////////////////////////////////////

    glGenTextures(1, &app->frameBuffer.colorAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->frameBuffer.colorAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &app->frameBuffer.depthAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->frameBuffer.depthAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, app->displaySize.x, app->displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &app->frameBuffer.frameBufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, app->frameBuffer.frameBufferHandle);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, app->frameBuffer.colorAttachmentHandle, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, app->frameBuffer.depthAttachmentHandle,0);

    app->frameBuffer.frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (app->frameBuffer.frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (app->frameBuffer.frameBufferStatus)
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

    glDrawBuffers(1, &app->frameBuffer.colorAttachmentHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ///////////////////////////////////////////EndFrameBuffer///////////////////////////////////////////

    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for (GLint i = 0; i < numExtensions; ++i)
    {
        app->glInfo.glExtension.push_back(reinterpret_cast<const char*> (glGetStringi(GL_EXTENSIONS, GLuint(i))));
    }

    app->mode = Mode_TexturedQuad;
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
    ImGui::End();

    /*if (app->input.keys[Key::K_SPACE] == ButtonState::BUTTON_PRESS)
    {
        ImGui::OpenPopup("OpenGL Info");
    }*/

    if (ImGui::BeginPopup("OpenGL Info"))
    {
        ImGui::Text("Version %s", app->glInfo.glVersion.c_str());
        ImGui::Text("Render %s", app->glInfo.glRender.c_str());
        ImGui::Text("Vendor %s", app->glInfo.glVendor.c_str());
        ImGui::Text("GLSL Version %s", app->glInfo.glShadingVersion.c_str());

        ImGui::Text("Extensions");
        for (size_t i = 0; i < app->glInfo.glExtension.size(); ++i)
        {
            ImGui::Text("%s", app->glInfo.glExtension[i].c_str());
        }

        ImGui::EndPopup();
    }
}

void Update(App* app)
{
    app->camera.Update(app->displaySize, app);

    ///////////////////////////////////////////Lights///////////////////////////////////////////
    //Global Param
    MapBuffer(app->lightBuffer, GL_WRITE_ONLY);

    app->globalParamsOffset = app->lightBuffer.head;

    PushVec3(app->lightBuffer, app->camera.cameraPos);
    PushUInt(app->lightBuffer, app->lights.size());

    for (u32 i = 0; i < app->lights.size(); ++i)
    {
        AlignHead(app->lightBuffer, sizeof(vec4));

        Light& light = app->lights[i];
        PushUInt(app->lightBuffer, light.type);
        PushVec3(app->lightBuffer, light.color);
        PushVec3(app->lightBuffer, light.direction);
        PushVec3(app->lightBuffer, light.position);
        PushData(app->lightBuffer, &light.intesity, sizeof(float));
    }

    app->globalParamsSize = app->lightBuffer.head - app->globalParamsOffset;
    UnmapBuffer(app->lightBuffer);
    ///////////////////////////////////////////EndLights///////////////////////////////////////////
    ///////////////////////////////////////////Entities///////////////////////////////////////////
    MapBuffer(app->uniformBuffer, GL_WRITE_ONLY);
    for (Entity& entity : app->entities)
    {
        entity.worldMatrix = entity.TransformPositionScale(vec3(2.5f, 1.5f, -2.0f), glm::vec3(0.45f));
        glBindBuffer(GL_UNIFORM_BUFFER, app->uniformBuffer.handle);
                
        AlignHead(app->uniformBuffer, app->uniformBlockAlignment);

        entity.localParamsOffset = app->uniformBuffer.head;

        glm::mat4 trans = entity.worldMatrix;
        trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));
        PushMat4(app->uniformBuffer, trans);
        PushMat4(app->uniformBuffer, app->camera.projection * app->camera.view * trans);

        entity.localParamSize = app->uniformBuffer.head - entity.localParamsOffset;

        int ligma = 0;
    }
    ///////////////////////////////////////////EndEntities///////////////////////////////////////////
    UnmapBuffer(app->uniformBuffer);

    
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
    {
        if (submesh.vaos[i].programHandle == program.handle)
            return submesh.vaos[i].handle;
    }
    GLuint vaoHandle = 0;

    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

    for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
    {
        bool attributeWasLinked = false;

        for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
        {
            if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
            {
                const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;
                const u32 stride = submesh.vertexBufferLayout.stride;
                glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                glEnableVertexAttribArray(index);
                attributeWasLinked = true;
            }
        }
        assert(attributeWasLinked);
    }
    glBindVertexArray(0);

    Vao vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}

void Render(App* app)
{
    switch (app->mode)
    {
    case Mode_TexturedQuad:
    {
        // TODO: Draw your textured quad here!
        // - clear the framebuffer
        // - set the viewport
        // - set the blending state
        // - bind the texture into unit 0
        // - bind the program 
        //   (...and make its texture sample from unit 0)
        // - bind the vao
        // - glDrawElements() !!!

        //glBindFramebuffer(GL_FRAMEBUFFER, app->frameBuffer.frameBufferHandle);

        GLuint drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, drawBuffers);

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, app->displaySize.x, app->displaySize.y);

        Program& textureMeshProgram = app->programs[app->texturedMeshProgramIdx];
        glUseProgram(textureMeshProgram.handle);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->lightBuffer.handle, app->globalParamsOffset, app->globalParamsSize);

        for (Entity entity : app->entities)
        {
            Mesh& mesh = app->meshes[entity.modelIndex];

            for (u32 i = 0; i < mesh.submeshes.size(); ++i)
            {
                GLuint vao = FindVAO(mesh, i, textureMeshProgram);
                glBindVertexArray(vao);

                u32 submeshMaterialIdx = entity.materialIdx[i];
                Material& submeshMaterial = app->materials[submeshMaterialIdx];

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);

                glUniform1i(glGetUniformLocation(textureMeshProgram.handle, "uTexture"), 0);
                //glUniformMatrix4fv(glGetUniformLocation(textureMeshProgram.handle, "viewMatrix"), 1,GL_FALSE, glm::value_ptr(app->camera.view));
                //glUniformMatrix4fv(glGetUniformLocation(textureMeshProgram.handle, "projection"), 1,GL_FALSE, glm::value_ptr(app->camera.projection));

                glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->uniformBuffer.handle, entity.localParamsOffset, entity.localParamSize);

                Submesh& submesh = mesh.submeshes[i];
                glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
            }
        }
        //glBindFramebuffer(GL_FRAMEBUFFER, app->frameBuffer.frameBufferHandle);
    }
    break;

    default:;
    }
}

