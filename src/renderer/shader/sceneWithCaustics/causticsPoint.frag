#version 430 core

in vec4 vCenterPositionWorldSpace;

layout(location = 0) out vec4 fColor;

// uniform
uniform int uFlag;
uniform float uPhotonEnergy;

void main() {
    // w-component is 0 means invalid
    if (vCenterPositionWorldSpace.w != 1.0) {
        // return;
        discard;
    }

    // fColor = vec4(vec3(0.1), 1.0);
    if (uFlag == 0) {
        fColor = vec4(vec3(uPhotonEnergy), uPhotonEnergy);
    } else if (uFlag == 1) {
        fColor = vec4(uPhotonEnergy, 0.0, 0.0, uPhotonEnergy);
    } else if (uFlag == 2) {
        fColor = vec4(0.0, uPhotonEnergy, 0.0, uPhotonEnergy);
    } else if (uFlag == 3) {
        fColor = vec4(0.0, 0.0, uPhotonEnergy, uPhotonEnergy);
    }
}
