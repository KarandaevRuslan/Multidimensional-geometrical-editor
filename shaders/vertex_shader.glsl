#version 330 core

/**
 *  Vertex position in object space.
 */
layout(location = 0) in vec3 aPosition;

/**
 *  Vertex normal in object space.
 */
layout(location = 1) in vec3 aNormal;

/**
 *  Vertex color in object space.
 */
layout(location = 2) in vec3 aColor;

/**
 *  Interpolated normal passed to the fragment shader.
 */
out vec3 vNormal;

/**
 *  Interpolated color passed to the fragment shader.
 */
out vec3 vColor;

/**
 *  World-space position passed to the fragment shader.
 */
out vec3 vWorldPos;

/**
 *  Light-space position for shadow mapping.
 */
out vec4 vShadowCoord;

/**
 *  Combined projection-view matrix from camera.
 */
uniform mat4 uMvpMatrix;

/**
 *  Model matrix for transforming from object to world space.
 */
uniform mat4 uModelMatrix;

/**
 *  Matrix for transforming world coordinates to light-space.
 */
uniform mat4 uLightSpaceMatrix;

void main()
{
    vColor = aColor;

    vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
    vWorldPos     = worldPos.xyz;

    // Correct normal transform
    vNormal = mat3(transpose(inverse(uModelMatrix))) * aNormal;

    gl_Position = uMvpMatrix * vec4(aPosition, 1.0);

    // Light-space position (for shadow lookups)
    vShadowCoord = uLightSpaceMatrix * worldPos;
}
