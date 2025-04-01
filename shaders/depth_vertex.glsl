#version 330 core
layout(location = 0) in vec3 aPosition;

uniform mat4 uLightMvpMatrix;

void main()
{
    // We only need position data to write the fragment's depth.
    gl_Position = uLightMvpMatrix * vec4(aPosition, 1.0);
}
