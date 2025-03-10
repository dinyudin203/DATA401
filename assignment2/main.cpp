#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <assert.h>
#include "textfile.h"
#include "Angel.h"
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

vec3 lightPosition = vec3(-5.77, 5.77, -5.77);


// Shader programs
GLuint p;
GLuint diffuseTexture, normalTexture, roughnessTexture;
GLuint renderMode = 0;

// Camera and Model transformations
vec3 eyePosition = vec3(-10, 50, 50);
vec3 modelPosition = vec3(0, 0, 0);
float modelRotationAngle = 0.0f; // Model rotation angle (Y-axis)
mat4 modelMatrix = Scale(1.0, 1.0, 1.0);

vec4 cameraOffset = vec4(0.0, 50.0, 50.0, 1.0); // 카메라가 모델과 떨어진 위치
float modelSpeed = 5.0f;
float modelRotationSpeed = 90.0f; // Degrees per second

bool isPerspective = true;
GLuint buffer, ibo;
int verticesNum, indicesNum;
vec3* vertices;
vec3* vertex_normals;
vec2* texCoords;
int* indices;

mat4 viewMatrix = LookAt(eyePosition, modelPosition, vec4(0, 1, 0, 0));


int vertices_size;
int indices_size;
int normals_size;
int texture_size;
// Debugging helper
void checkGLError(const char* context) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        printf("GL Error in %s: %d\n", context, error);
    }
}


GLuint loadTexture(const char* filePath) {
    printf("Attempting to load texture: %s\n", filePath);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
    if (!data) {
        printf("stbi_load failed for %s. Check the file path or format.\n", filePath);
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    if (textureID == 0) {
        printf("glGenTextures failed for %s. Check OpenGL initialization.\n", filePath);
        checkGLError("glGenTextures");
        stbi_image_free(data);
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);

    if (nrChannels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else if (nrChannels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
        printf("Unsupported channel count (%d) in texture: %s\n", nrChannels, filePath);
        stbi_image_free(data);
        return 0;
    }

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    printf("Texture loaded successfully: %s (ID: %u, Width: %d, Height: %d)\n", filePath, textureID, width, height);

    stbi_image_free(data);
    return textureID;
}

void replaceFileExtension(char* filePath, const char* newExtension) {
    char* dot = strrchr(filePath, '.'); // 파일 경로에서 마지막 '.' 찾기
    if (dot) {
        strcpy(dot, newExtension); // 확장자 대체
    }
}

void loadMTL(const char* filename) {
    printf("Attempting to load MTL file: %s\n", filename);

    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open MTL file: %s\n", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "map_Kd", 6) == 0) {
            char texturePath[256];
            sscanf(line, "map_Kd %s", texturePath);
            replaceFileExtension(texturePath, ".png"); // .tif -> .png
            diffuseTexture = loadTexture(texturePath);
        }
        else if (strncmp(line, "map_Bump", 8) == 0) {
            char texturePath[256];
            sscanf(line, "map_Bump %s", texturePath);
            replaceFileExtension(texturePath, ".png"); // .tif -> .png
            normalTexture = loadTexture(texturePath);
        }
        else if (strncmp(line, "map_Ns", 6) == 0) {
            char texturePath[256];
            sscanf(line, "map_Ns %s", texturePath);
            replaceFileExtension(texturePath, ".png"); // .tif -> .png
            roughnessTexture = loadTexture(texturePath);
        }
    }

    fclose(file);
    printf("MTL file loaded successfully: %s\n", filename);
}

