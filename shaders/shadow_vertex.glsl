#version 330 core

/**
 *  Vertex position in object space.
 */
layout(location = 0) in vec3 aPosition;

/**
 *  Vertex normal in object space (unused for depth pass, but may be present in VBO).
 */
layout(location = 1) in vec3 aNormal;

/**
 *  Vertex color (unused for depth pass, but may be present in VBO).
 */
layout(location = 2) in vec3 aColor;

/**
 *  Combined light-space matrix for depth pass.
 */
uniform mat4 uLightSpaceMatrix;

void main()
{
    gl_Position = uLightSpaceMatrix * vec4(aPosition, 1.0);
}
