#version 330 core

in vec3 vColor;
// in vec3 vNormal; // Uncomment if you have real normals
in vec4 vFragPosLightSpace;

out vec4 fragColor;

uniform sampler2D uShadowMap;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;

void main()
{
    // --------------------------------------------------
    // 1) Basic lighting (requires normal)
    //    For lines/points, a "fake" normal or skip entirely.
    // --------------------------------------------------
    //vec3 norm = normalize(vNormal);
    //vec3 lightDir = normalize(uLightPos - <some world pos>);

    // float diff  = max(dot(norm, lightDir), 0.0);
    // vec3 diffuse  = diff * uLightColor;
    // ... specular, etc.

    // For demonstration, we skip real normal-based shading
    // and rely on vColor as a placeholder:
    vec3 lighting = vColor * (uLightColor + uAmbientColor);

    // --------------------------------------------------
    // 2) Shadow calculation
    // --------------------------------------------------
    // Perspective divide
    vec3 projCoords = vFragPosLightSpace.xyz / vFragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // If outside shadow map, skip
    if(projCoords.x < 0.0 || projCoords.x > 1.0 ||
       projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        // No shadow clamp if out of range
        fragColor = vec4(lighting, 1.0);
        return;
    }

    float closestDepth = texture(uShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // Offset/bias to reduce shadow acne
    float bias = 0.005;
    // If in shadow, reduce the lighting
    float shadow = (currentDepth - bias) > closestDepth ? 0.5 : 1.0;

    vec3 finalColor = lighting * shadow;
    fragColor = vec4(finalColor, 1.0);
}
