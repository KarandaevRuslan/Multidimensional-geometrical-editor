#version 330 core

/**
 *  Interpolated normal from the vertex shader.
 */
in vec3 vNormal;

/**
 *  Interpolated color from the vertex shader.
 */
in vec3 vColor;

/**
 *  World-space position of this fragment.
 */
in vec3 vWorldPos;

/**
 *  Light-space coordinate for shadow testing.
 */
in vec4 vShadowCoord;

/**
 *  Final output color of this fragment.
 */
out vec4 fragColor;

// ---------------------------
// Lighting control
// ---------------------------

/**
 *  Toggles lighting on/off. If false, color is used directly.
 */
uniform bool uApplyLighting;

/**
 *  Toggles shadows on/off. If false, shadows will not be applied.
 */
uniform bool uApplyShadow;

/**
 *  Shininess exponent for specular lighting.
 */
uniform float uShininess;

/**
 *  Factor controlling how strong ambient lighting is.
 */
uniform float uAmbientStrength;

/**
 *  Strength of specular highlights.
 */
uniform float uSpecularStrength;

/**
 *  Strength of primary directional (diffuse) lighting.
 */
uniform float uDirectionalStrength;

/**
 *  Intensity/strength of the additional light.
 */
uniform float uShadowLightStrength;

/**
 *  Blend factor between main and shadow lighting (0.0 - 1.0).
 */
uniform float uColorBlendFactor;

// ---------------------------
// Light properties
// ---------------------------

/**
 *  Ambient color in the scene.
 */
uniform vec3 uAmbientColor;

/**
 *  Light color used for the primary directional lighting.
 */
uniform vec3 uLightColor;

/**
 *  Color of the additional light (from uShadowDir).
 */
uniform vec3 uShadowLightColor;

// ---------------------------
// Light directions & positions
// ---------------------------

/**
 *  Direction of the light for shading (primary light).
 */
uniform vec3 uCameraForward;

/**
 *  Direction from which the shadow light is coming.
 */
uniform vec3 uShadowDir;

/**
 *  Position of the camera, needed for specular calculations.
 */
uniform vec3 uViewPos;

/**
 *  Position of the light for shading, needed for specular calculations.
 */
uniform vec3 uShadowViewPos;

// ---------------------------
// Shadow mapping
// ---------------------------

/**
 *  Dimension of the PCF kernel.
 */
uniform int uPcfKernelDim;

/**
 *  Scale factor for shadow bias based on surface angle.
 */
uniform float uShadowBiasScale;

/**
 *  Minimum shadow bias.
 */
uniform float uShadowBiasMin;

/**
 *  Shadow map to compare depth values against (sampler2DShadow).
 */
uniform sampler2DShadow uShadowMap;

vec3 computeLighting(vec3 lightDir, vec3 lightColor, float lightStrength, vec3 viewPos, vec3 ambient) {
    vec3 norm     = normalize(vNormal);
    float lambert = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = lightStrength * lightColor * lambert;
    vec3 viewDir  = normalize(viewPos - vWorldPos);
    vec3 halfDir  = normalize(lightDir + viewDir);
    float spec    = pow(max(dot(norm, halfDir), 0.0), uShininess);
    vec3 specular = uSpecularStrength * spec * lightColor;
    return vColor * (ambient + diffuse) + specular;
}

float computeShadowFactor(vec3 norm) {
    float bias = max(uShadowBiasScale * (1.0 - dot(norm, normalize(uShadowDir))),
                     uShadowBiasMin);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    float kernelRadius = float(uPcfKernelDim - 1) * 0.5;

    for (int x = 0; x < uPcfKernelDim; ++x) {
        for (int y = 0; y < uPcfKernelDim; ++y) {
            vec2 offset = (vec2(float(x), float(y)) - kernelRadius) * texelSize;
            vec4 offsetCoord = vShadowCoord;
            offsetCoord.xy += offset;
            offsetCoord.z  -= bias;
            shadow += textureProj(uShadowMap, offsetCoord);
        }
    }

    return shadow / float(uPcfKernelDim * uPcfKernelDim);
}

void main()
{
    if (!uApplyLighting) {
        fragColor = vec4(vColor, 1.0);
        return;
    }

    vec3 norm    = normalize(vNormal);
    vec3 ambient = uAmbientColor * uAmbientStrength;

    // ----- Primary lighting (camera forward) -----
    vec3 lightDirMain = normalize(-uCameraForward);
    vec3 litColorMain = computeLighting(lightDirMain, uLightColor, uDirectionalStrength, uViewPos, ambient);

    // ----- Shadow lighting -----
    float shadow = computeShadowFactor(norm);
    vec3 lightDirShadow = normalize(-uShadowDir);
    vec3 litColorShadow = computeLighting(lightDirShadow, uShadowLightColor, uShadowLightStrength, uShadowViewPos, ambient);

    // ----- Final -----
    if (!uApplyShadow)
    {
        fragColor = vec4(litColorMain, 1.0);
        return;
    }

    vec3 finalColor = uColorBlendFactor * litColorMain + (1.0 - uColorBlendFactor) * litColorShadow * shadow;
    // vec3 finalColor = vec3(shadow); // debug shadows
    fragColor = vec4(finalColor, 1.0);
}