// Read .obj file
void readObjFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open the .obj file.\n");
        exit(1);
    }

    // 파일의 전체 라인을 탐색하여 꼭짓점(vertex) 및 노멀(normal) 개수 계산
    char lineHeader[128];
    int vertexCount = 0, normalCount = 0, texCoordCount = 0, faceCount = 0;

    while (fscanf(file, "%s", lineHeader) != EOF) {
        if (strcmp(lineHeader, "v") == 0) {
            vertexCount++;
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            normalCount++;
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            texCoordCount++;
        }
        else if (strcmp(lineHeader, "f") == 0) {
            faceCount++;
        }
    }

    // 배열 크기 할당
    verticesNum = vertexCount;
    indicesNum = faceCount * 3; // 삼각형 기준

    vertices = (vec3*)malloc(sizeof(vec3) * verticesNum);
    vertex_normals = (vec3*)malloc(sizeof(vec3) * normalCount);
    indices = (int*)malloc(sizeof(int) * indicesNum);
    texCoords = (vec2*)malloc(sizeof(vec2) * texCoordCount);

    if (vertices == NULL || vertex_normals == NULL || indices == NULL || texCoords == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    // 파일 다시 읽기
    rewind(file);

    int vertexIndex = 0, normalIndex = 0, indexIndex = 0, texCoordsIndex = 0;
    while (fscanf(file, "%s", lineHeader) != EOF) {
        if (strcmp(lineHeader, "v") == 0) {
            // 꼭짓점 읽기
            fscanf(file, "%f %f %f\n", &vertices[vertexIndex].x, &vertices[vertexIndex].y, &vertices[vertexIndex].z);
            vertexIndex++;
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            // 노멀 벡터 읽기
            fscanf(file, "%f %f %f\n", &vertex_normals[normalIndex].x, &vertex_normals[normalIndex].y, &vertex_normals[normalIndex].z);
            normalIndex++;
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            // 텍스처 좌표 읽기
            fscanf(file, "%f %f\n", &texCoords[texCoordsIndex].x, &texCoords[texCoordsIndex].y);
            texCoordsIndex++;
        }
        else if (strcmp(lineHeader, "f") == 0) {
            // 면(face) 데이터 읽기
            int vertexIdx[3], uvIdx[3], normalIdx[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                &vertexIdx[0], &uvIdx[0], &normalIdx[0],
                &vertexIdx[1], &uvIdx[1], &normalIdx[1],
                &vertexIdx[2], &uvIdx[2], &normalIdx[2]);
            if (matches != 9) {
                printf("Error reading face data in .obj file.\n");
                exit(1);
            }

            // 인덱스를 저장
            indices[indexIndex++] = vertexIdx[0] - 1;
            indices[indexIndex++] = vertexIdx[1] - 1;
            indices[indexIndex++] = vertexIdx[2] - 1;
        }
    }
    fclose(file);

    // 크기 계산
    vertices_size = sizeof(vec3) * verticesNum;
    normals_size = sizeof(vec3) * normalCount;
    texture_size = sizeof(vec2) * texCoordCount;
    indices_size = sizeof(int) * indicesNum;

    printf("File loaded: %d vertices, %d normals, %d indices\n",
        verticesNum, normalCount, indicesNum);
}

