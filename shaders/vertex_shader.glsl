#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

uniform mat4 uMvpMatrix;
uniform mat4 uModelMatrix; // Allows proper world-space transformation

out vec3 vNormal;
out vec3 vColor;
out vec3 vWorldPos;

void main()
{
    // Pass vertex color directly.
    vColor = aColor;

    // Compute world-space position.
    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;

    // Transform normals correctly (using the inverse transpose for non-uniform scaling).
    vNormal = mat3(transpose(inverse(uModelMatrix))) * aNormal;

    // Standard MVP transform.
    gl_Position = uMvpMatrix * vec4(aPosition, 1.0);
}
