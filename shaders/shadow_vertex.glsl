#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

uniform mat4 uModelMatrix;
uniform mat4 uLightSpaceMatrix;

void main()
{
    gl_Position = uLightSpaceMatrix * uModelMatrix *  vec4(aPosition, 1.0);
}
