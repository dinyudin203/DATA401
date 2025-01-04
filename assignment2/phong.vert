#version 140

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

out vec2 TexCoord;
out vec3 pos, normal;

uniform mat4 model, view, projection;
uniform int renderMode;
uniform sampler2D normalTexture;

void main()
{
    if (renderMode == 0) {
        vec4 temp = projection * view * vec4(vPosition, 1.0);
        pos = temp.xyz;
        TexCoord = vTexCoord;
        gl_Position = projection * view * vec4(vPosition, 1.0);
        normal = normalize((transpose(inverse(view)) * vec4(vNormal, 0.0)).xyz);
    } else if (renderMode == 1) {
        vec4 temp = projection * view * model * vec4(vPosition, 1.0);
        pos = temp.xyz;
        TexCoord = vTexCoord;
        gl_Position = projection * view * model* vec4(vPosition, 1.0);
        normal = normalize((transpose(inverse(view * model)) * vec4(vNormal, 0.0)).xyz);
    }
}
