//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "Global.h"
#include <imgui.h>
#include <imgui_internal.h>
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

bool DrawVec3(const char* name, glm::vec3& vec)
{
    glm::vec3 lastVec = vec;
    ImGui::PushID(name);

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 100.0f);
    ImGui::Text(name);
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    ImGui::Button("X");
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##X", &vec.x, 0.1f, 0.0f, 0.0f, "%.2f");
    if (ImGui::IsItemActivated())
        //CommandDispatcher::Execute(new MoveGameObjectCommand(owner));
        ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::Button("Y");
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Y", &vec.y, 0.1f, 0.0f, 0.0f, "%.2f");
    if (ImGui::IsItemActivated())
        //CommandDispatcher::Execute(new MoveGameObjectCommand(owner));
        ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    ImGui::Button("Z");
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::DragFloat("##Z", &vec.z, 0.1f, 0.0f, 0.0f, "%.2f");
    if (ImGui::IsItemActivated())
        //CommandDispatcher::Execute(new MoveGameObjectCommand(owner));
        ImGui::PopItemWidth();

    ImGui::PopStyleVar();

    ImGui::Columns(1);

    ImGui::PopID();

    if (lastVec.x != vec.x || lastVec.y != vec.y || lastVec.z != vec.z)
        return true;
    else return false;
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

    app->texturedMeshProgramIdx = LoadProgram(app, "geometryShaders.glsl", "TEXTURED_GEOMETRY");
    app->frameBufferProgramIdx = LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");

    Program& textureMeshProgram = app->programs[app->texturedMeshProgramIdx];
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
    //////

    Program& frameBufferProgram = app->programs[app->frameBufferProgramIdx];
    glGetProgramiv(frameBufferProgram.handle, GL_ACTIVE_ATTRIBUTES, &frameBufferProgram.lenght);

    for (u32 i = 0; i < frameBufferProgram.lenght; ++i)
    {
        glGetActiveAttrib(frameBufferProgram.handle, i, ARRAY_COUNT(name), &length, &size, &type, name);
        GLuint attributeLocation = glGetAttribLocation(frameBufferProgram.handle, name);

        u8 test = sizeof(type);

        frameBufferProgram.vertexInputLayout.attributes.push_back({ (u8)attributeLocation, (u8)size });
    }
    
    app->depth = 0;


    app->glInfo.glVersion = reinterpret_cast<const char*> (glGetString(GL_VERSION));
    app->glInfo.glRender = reinterpret_cast<const char*> (glGetString(GL_RENDERER));
    app->glInfo.glVendor = reinterpret_cast<const char*> (glGetString(GL_VENDOR));
    app->glInfo.glShadingVersion = reinterpret_cast<const char*> (glGetString(GL_SHADING_LANGUAGE_VERSION));

    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);

    app->uniformBuffer = CreateBuffer(app->maxUniformBufferSize, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);

    app->lightBuffer = CreateBuffer(app->maxUniformBufferSize, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);

    app->modelPatrick = LoadModel(app,"Patrick/Patrick.obj", std::string("Patrick"), {-5,1,1}, {0,0,0}, {1,1,1});
    app->selectedEntity = 0;
    app->modelPatrick = LoadModel(app,"Patrick/Patrick.obj", std::string("Patri"), {1,1,1}, {0,0,0}, {1,1,1});

    app->modelPatrick = LoadModel(app,"Patrick/NoSeProfe.obj", std::string("Hola profe"), {1,1,1}, {0,0,0}, {1,1,1});

    //app->modelPatrick = LoadModel(app,"Patrick/Plane.obj", std::string("Plane"), {0,0,0}, {0,0,0}, {1,1,1});

    Light lightdios = Light(LightType::DIRECTIONAL, { 1, 1, 1 }, { 1 ,1, 1 }, 10.0f, glm::normalize(glm::vec3(1.0, 1.0, 1.0)));
    app->lights.emplace_back(lightdios);

    Light lightdios1 = Light(LightType::DIRECTIONAL, { 10, 1, 5 }, { 1 ,0, 0 }, 10.0f, glm::normalize(glm::vec3(1.0, 1.0, 1.0)));
    app->lights.emplace_back(lightdios1);

    Light lightdios2 = Light(LightType::DIRECTIONAL, { -10, -6, 1 }, { 1 ,1, 0 }, 10.0f, glm::normalize(glm::vec3(1.0, 1.0, 1.0)));
    app->lights.emplace_back(lightdios2);

    Light PointLight = Light(LightType::POINT_LIGHT, { -6, 0, 1 }, { 0. ,0., 1. }, 10.0f, glm::normalize(glm::vec3(0, 0, 0)));
    //app->lights.emplace_back(lightdios1);


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

    glGenTextures(1, &app->frameBuffer.normalAttachment);
    glBindTexture(GL_TEXTURE_2D, app->frameBuffer.normalAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &app->frameBuffer.positionAttachment);
    glBindTexture(GL_TEXTURE_2D, app->frameBuffer.positionAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, app->frameBuffer.normalAttachment, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, app->frameBuffer.positionAttachment, 0);
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

    app->finalAttachment = app->frameBuffer.colorAttachmentHandle;
    app->mode = Mode_TexturedQuad;
}
void ShowChildren(App* app)
{
    for (int i = 0; i < app->entities.size(); ++i)
    {
        if (ImGui::Button(app->entities[i].name.c_str()))
        {
            app->selectedEntity = i;
        }
    }

    ImGui::Dummy(ImVec2(0.0f, 15.0f));
    ImGui::Text("Lights: ");

    for (int i = 0; i < app->lights.size(); ++i)
    {
        if (ImGui::Button(app->lights[i].name.c_str()))
        {
            //app->selectedEntity = i + app->entities.size();
        }
    }
    
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Dummy(ImVec2(0.0f, 15.0f));
    ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
    ImGui::Dummy(ImVec2(0.0f, 15.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 15.0f));

    if (app->input.keys[Key::K_SPACE] == ButtonState::BUTTON_PRESS)
    {
        ImGui::OpenPopup("OpenGL Info");
    }
    if (DrawVec3("Position: ", app->entities[app->selectedEntity].position))
    {
        app->entities[app->selectedEntity].worldMatrix = app->entities[app->selectedEntity].TransformPositionScale(app->entities[app->selectedEntity].position, glm::vec3(1.0f));
    }
    ImGui::Dummy(ImVec2(0.0f, 15.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 15.0f));

    const char* items[] = { "Albedo", "Normal", "Position", "Depth"};
    static int item_current_idx = 0;
    const char* combo_label = items[item_current_idx];

    ImGui::Text("Render Mode: ");
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    if (ImGui::BeginCombo("", combo_label))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(items[n], is_selected))
            {
                item_current_idx = n;
                switch (n)
                {
                case 0:
                    app->finalAttachment = app->frameBuffer.colorAttachmentHandle;
                    app->depth = 0;
                    break;
                case 1:
                    app->finalAttachment = app->frameBuffer.normalAttachment;
                    app->depth = 0;
                    break;
                case 2:
                    app->finalAttachment = app->frameBuffer.positionAttachment;
                    app->depth = 0;
                    break;
                case 3:
                    app->finalAttachment = app->frameBuffer.depthAttachmentHandle;
                    app->depth = 1;
                    break;
                default:
                    app->finalAttachment = app->frameBuffer.colorAttachmentHandle;
                    break;
                }
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Dummy(ImVec2(0.0f, 15.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 15.0f));
    if (ImGui::TreeNodeEx("root", ImGuiTreeNodeFlags_DefaultOpen, "GameObjects"))
    {
        ShowChildren(app);
        ImGui::TreePop();
    }
    
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
    ImGui::End();
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
        int insda = 0;

    }

    app->globalParamsSize = app->lightBuffer.head - app->globalParamsOffset;
    UnmapBuffer(app->lightBuffer);
    ///////////////////////////////////////////EndLights//////////////////////////////////////////
    ///////////////////////////////////////////Entities///////////////////////////////////////////
    MapBuffer(app->uniformBuffer, GL_WRITE_ONLY);
    for (Entity& entity : app->entities)
    {
        entity.worldMatrix = entity.TransformPositionScale(entity.position, glm::vec3(1.0f));
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

        glBindFramebuffer(GL_FRAMEBUFFER, app->frameBuffer.frameBufferHandle);

        glViewport(0, 0, app->displaySize.x, app->displaySize.y);

        glEnable(GL_DEPTH_TEST);

        GLuint drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, drawBuffers);

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


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
                
                glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->uniformBuffer.handle, entity.localParamsOffset, entity.localParamSize);

                Submesh& submesh = mesh.submeshes[i];
                glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //////FrameBuffer

        Program& frameBufferProgram = app->programs[app->frameBufferProgramIdx];
        glUseProgram(frameBufferProgram.handle);

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(app->vao);

        glUniform1i(glGetUniformLocation(frameBufferProgram.handle, "uTexture"), 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, app->finalAttachment);

        glUniform1i(glGetUniformLocation(frameBufferProgram.handle, "isDepth"), app->depth);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    }
    break;

    default:;
    }
}