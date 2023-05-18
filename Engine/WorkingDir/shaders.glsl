///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

struct Light
{
    unsigned int type;
    vec3 color;
    vec3 direction;
    vec3 position;
};

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in vec3 aTangent;
layout(location=4) in vec3 aBitangent;

layout(binding = 0, std140) uniform GlobalParams
{
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLight[16];
};

layout(binding = 1, std140) uniform LocalParams
{
    mat4 uWorldMatrix;
    mat4 uWorldViewPorjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vNormal;
out vec3 vViewDir;

//uniform mat4 viewMatrix;
//uniform mat4 projection;
void main()
{
    vTexCoord = aTexCoord;

    //gl_Position = uWorldViewPorjectionMatrix * vec4(aPosition, 1);
    //gl_Position.z = -gl_Position.z;

    vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));

    vNormal = vec3(uWorldMatrix * vec4(aNormal, 0.0));

    vViewDir = normalize(uCameraPosition - vPosition);

    gl_Position = uWorldViewPorjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here

in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;
in vec3 vViewDir;

uniform sampler2D uTexture;

layout(binding = 0, std140) uniform GlobalParams
{
    vec3 uCameraPosition;
    unsigned int uLightCount;
    Light uLight[16];
};

layout(location=0) out vec4 oColor;

void main()
{
    vec3 lightStrenght = vec3(0.0);
    for(int i = 0; i< uLightCount; ++i)
    {
        float ambientStrenght = 0.2;
        vec3 ambient = ambientStrenght * uLight[i].color;
        
        float diff = max(dot(normalize(vNormal), normalize(uLight[i].direction)), 0.0);

        vec3 diffuse = diff * uLight[i].color;

        float specularStrength = 0.5;

        vec3 reflectDir = reflect(normalize(-uLight[i].direction), normalize(vNormal));

        float spec = pow(max(dot(normalize(vViewDir), reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * uLight[i].color;

        lightStrenght += (ambient + diffuse + specular) * texture(uTexture, vTexCoord).rgb;
    }
    oColor = vec4(lightStrenght, 1.0);
   //oColor = vec4(uLight[0].color, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
