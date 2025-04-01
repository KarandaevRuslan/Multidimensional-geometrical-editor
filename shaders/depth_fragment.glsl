#version 330 core
// We do not output any color here. Just let OpenGL record the fragment depth.
void main()
{
    // gl_FragCoord.z is automatically written to the depth buffer
}
