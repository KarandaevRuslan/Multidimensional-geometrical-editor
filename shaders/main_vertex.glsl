#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
// If you have actual normals for surfaces:
// layout(location = 2) in vec3 aNormal;

out vec3 vColor;
// out vec3 vNormal;      // Uncomment if you have normals
out vec4 vFragPosLightSpace; // For shadow map lookup

uniform mat4 uMvpMatrix;        // Camera projection * view * model
uniform mat4 uModelMatrix;      // Model matrix (object -> world)
uniform mat4 uLightSpaceMatrix; // Light's projection * view

void main()
{
    gl_Position = uMvpMatrix * vec4(aPosition, 1.0);
    vColor      = aColor;

    // If using real geometry with normals:
    // vNormal = mat3(uModelMatrix) * aNormal;

    // Transform world position into light space
    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vFragPosLightSpace = uLightSpaceMatrix * worldPos;
}
