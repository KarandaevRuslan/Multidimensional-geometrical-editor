#version 330 core

in vec3 vNormal;
in vec3 vColor;
in vec3 vWorldPos;
out vec4 fragColor;

uniform vec3 uCameraForward;       // The camera’s forward vector (to determine light direction)
uniform vec3 uLightColor;          // Base color/intensity for the directional light
uniform vec3 uAmbientColor;        // Ambient light color
uniform float uAmbientStrength;    // Intensity of the ambient light
uniform float uDirectionalStrength;// Multiplier for the directional (diffuse) light
uniform float uSpecularStrength;   // Intensity of the specular highlight
uniform float uShininess;          // Specular exponent controlling the highlight size
uniform vec3 uViewPos;             // Camera position in world space
uniform bool uApplyLighting;       // Toggle lighting on/off

void main()
{
    if (!uApplyLighting) {
        fragColor = vec4(vColor, 1.0);
        return;
    }

    // Compute ambient lighting.
    vec3 ambient = uAmbientColor * uAmbientStrength;

    // Compute directional lighting from behind the camera.
    // (Using the negative camera forward vector to simulate light coming from behind.)
    vec3 lightDir = normalize(-uCameraForward);
    vec3 norm = normalize(vNormal);
    float lambert = max(dot(norm, lightDir), 0.0);
    vec3 directional = uDirectionalStrength * uLightColor * lambert;

    // Compute specular highlights using the Blinn–Phong model.
    vec3 viewDir = normalize(uViewPos - vWorldPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(norm, halfDir), 0.0);
    float specularFactor = pow(specAngle, uShininess);
    vec3 specular = uSpecularStrength * specularFactor * uLightColor;

    // Combine lighting contributions.
    vec3 finalColor = vColor * (ambient + directional) + specular;
    fragColor = vec4(finalColor, 1.0);
}
