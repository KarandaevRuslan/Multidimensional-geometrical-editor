#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

uniform mat4 uMvpMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uLightSpaceMatrix; // <-- Add this

out vec3 vNormal;
out vec3 vColor;
out vec3 vWorldPos;

// ADD THIS
out vec4 vShadowCoord;

void main()
{
    vColor = aColor;

    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;

    vNormal = mat3(transpose(inverse(uModelMatrix))) * aNormal;

    // The usual MVP
    gl_Position = uMvpMatrix * vec4(aPosition, 1.0);

    // Add a projected coordinate from the light’s perspective:
    vShadowCoord = uLightSpaceMatrix * worldPos;


}
