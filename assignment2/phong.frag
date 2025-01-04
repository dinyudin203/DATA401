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
         vec3 color = vec3(250.0 / 255.0, 235.0 / 255.0, 214.0 / 255.0); // �⺻ ����

        // ���� ���� ����
        fColor = vec4(color, 1.0);
    } else if (renderMode == 1) {
      // Texture-based rendering with lighting and material properties
        vec4 diffuseColor = texture(diffuseTexture, TexCoord); // �ؽ�ó���� Diffuse ���� ��������
        vec3 normal = normalize(texture(normalTexture, TexCoord).rgb * 2.0 - 1.0); // Normal �� ó��
        float roughness = texture(roughnessTexture, TexCoord).r; // Roughness �� (0~1)
        vec3 lightDir = normalize(lightPosition - pos); // ���� ����

        // **Diffuse ���**
        float diff = max(dot(normalize(normal), lightDir), 0.0);
        vec3 diffuse = diff * diffuseColor.rgb; // Kd ����

        // **Specular ��� (Phong Reflection Model)**
        vec3 viewDir = normalize(-pos); // ī�޶� ���� (ī�޶� ������ �ִٰ� ����)
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 54.223116); // Ns�� ����� ���̶���Ʈ ����
        vec3 specular = spec * vec3(1, 1, 1); // Ks ����

        vec3 ambient = vec3(0.1, 0.1, 0.1); // Ambient ����

        // ���� ���� ���
        vec3 finalColor = ambient + diffuse + specular; // Ambient + Diffuse + Specular
        fColor = vec4(diffuse+specular, 1.0);
    }
}
