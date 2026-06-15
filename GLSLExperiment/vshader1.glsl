#version 400

in vec4 vPosition;
in vec4 vColor;
in vec4 vNormal;

out vec4 color;
out vec3 N;
out vec3 posEye;

uniform mat4 ModelView;
uniform mat4 Projection;

void main()
{
    vec4 positionEye = ModelView * vPosition;

    posEye = positionEye.xyz;
    N = normalize((ModelView * vNormal).xyz);
    color = vColor;

    gl_Position = Projection * positionEye;
}//