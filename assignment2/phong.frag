#version 140

in vec2 TexCoord;
in vec3 pos;
out vec4 fColor;

uniform int renderMode;
uniform vec3 lightPosition;
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D roughnessTexture;

void main()
{
    if (renderMode == 0) {
         vec3 color = vec3(250.0 / 255.0, 235.0 / 255.0, 214.0 / 255.0); // 기본 색상

        // 최종 색상 적용
        fColor = vec4(color, 1.0);
    } else if (renderMode == 1) {
      // Texture-based rendering with lighting and material properties
        vec4 diffuseColor = texture(diffuseTexture, TexCoord); // 텍스처에서 Diffuse 색상 가져오기
        vec3 normal = normalize(texture(normalTexture, TexCoord).rgb * 2.0 - 1.0); // Normal 맵 처리
        float roughness = texture(roughnessTexture, TexCoord).r; // Roughness 값 (0~1)
        vec3 lightDir = normalize(lightPosition - pos); // 광원 방향

        // **Diffuse 계산**
        float diff = max(dot(normalize(normal), lightDir), 0.0);
        vec3 diffuse = diff * diffuseColor.rgb; // Kd 적용

        // **Specular 계산 (Phong Reflection Model)**
        vec3 viewDir = normalize(-pos); // 카메라 방향 (카메라가 원점에 있다고 가정)
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 54.223116); // Ns를 사용해 하이라이트 조정
        vec3 specular = spec * vec3(1, 1, 1); // Ks 적용

        vec3 ambient = vec3(0.1, 0.1, 0.1); // Ambient 색상

        // 최종 색상 계산
        vec3 finalColor = ambient + diffuse + specular; // Ambient + Diffuse + Specular
        fColor = vec4(diffuse+specular, 1.0);
    }
}