void drawCube(vec3 lightPosition, float size, mat4 rotationMatrix) {
    float halfSize = size / 2.0f;

    // 각 면의 정점 계산 (회전 적용)
    vec4 p1 = rotationMatrix * vec4(-halfSize, -halfSize, -halfSize, 1.0f);
    vec4 p2 = rotationMatrix * vec4(halfSize, -halfSize, -halfSize, 1.0f);
    vec4 p3 = rotationMatrix * vec4(halfSize, halfSize, -halfSize, 1.0f);
    vec4 p4 = rotationMatrix * vec4(-halfSize, halfSize, -halfSize, 1.0f);

    vec4 p5 = rotationMatrix * vec4(-halfSize, -halfSize, halfSize, 1.0f);
    vec4 p6 = rotationMatrix * vec4(halfSize, -halfSize, halfSize, 1.0f);
    vec4 p7 = rotationMatrix * vec4(halfSize, halfSize, halfSize, 1.0f);
    vec4 p8 = rotationMatrix * vec4(-halfSize, halfSize, halfSize, 1.0f);

    // 중심 이동 (lightPosition 기준으로 이동)
    p1 = vec4(lightPosition, 1.0f) + p1;
    p2 = vec4(lightPosition, 1.0f) + p2;
    p3 = vec4(lightPosition, 1.0f) + p3;
    p4 = vec4(lightPosition, 1.0f) + p4;
    p5 = vec4(lightPosition, 1.0f) + p5;
    p6 = vec4(lightPosition, 1.0f) + p6;
    p7 = vec4(lightPosition, 1.0f) + p7;
    p8 = vec4(lightPosition, 1.0f) + p8;

    // 정육면체 그리기
    glBegin(GL_QUADS);

    // 뒤쪽 면
    glVertex3f(p1.x, p1.y, p1.z);
    glVertex3f(p2.x, p2.y, p2.z);
    glVertex3f(p3.x, p3.y, p3.z);
    glVertex3f(p4.x, p4.y, p4.z);

    // 앞쪽 면
    glVertex3f(p5.x, p5.y, p5.z);
    glVertex3f(p6.x, p6.y, p6.z);
    glVertex3f(p7.x, p7.y, p7.z);
    glVertex3f(p8.x, p8.y, p8.z);

    // 왼쪽 면
    glVertex3f(p1.x, p1.y, p1.z);
    glVertex3f(p5.x, p5.y, p5.z);
    glVertex3f(p8.x, p8.y, p8.z);
    glVertex3f(p4.x, p4.y, p4.z);

    // 오른쪽 면
    glVertex3f(p2.x, p2.y, p2.z);
    glVertex3f(p6.x, p6.y, p6.z);
    glVertex3f(p7.x, p7.y, p7.z);
    glVertex3f(p3.x, p3.y, p3.z);

    // 위쪽 면
    glVertex3f(p4.x, p4.y, p4.z);
    glVertex3f(p8.x, p8.y, p8.z);
    glVertex3f(p7.x, p7.y, p7.z);
    glVertex3f(p3.x, p3.y, p3.z);

    // 아래쪽 면
    glVertex3f(p1.x, p1.y, p1.z);
    glVertex3f(p5.x, p5.y, p5.z);
    glVertex3f(p6.x, p6.y, p6.z);
    glVertex3f(p2.x, p2.y, p2.z);

    glEnd();
}

void renderScene(void) {
    static float lastFrameTime = 0.0f;
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    mat4 projection = Perspective(60.0, 1.0, 0.1, 100.0); // 투영 매트릭스 수정
    glUseProgram(p);
    checkGLError("glUseProgram");


    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    checkGLError("glBindBuffer");

    glEnableVertexAttribArray(0); // vPosition
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    checkGLError("vPosition");

    glEnableVertexAttribArray(1); // vNormal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(vertices_size));
    checkGLError("vNormal");

    glEnableVertexAttribArray(2); // vTexCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(vertices_size + normals_size));
    checkGLError("vTexCoord");

    glUniformMatrix4fv(glGetUniformLocation(p, "model"), 1, GL_TRUE, modelMatrix);
    glUniformMatrix4fv(glGetUniformLocation(p, "view"), 1, GL_TRUE, viewMatrix);

    glUniformMatrix4fv(glGetUniformLocation(p, "projection"), 1, GL_TRUE, projection);
    checkGLError("glUniformMatrix4fv");

    glUniform3fv(glGetUniformLocation(p, "lightPosition"), 1, lightPosition);

    renderMode = 0;
    glUniform1i(glGetUniformLocation(p, "renderMode"), renderMode);

    drawCube(lightPosition, 3.0, mat4(1.0f));

    renderMode = 1;
    glUniform1i(glGetUniformLocation(p, "renderMode"), renderMode);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseTexture);
    glUniform1i(glGetUniformLocation(p, "diffuseTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glUniform1i(glGetUniformLocation(p, "normalTexture"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, roughnessTexture);
    glUniform1i(glGetUniformLocation(p, "roughnessTexture"), 2);

    glDrawElements(GL_TRIANGLES, 3 * indicesNum, GL_UNSIGNED_INT, 0);
    glutSwapBuffers();
}
void changeSize(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

void init() {
    printf("Initializing buffers...\n");




    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices_size + normals_size + texture_size, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_size, vertices);
    glBufferSubData(GL_ARRAY_BUFFER, vertices_size, normals_size, vertex_normals);
    glBufferSubData(GL_ARRAY_BUFFER, vertices_size + normals_size, texture_size, texCoords);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);


    printf("Buffers initialized successfully.\n");

    p = createGLSLProgram("../phong.vert", NULL, "../phong.frag");
    if (!p) {
        printf("Failed to create shader program.\n");
        exit(1);
    }

    printf("Shader program created successfully.\n");

    glEnable(GL_DEPTH_TEST);
    glClearColor(25.0 / 255.0, 25.0 / 255.0, 113.0 / 255.0, 0.2);
}

