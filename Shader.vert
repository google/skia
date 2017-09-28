layout(location=0) in vec3 inPosition;
layout(location=1) in vec2 inTexCoord;
layout(location=2) mediump in vec3 inNormal;

out vec2 outTexCoord;
out mediump vec3 outEyePosition;
out mediump vec3 outNormal;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform float materialShininess;
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;
uniform vec3 ambientLightColor;
uniform vec3 directionalLightDirection;
uniform vec3 directionalLightColor;

void main() {
    vec4 position = vec4(inPosition, 1);
    gl_Position = projectionMatrix * modelViewMatrix * position;
    outTexCoord = inTexCoord;
    outEyePosition = (modelViewMatrix * position).xyz;
    outNormal = normalize(normalMatrix * inNormal);
}