#version 330 core

in vec3 vNormal;
in vec3 vColor;
in vec3 vWorldPos;

// ADD THIS
in vec4 vShadowCoord; // We’ll pass this from the vertex shader

out vec4 fragColor;

// Already in your code:
uniform vec3  uCameraForward;
uniform vec3  uShadowDir;
uniform vec3  uLightColor;
uniform vec3  uAmbientColor;
uniform float uAmbientStrength;
uniform float uDirectionalStrength;
uniform float uSpecularStrength;
uniform float uShininess;
uniform vec3  uViewPos;
uniform bool  uApplyLighting;

// ADD THESE UNIFORMS
uniform sampler2DShadow uShadowMap;
uniform mat4 uLightSpaceMatrix;

void main()
{
    // If lighting is off, do exactly what you had before:
    if (!uApplyLighting) {
        fragColor = vec4(vColor, 1.0);
        return;
    }

    // 1) Normal lighting steps (unchanged)
    vec3 ambient = uAmbientColor * uAmbientStrength;
    vec3 lightDir = normalize(-uCameraForward);
    vec3 norm = normalize(vNormal);
    float lambert = max(dot(norm, lightDir), 0.0);
    vec3 directional = uDirectionalStrength * uLightColor * lambert;

    vec3 viewDir = normalize(uViewPos - vWorldPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(norm, halfDir), 0.0);
    float specularFactor = pow(specAngle, uShininess);
    vec3 specular = uSpecularStrength * specularFactor * uLightColor;

    // 2) Combine them ignoring shadows (so far)
    vec3 litColor = vColor * (ambient + directional) + specular;

    // 3) Calculate a shadow factor in [0..1]
    //    We need the shadowMap and vShadowCoord (projected light space).
    //    If you used an orthographic light, or a 4D lightSpaceCoord, we do:
    //        float shadow = textureProj(uShadowMap, vShadowCoord);
    //    The result is ~1.0 if lit, ~0.0 if in shadow.  So multiply final color by that.
    //    Check that we have the correct .xyz/.w arrangement.
    // Вычисление динамического bias (опционально, для борьбы с shadow acne)
    float bias = max(0.001 * (1.0 - dot(norm, uShadowDir)), 0.0008);

    // PCF – сглаживание теней
    float shadow = 0.0;
    // Определяем размер одного текселя теневой карты
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    for (int x = 0; x < 4; ++x) {
        for (int y = 0; y < 4; ++y) {
            vec4 offsetCoord = vShadowCoord;
            offsetCoord.xy += (vec2(float(x), float(y)) - 1.5) * texelSize;
            // Применяем bias к z-компоненте
            offsetCoord.z -= bias;
            shadow += textureProj(uShadowMap, offsetCoord);
        }
    }
    // Усредняем 16 выборок
    shadow /= 16.0;

    // Итоговый цвет с учетом тени
    vec3 finalColor = litColor * (0.7 + 0.35 * shadow);

    fragColor = vec4(finalColor, 1.0);

    // Debug

    // fragColor = vec4(vec3(shadow), 1.0);

    // vec3 shadowCoord = vShadowCoord.xyz / vShadowCoord.w;
    // fragColor = vec4(shadowCoord, 1.0);

    // fragColor = vec4(1.0, 1.0, 0.0, 1.0);
    // vec3 proj = vShadowCoord.xyz / vShadowCoord.w;
    // if (proj.x < 0.0 || proj.x > 1.0 ||
    //     proj.y < 0.0 || proj.y > 1.0 ||
    //     proj.z < 0.0 || proj.z > 1.0) {
    //     fragColor = vec4(1.0, 0.0, 0.0, 1.0); // красный = вне shadow map
    //     return;
    // }
}