// 카메라 업데이트: 모델의 위치만 따라가도록 설정
void updateCamera() {
    // 카메라의 시점과 타겟 설정 (모델 위치만 기준으로 함)
    vec4 eyePosition = vec4(modelPosition.x, modelPosition.y + 50.0, modelPosition.z + 50.0, 1.0); // 고정된 거리와 높이
    vec4 lookAtPosition = vec4(modelPosition, 1.0); // 모델의 현재 위치를 바라봄

    // 회전 없이 고정된 카메라 시점 설정
    viewMatrix = LookAt(eyePosition, lookAtPosition, vec4(0, 1, 0, 0));

    // 셰이더에 뷰 매트릭스 전송
    glUniformMatrix4fv(glGetUniformLocation(p, "view"), 1, GL_TRUE, viewMatrix);
}


// 모델이 지정된 방향을 바라보도록 회전
void rotateModelToFace(vec3 direction) {
    float targetAngle = atan2(direction.x, -direction.z) * 180.0 / M_PI; // 목표 각도 계산
    float angleDifference = fmod((targetAngle - modelRotationAngle) + 360.0f, 360.0f);
    if (angleDifference > 180.0f) angleDifference -= 360.0f;

    float stepAngle = modelRotationSpeed; // 고정 회전 속도
    if (fabs(angleDifference) < stepAngle) {
        modelRotationAngle = targetAngle; // 최종 회전 완료
    }
    else {
        modelRotationAngle += (angleDifference > 0 ? stepAngle : -stepAngle); // 점진적 회전
    }
}

// 모델 이동 (회전 후 이동)
void moveModel(vec3 direction) {
    static bool isRotating = false;

    // 방향을 먼저 회전시킴
    rotateModelToFace(direction);

    // 회전이 완료된 경우에만 이동
    if (!isRotating || fabs(atan2(direction.x, -direction.z) * 180.0 / M_PI - modelRotationAngle) < 0.01) {
        modelPosition += direction * modelSpeed;
        isRotating = false;
    }
    else {
        isRotating = true;
    }

    // 모델 및 카메라 업데이트
    modelMatrix = Translate(modelPosition) * RotateY(modelRotationAngle);
    updateCamera();

    glutPostRedisplay(); // 화면 갱신
}

// 키보드 입력 처리
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': // 위쪽으로 이동
        moveModel(vec3(0, 0, -1));
        break;
    case 's': // 아래쪽으로 이동
        moveModel(vec3(0, 0, 1));
        break;
    case 'a': // 왼쪽으로 이동
        moveModel(vec3(-1, 0, 0));
        break;
    case 'd': // 오른쪽으로 이동
        moveModel(vec3(1, 0, 0));
        break;
    }
}


int main(int argc, char** argv) {
    printf("Starting program...\n");
    readObjFile("Stone.obj");

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(600, 600);
    glutCreateWindow("COSE436 - Assignment 2");

    glewInit();
    if (glewIsSupported("GL_VERSION_3_3")) {
        printf("Ready for OpenGL 3.3\n");
    }
    else {
        printf("OpenGL 3.3 is not supported.\n");
        exit(1);
    }



    init();
    loadMTL("Stone.mtl");
    printf("Read OBJ and MTL files successfully.\n");
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutIdleFunc(renderScene);
    glutKeyboardFunc(keyboard);

    printf("Starting main loop...\n");
    glutMainLoop();

    return 0;
}