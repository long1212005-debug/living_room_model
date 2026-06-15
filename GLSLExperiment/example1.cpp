// Phong khach 3D tuong tac bang cube don vi
// Dieu khien:
// W A S D: di chuyen
// Chuot: xoay goc nhin
// E: tuong tac voi vat dang nhin
// L: bat/tat den chinh
// F: bat/tat den cay
// R: doi muc quat
// T: bat/tat TV
// C: tang gio
// V: giam gio
// ESC: thoat

#include "Angel.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>

#ifdef _WIN32
#include <GL/glu.h>
#endif

typedef vec4 point4;
typedef vec4 color4;

const int NumPoints = 36;

point4 points[NumPoints];
color4 colors[NumPoints];
vec4 normals[NumPoints];

point4 vertices[8];
color4 vertex_colors[8];

GLuint program;

GLuint modelView_loc;
GLuint projection_loc;
GLuint objectColor_loc;
GLuint brightness_loc;

GLuint mainLightPosition_loc;
GLuint lampLightPosition_loc;
GLuint mainLightOn_loc;
GLuint lampOn_loc;
GLuint shininess_loc;

GLuint tvLightPosition_loc;
GLuint tvOn_loc;

mat4 projection;
mat4 view;

// ======================= CAMERA FPS =======================

float cameraX = 5.0f;
float cameraY = 1.7f;
float cameraZ = 3.0f;

float yawAngle = -90.0f;
float pitchAngle = 0.0f;

bool keys[256];

int windowWidth = 1200;
int windowHeight = 800;

bool mouseLocked = true;
bool firstMouse = true;

const float PI = 3.1415926535f;

// ======================= TRANG THAI PHONG =======================

bool doorOpen = false;
float doorAngle = 0.0f;

bool mainLightOn = true;
bool lampOn = true;

bool tvOn = false;

int fanLevel = 0;
float fanAngle = 0.0f;

bool acOn = false;
float acFlapAngle = 0.0f;

bool curtainDown = true;
float curtainHeight = 1.45f;

bool drawerOpen = false;
float drawerOffset = 0.0f;

float gameHour = 18.0f;

float skyR = 0.55f;
float skyG = 0.78f;
float skyB = 1.0f;

// Do sang tong the theo gio trong ngay.
// Khong phu thuoc vao cong tac den chinh.
float roomBrightness = 1.0f;

// ======================= TARGET TUONG TAC =======================

enum InteractionTarget
{
    TARGET_NONE,
    TARGET_DOOR,
    TARGET_LIGHT_SWITCH,
    TARGET_FAN_SWITCH,
    TARGET_AC_SWITCH,
    TARGET_CURTAIN_SWITCH,
    TARGET_TV,
    TARGET_CLOCK,
    TARGET_DRAWER
};

InteractionTarget currentTarget = TARGET_NONE;

// ======================= CUBE CO BAN =======================

void initCube()
{
    vertices[0] = point4(-0.5, -0.5, 0.5, 1.0);
    vertices[1] = point4(-0.5, 0.5, 0.5, 1.0);
    vertices[2] = point4(0.5, 0.5, 0.5, 1.0);
    vertices[3] = point4(0.5, -0.5, 0.5, 1.0);
    vertices[4] = point4(-0.5, -0.5, -0.5, 1.0);
    vertices[5] = point4(-0.5, 0.5, -0.5, 1.0);
    vertices[6] = point4(0.5, 0.5, -0.5, 1.0);
    vertices[7] = point4(0.5, -0.5, -0.5, 1.0);

    for (int i = 0; i < 8; i++) {
        vertex_colors[i] = color4(1.0, 1.0, 1.0, 1.0);
    }
}

int Index = 0;

void quad(int a, int b, int c, int d, vec4 normal)
{
    colors[Index] = vertex_colors[a];
    points[Index] = vertices[a];
    normals[Index] = normal;
    Index++;

    colors[Index] = vertex_colors[b];
    points[Index] = vertices[b];
    normals[Index] = normal;
    Index++;

    colors[Index] = vertex_colors[c];
    points[Index] = vertices[c];
    normals[Index] = normal;
    Index++;

    colors[Index] = vertex_colors[a];
    points[Index] = vertices[a];
    normals[Index] = normal;
    Index++;

    colors[Index] = vertex_colors[c];
    points[Index] = vertices[c];
    normals[Index] = normal;
    Index++;

    colors[Index] = vertex_colors[d];
    points[Index] = vertices[d];
    normals[Index] = normal;
    Index++;
}

void makeColorCube(void)
{
    quad(1, 0, 3, 2, vec4(0.0, 0.0, 1.0, 0.0));
    quad(2, 3, 7, 6, vec4(1.0, 0.0, 0.0, 0.0));
    quad(3, 0, 4, 7, vec4(0.0, -1.0, 0.0, 0.0));
    quad(6, 5, 1, 2, vec4(0.0, 1.0, 0.0, 0.0));
    quad(4, 5, 6, 7, vec4(0.0, 0.0, -1.0, 0.0));
    quad(5, 4, 0, 1, vec4(-1.0, 0.0, 0.0, 0.0));
}

void generateGeometry(void)
{
    initCube();
    Index = 0;
    makeColorCube();
}

// ======================= GPU BUFFER + SHADER =======================

void initGPUBuffers(void)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(points) + sizeof(colors) + sizeof(normals),
        NULL,
        GL_STATIC_DRAW
    );

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), sizeof(normals), normals);
}

void shaderSetup(void)
{
    program = InitShader("vshader1.glsl", "fshader1.glsl");
    glUseProgram(program);

    GLuint loc_vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(loc_vPosition);
    glVertexAttribPointer(loc_vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    GLuint loc_vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(loc_vColor);
    glVertexAttribPointer(loc_vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

    GLuint loc_vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(loc_vNormal);
    glVertexAttribPointer(
        loc_vNormal,
        4,
        GL_FLOAT,
        GL_FALSE,
        0,
        BUFFER_OFFSET(sizeof(points) + sizeof(colors))
    );

    modelView_loc = glGetUniformLocation(program, "ModelView");
    projection_loc = glGetUniformLocation(program, "Projection");
    objectColor_loc = glGetUniformLocation(program, "objectColor");
    brightness_loc = glGetUniformLocation(program, "brightness");

    mainLightPosition_loc = glGetUniformLocation(program, "mainLightPosition");
    lampLightPosition_loc = glGetUniformLocation(program, "lampLightPosition");
    mainLightOn_loc = glGetUniformLocation(program, "mainLightOn");
    lampOn_loc = glGetUniformLocation(program, "lampOn");
    shininess_loc = glGetUniformLocation(program, "shininess");
    tvLightPosition_loc =
        glGetUniformLocation(program, "tvLightPosition");

    tvOn_loc = glGetUniformLocation(program, "tvOn");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.14f, 0.15f, 0.18f, 1.0f);
}

// ======================= HAM VE CUBE =======================

void sendLightUniforms()
{
    point4 mainLightEye =
        view * point4(5.0f, 3.80f, 4.0f, 1.0f);

    point4 lampLightEye =
        view * point4(1.10f, 1.65f, 5.35f, 1.0f);

    point4 tvLightEye =
        view * point4(7.0f, 1.65f, 2.5f, 1.0f);

    glUniform4fv(
        mainLightPosition_loc,
        1,
        mainLightEye
    );

    glUniform4fv(
        lampLightPosition_loc,
        1,
        lampLightEye
    );

    glUniform4fv(
        tvLightPosition_loc,
        1,
        tvLightEye
    );

    glUniform1i(
        mainLightOn_loc,
        mainLightOn ? 1 : 0
    );

    glUniform1i(
        lampOn_loc,
        lampOn ? 1 : 0
    );

    glUniform1i(
        tvOn_loc,
        tvOn ? 1 : 0
    );

    glUniform1f(
        shininess_loc,
        36.0f
    );
}

void drawCube(float x, float y, float z,
    float sx, float sy, float sz,
    color4 color)
{
    mat4 model = Translate(x, y, z) * Scale(sx, sy, sz);
    mat4 modelView = view * model;

    glUniformMatrix4fv(modelView_loc, 1, GL_TRUE, modelView);
    glUniformMatrix4fv(projection_loc, 1, GL_TRUE, projection);
    glUniform4fv(objectColor_loc, 1, color);
    glUniform1f(brightness_loc, roomBrightness);

    sendLightUniforms();
    glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

void drawCubeModel(mat4 model, color4 color)
{
    mat4 modelView = view * model;

    glUniformMatrix4fv(modelView_loc, 1, GL_TRUE, modelView);
    glUniformMatrix4fv(projection_loc, 1, GL_TRUE, projection);
    glUniform4fv(objectColor_loc, 1, color);
    glUniform1f(brightness_loc, roomBrightness);

    sendLightUniforms();
    glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

// ======================= MAU SAC =======================

color4 whiteWall() { return color4(0.86f, 0.86f, 0.82f, 1.0f); }
color4 creamWall() { return color4(0.86f, 0.82f, 0.72f, 1.0f); }
color4 floorWood() { return color4(0.58f, 0.36f, 0.18f, 1.0f); }
color4 rugColor() { return color4(0.70f, 0.67f, 0.56f, 1.0f); }
color4 darkWood() { return color4(0.38f, 0.22f, 0.10f, 1.0f); }
color4 wood2() { return color4(0.55f, 0.32f, 0.14f, 1.0f); }
color4 sofaBlue() { return color4(0.20f, 0.42f, 0.55f, 1.0f); }
color4 sofaBlue2() { return color4(0.12f, 0.28f, 0.38f, 1.0f); }
color4 blackColor() { return color4(0.08f, 0.08f, 0.09f, 1.0f); }
color4 greenColor() { return color4(0.28f, 0.62f, 0.22f, 1.0f); }
color4 greenColor2() { return color4(0.18f, 0.45f, 0.16f, 1.0f); }
color4 lightYellow() { return color4(1.0f, 0.78f, 0.38f, 1.0f); }
color4 creamPot() { return color4(0.86f, 0.80f, 0.64f, 1.0f); }

// ======================= PHONG - 4 TUONG =======================

void drawRoom()
{
    drawCube(5.0f, -0.05f, 4.0f, 10.0f, 0.1f, 8.0f, floorWood());

    drawCube(5.0f, 2.0f, 0.0f, 10.0f, 4.0f, 0.1f, whiteWall());
    drawCube(0.0f, 2.0f, 4.0f, 0.1f, 4.0f, 8.0f, whiteWall());

    drawCube(10.0f, 2.0f, 2.75f, 0.1f, 4.0f, 5.5f, creamWall());
    drawCube(10.0f, 2.0f, 7.65f, 0.1f, 4.0f, 0.7f, creamWall());
    drawCube(10.0f, 3.55f, 6.4f, 0.1f, 0.9f, 1.8f, creamWall());

    drawCube(5.0f, 2.0f, 8.0f, 10.0f, 4.0f, 0.1f, creamWall());

    drawCube(5.0f, 4.05f, 4.0f, 10.0f, 0.1f, 8.0f, color4(0.70f, 0.60f, 0.46f, 1.0f));
}

// ======================= CUA SO + REM =======================

void drawWindow()
{
    drawCube(0.052f, 2.25f, 2.50f, 0.06f, 1.62f, 2.98f, color4(skyR, skyG, skyB, 1.0f));
    drawCube(0.040f, 2.25f, 2.50f, 0.035f, 1.48f, 2.82f, color4(skyR * 0.92f + 0.08f, skyG * 0.92f + 0.08f, skyB, 1.0f));

    drawCube(0.12f, 2.25f, 0.95f, 0.16f, 1.82f, 0.16f, darkWood());
    drawCube(0.12f, 2.25f, 4.05f, 0.16f, 1.82f, 0.16f, darkWood());
    drawCube(0.12f, 3.16f, 2.50f, 0.16f, 0.16f, 3.20f, darkWood());
    drawCube(0.12f, 1.34f, 2.50f, 0.16f, 0.16f, 3.20f, darkWood());

    drawCube(0.13f, 2.25f, 2.50f, 0.17f, 1.72f, 0.13f, darkWood());
    drawCube(0.13f, 2.25f, 2.50f, 0.17f, 0.10f, 3.02f, darkWood());

    drawCube(0.28f, 1.15f, 2.50f, 0.42f, 0.08f, 3.25f, color4(0.80f, 0.74f, 0.62f, 1.0f));
}

void drawCurtain()
{
    drawCube(0.20f, 3.36f, 2.50f, 0.14f, 0.14f, 3.35f, color4(0.28f, 0.18f, 0.08f, 1.0f));

    float centerY = 3.16f - curtainHeight / 2.0f;

    drawCube(0.19f, centerY, 1.72f, 0.08f, curtainHeight, 1.42f, color4(0.64f, 0.48f, 0.32f, 1.0f));
    drawCube(0.19f, centerY, 3.28f, 0.08f, curtainHeight, 1.42f, color4(0.64f, 0.48f, 0.32f, 1.0f));

    drawCube(0.16f, centerY, 1.20f, 0.05f, curtainHeight, 0.08f, color4(0.46f, 0.32f, 0.20f, 1.0f));
    drawCube(0.16f, centerY, 1.72f, 0.05f, curtainHeight, 0.08f, color4(0.46f, 0.32f, 0.20f, 1.0f));
    drawCube(0.16f, centerY, 2.25f, 0.05f, curtainHeight, 0.08f, color4(0.46f, 0.32f, 0.20f, 1.0f));
    drawCube(0.16f, centerY, 2.75f, 0.05f, curtainHeight, 0.08f, color4(0.46f, 0.32f, 0.20f, 1.0f));
    drawCube(0.16f, centerY, 3.28f, 0.05f, curtainHeight, 0.08f, color4(0.46f, 0.32f, 0.20f, 1.0f));
    drawCube(0.16f, centerY, 3.80f, 0.05f, curtainHeight, 0.08f, color4(0.46f, 0.32f, 0.20f, 1.0f));
}

void drawCurtainSwitch()
{
    float x = 0.18f;
    float y = 1.55f;
    float z = 4.45f;

    drawCube(x, y, z, 0.08f, 0.46f, 0.28f, color4(0.82f, 0.78f, 0.68f, 1.0f));

    drawCube(
        x - 0.04f, y, z,
        0.06f, 0.28f, 0.16f,
        curtainDown ? color4(0.58f, 0.42f, 0.26f, 1.0f) : color4(0.24f, 0.44f, 0.72f, 1.0f)
    );
}

// ======================= CUA RA VAO =======================

void drawDoor()
{
    drawCube(9.95f, 1.6f, 5.55f, 0.15f, 3.2f, 0.15f, darkWood());
    drawCube(9.95f, 1.6f, 7.25f, 0.15f, 3.2f, 0.15f, darkWood());
    drawCube(9.95f, 3.2f, 6.40f, 0.15f, 0.15f, 1.85f, darkWood());

    mat4 model =
        Translate(9.90f, 1.55f, 5.55f) *
        RotateY(-doorAngle) *
        Translate(0.0f, 0.0f, 0.85f) *
        Scale(0.12f, 3.0f, 1.70f);

    drawCubeModel(model, color4(0.48f, 0.28f, 0.14f, 1.0f));

    mat4 knob =
        Translate(9.72f, 1.55f, 6.92f) *
        RotateY(-doorAngle) *
        Scale(0.10f, 0.14f, 0.14f);

    drawCubeModel(knob, color4(0.90f, 0.82f, 0.55f, 1.0f));
}

// ======================= SOFA  =======================

void drawSofa()
{
    drawCube(4.25f, 0.02f, 5.85f, 4.80f, 0.03f, 2.70f, rugColor());

    // Sofa dai
    drawCube(4.25f, 0.40f, 6.78f, 4.05f, 0.50f, 1.02f, sofaBlue2());
    drawCube(4.25f, 0.72f, 6.56f, 3.80f, 0.20f, 0.88f, sofaBlue());
    drawCube(4.25f, 1.02f, 7.18f, 4.10f, 0.88f, 0.26f, sofaBlue());

    // Tay ghe phai
    drawCube(6.32f, 0.76f, 6.70f, 0.28f, 0.80f, 1.08f, sofaBlue());

    // Sofa chu L ben trai
    drawCube(2.58f, 0.40f, 5.24f, 1.04f, 0.50f, 2.05f, sofaBlue2());
    drawCube(2.58f, 0.72f, 5.24f, 0.88f, 0.20f, 1.86f, sofaBlue());
    drawCube(2.02f, 1.02f, 5.24f, 0.26f, 0.88f, 2.10f, sofaBlue());

    // Tay ghe trai o dau chu L
    drawCube(2.58f, 0.76f, 4.18f, 1.04f, 0.80f, 0.26f, sofaBlue());

    // Goc noi nho, khong con phan thua bi chong
    drawCube(2.92f, 0.72f, 6.42f, 0.62f, 0.20f, 0.46f, sofaBlue());

    // Chan ghe
    drawCube(2.70f, 0.12f, 6.32f, 0.22f, 0.24f, 0.22f, blackColor());
    drawCube(5.85f, 0.12f, 6.32f, 0.22f, 0.24f, 0.22f, blackColor());
    drawCube(2.25f, 0.12f, 4.55f, 0.22f, 0.24f, 0.22f, blackColor());

    // Goi
    drawCube(3.26f, 1.12f, 6.98f, 0.52f, 0.42f, 0.12f, color4(0.78f, 0.64f, 0.42f, 1.0f));
    drawCube(5.06f, 1.12f, 6.98f, 0.52f, 0.42f, 0.12f, color4(0.64f, 0.50f, 0.34f, 1.0f));
    drawCube(2.18f, 1.06f, 5.12f, 0.12f, 0.40f, 0.50f, color4(0.54f, 0.68f, 0.72f, 1.0f));
}

// ======================= BAN TRA =======================

void drawCoffeeTable()
{
    drawCube(4.65f, 0.36f, 4.80f, 2.10f, 0.16f, 1.25f, darkWood());
    drawCube(3.78f, 0.08f, 4.32f, 0.16f, 0.42f, 0.16f, darkWood());
    drawCube(5.52f, 0.08f, 4.32f, 0.16f, 0.42f, 0.16f, darkWood());
    drawCube(3.78f, 0.08f, 5.28f, 0.16f, 0.42f, 0.16f, darkWood());
    drawCube(5.52f, 0.08f, 5.28f, 0.16f, 0.42f, 0.16f, darkWood());

    drawCube(4.35f, 0.50f, 4.65f, 0.44f, 0.08f, 0.28f, color4(0.62f, 0.22f, 0.16f, 1.0f));
    drawCube(4.45f, 0.58f, 4.72f, 0.32f, 0.06f, 0.22f, color4(0.16f, 0.44f, 0.68f, 1.0f));
}

// ======================= TV + KE + NGAN KEO RONG =======================

void drawTVArea()
{
    // Khung ke TV
    drawCube(7.0f, 0.72f, 0.52f, 3.25f, 0.16f, 0.78f, wood2());
    drawCube(7.0f, 0.18f, 0.52f, 3.25f, 0.16f, 0.78f, darkWood());
    drawCube(7.0f, 0.45f, 0.18f, 3.25f, 0.55f, 0.12f, color4(0.30f, 0.17f, 0.08f, 1.0f));

    drawCube(5.35f, 0.45f, 0.52f, 0.12f, 0.55f, 0.78f, darkWood());
    drawCube(8.65f, 0.45f, 0.52f, 0.12f, 0.55f, 0.78f, darkWood());

    drawCube(6.45f, 0.45f, 0.52f, 0.08f, 0.50f, 0.72f, darkWood());
    drawCube(7.55f, 0.45f, 0.52f, 0.08f, 0.50f, 0.72f, darkWood());

    drawCube(5.90f, 0.45f, 0.92f, 0.85f, 0.42f, 0.08f, color4(0.58f, 0.32f, 0.14f, 1.0f));
    drawCube(8.10f, 0.45f, 0.92f, 0.85f, 0.42f, 0.08f, color4(0.58f, 0.32f, 0.14f, 1.0f));

    drawCube(7.00f, 0.45f, 0.56f, 0.96f, 0.48f, 0.42f, color4(0.22f, 0.12f, 0.06f, 1.0f));

    // drawerOffset = 0 => dong sat vao ke
    // drawerOffset > 0 => keo ra
    float dz = drawerOffset;

    // Day ngan keo
    drawCube(
        7.00f, 0.28f, 0.72f + dz,
        0.82f, 0.06f, 0.42f,
        color4(0.48f, 0.28f, 0.12f, 1.0f)
    );

    // Thanh trai
    drawCube(
        6.60f, 0.45f, 0.72f + dz,
        0.06f, 0.28f, 0.42f,
        color4(0.56f, 0.34f, 0.16f, 1.0f)
    );

    // Thanh phai
    drawCube(
        7.40f, 0.45f, 0.72f + dz,
        0.06f, 0.28f, 0.42f,
        color4(0.56f, 0.34f, 0.16f, 1.0f)
    );

    // Thanh sau
    drawCube(
        7.00f, 0.45f, 0.50f + dz,
        0.82f, 0.28f, 0.06f,
        color4(0.56f, 0.34f, 0.16f, 1.0f)
    );

    // Mat truoc ngan keo 
    drawCube(
        7.00f, 0.45f, 0.96f + dz,
        0.98f, 0.42f, 0.08f,
        color4(0.78f, 0.52f, 0.28f, 1.0f)
    );

    // Tay nam
    drawCube(
        7.00f, 0.45f, 1.02f + dz,
        0.20f, 0.06f, 0.03f,
        color4(0.86f, 0.74f, 0.48f, 1.0f)
    );

    // TV
    if (tvOn) {
        drawCube(7.0f, 1.65f, 0.22f, 2.25f, 1.22f, 0.08f, color4(0.08f, 0.28f, 0.70f, 1.0f));
    }
    else {
        drawCube(7.0f, 1.65f, 0.22f, 2.25f, 1.22f, 0.08f, blackColor());
    }

   

    // Chân trái
    mat4 leftLeg =
        Translate(6.90f, 0.9f, 0.22f) *
        RotateZ(-30.0f) *
        Scale(0.06f, 0.40f, 0.06f);

    drawCubeModel(leftLeg, blackColor());

    // Chân phải
    mat4 rightLeg =
        Translate(7.10f, 0.9f, 0.22f) *
        RotateZ(30.0f) *
        Scale(0.06f, 0.40f, 0.06f);

    drawCubeModel(rightLeg, blackColor());
}

// ======================= TU SACH =======================

void drawBookshelf()
{
    float x = 3.25f;
    float z = 0.42f;

    drawCube(x, 1.25f, z, 0.90f, 2.30f, 0.55f, darkWood());

    drawCube(x - 0.42f, 1.25f, z, 0.08f, 2.30f, 0.60f, color4(0.28f, 0.16f, 0.08f, 1.0f));
    drawCube(x + 0.42f, 1.25f, z, 0.08f, 2.30f, 0.60f, color4(0.28f, 0.16f, 0.08f, 1.0f));

    drawCube(x, 0.55f, z + 0.28f, 0.85f, 0.08f, 0.12f, floorWood());
    drawCube(x, 1.20f, z + 0.28f, 0.85f, 0.08f, 0.12f, floorWood());
    drawCube(x, 1.85f, z + 0.28f, 0.85f, 0.08f, 0.12f, floorWood());

    drawCube(x - 0.25f, 0.85f, z + 0.30f, 0.12f, 0.42f, 0.18f, color4(0.65f, 0.16f, 0.12f, 1.0f));
    drawCube(x - 0.08f, 0.85f, z + 0.30f, 0.12f, 0.42f, 0.18f, color4(0.12f, 0.24f, 0.60f, 1.0f));
    drawCube(x + 0.10f, 0.85f, z + 0.30f, 0.12f, 0.42f, 0.18f, color4(0.70f, 0.62f, 0.18f, 1.0f));
    drawCube(x + 0.28f, 0.85f, z + 0.30f, 0.12f, 0.42f, 0.18f, color4(0.18f, 0.48f, 0.22f, 1.0f));

    drawCube(x - 0.20f, 1.50f, z + 0.30f, 0.16f, 0.40f, 0.18f, color4(0.72f, 0.34f, 0.14f, 1.0f));
    drawCube(x + 0.05f, 1.50f, z + 0.30f, 0.16f, 0.40f, 0.18f, color4(0.20f, 0.54f, 0.62f, 1.0f));
    drawCube(x + 0.28f, 1.50f, z + 0.30f, 0.16f, 0.40f, 0.18f, color4(0.58f, 0.22f, 0.50f, 1.0f));

    drawCube(x, 2.55f, z, 0.30f, 0.22f, 0.30f, creamPot());
    drawCube(x, 2.82f, z, 0.14f, 0.34f, 0.14f, greenColor2());
    drawCube(x - 0.16f, 2.78f, z, 0.22f, 0.14f, 0.16f, greenColor());
    drawCube(x + 0.16f, 2.78f, z, 0.22f, 0.14f, 0.16f, greenColor());
}

// ======================= DONG HO KHONG SO =======================

void drawClock()
{
    float cx = 2.10f;
    float cy = 2.65f;
    float zf = 0.08f;

    int hour = (int)gameHour;
    float minute = (gameHour - hour) * 60.0f;

    float minuteAngle = minute * 6.0f;
    float hourAngle = (hour % 12) * 30.0f + minute * 0.5f;

    drawCube(cx, cy, zf, 1.05f, 1.05f, 0.08f, color4(0.78f, 0.62f, 0.40f, 1.0f));
    drawCube(cx, cy, 0.03f, 0.86f, 0.86f, 0.03f, color4(0.88f, 0.84f, 0.72f, 1.0f));

    for (int i = 0; i < 12; i++) {
        float ang = (90.0f - i * 30.0f) * PI / 180.0f;
        float rx = cx + cos(ang) * 0.68f;
        float ry = cy + sin(ang) * 0.68f;

        if (i % 3 == 0) {
            drawCube(rx, ry, 0.05f, 0.10f, 0.10f, 0.025f, blackColor());
        }
        else {
            drawCube(rx, ry, 0.05f, 0.06f, 0.06f, 0.025f, color4(0.28f, 0.22f, 0.16f, 1.0f));
        }
    }

    drawCube(cx, cy, 0.015f, 0.12f, 0.12f, 0.04f, blackColor());

    mat4 hourHand =
        Translate(cx, cy, 0.01f) *
        RotateZ(-hourAngle) *
        Translate(0.0f, 0.16f, 0.0f) *
        Scale(0.055f, 0.34f, 0.025f);

    drawCubeModel(hourHand, blackColor());

    mat4 minuteHand =
        Translate(cx, cy, 0.0f) *
        RotateZ(-minuteAngle) *
        Translate(0.0f, 0.24f, 0.0f) *
        Scale(0.035f, 0.50f, 0.02f);

    drawCubeModel(minuteHand, color4(0.12f, 0.12f, 0.12f, 1.0f));
}

// ======================= CAY CANH GOC TV =======================

void drawCornerPlant()
{
    float x = 9.05f;
    float z = 0.82f;

    // ===== Chậu cây =====

    // Đế
    drawCube(x, 0.18f, z,
        0.55f, 0.12f, 0.55f,
        color4(0.75f, 0.70f, 0.62f, 1.0f));

    // Thân chậu
    drawCube(x, 0.42f, z,
        0.45f, 0.36f, 0.45f,
        creamPot());

    // Miệng chậu loe
    drawCube(x, 0.62f, z,
        0.55f, 0.08f, 0.55f,
        color4(0.88f, 0.82f, 0.72f, 1.0f));

    // Đất
    drawCube(x, 0.66f, z,
        0.42f, 0.05f, 0.42f,
        color4(0.20f, 0.12f, 0.06f, 1.0f));

    // ===== Thân cây =====

    drawCube(x, 1.05f, z,
        0.14f, 0.90f, 0.14f,
        color4(0.42f, 0.24f, 0.10f, 1.0f));

    // ===== Tán lá dưới =====
    drawCube(x, 1.30f, z,
        1.00f, 0.28f, 0.60f,
        greenColor2());

    drawCube(x - 0.45f, 1.22f, z,
        0.40f, 0.24f, 0.40f,
        greenColor());

    drawCube(x + 0.45f, 1.22f, z,
        0.40f, 0.24f, 0.40f,
        greenColor());

    drawCube(x, 1.22f, z - 0.42f,
        0.40f, 0.24f, 0.40f,
        greenColor());

    drawCube(x, 1.22f, z + 0.42f,
        0.40f, 0.24f, 0.40f,
        greenColor());


    // ===== Tán lá giữa =====
    drawCube(x, 1.55f, z,
        0.85f, 0.26f, 0.55f,
        greenColor2());

    drawCube(x - 0.30f, 1.50f, z + 0.12f,
        0.35f, 0.20f, 0.35f,
        greenColor());

    drawCube(x + 0.30f, 1.50f, z - 0.12f,
        0.35f, 0.20f, 0.35f,
        greenColor());


    // ===== Tán lá trên =====
    drawCube(x, 1.78f, z,
        0.65f, 0.24f, 0.45f,
        color4(0.16f, 0.55f, 0.20f, 1.0f));


    // ===== Đỉnh =====
    drawCube(x, 1.95f, z,
        0.38f, 0.18f, 0.28f,
        color4(0.20f, 0.65f, 0.24f, 1.0f));
}

// ======================= TRANG TRI THEM =======================

void drawDecorBlocks()
{
    drawCube(4.9f, 2.45f, 7.95f, 1.6f, 1.0f, 0.03f, color4(0.68f, 0.56f, 0.42f, 1.0f));
    drawCube(4.4f, 2.45f, 7.92f, 0.35f, 0.35f, 0.02f, color4(0.52f, 0.20f, 0.15f, 1.0f));
    drawCube(4.9f, 2.75f, 7.92f, 0.45f, 0.28f, 0.02f, color4(0.16f, 0.42f, 0.62f, 1.0f));
    drawCube(5.45f, 2.28f, 7.92f, 0.35f, 0.40f, 0.02f, color4(0.18f, 0.55f, 0.28f, 1.0f));
}

// ======================= DEN + QUAT =======================

void drawCeilingLight()
{
    if (mainLightOn) {
        drawCube(5.0f, 3.86f, 4.0f, 0.88f, 0.14f, 0.88f, lightYellow());
    }
    else {
        drawCube(5.0f, 3.86f, 4.0f, 0.88f, 0.14f, 0.88f, color4(0.30f, 0.28f, 0.24f, 1.0f));
    }
}

void drawFloorLamp()
{
    float x = 1.10f;
    float z = 5.35f;

    drawCube(x, 0.75f, z, 0.12f, 1.45f, 0.12f, blackColor());
    drawCube(x, 0.06f, z, 0.48f, 0.12f, 0.48f, blackColor());

    if (lampOn) {
        drawCube(x, 1.65f, z, 0.56f, 0.50f, 0.56f, lightYellow());
    }
    else {
        drawCube(x, 1.65f, z, 0.56f, 0.50f, 0.56f, color4(0.30f, 0.28f, 0.24f, 1.0f));
    }

    drawCube(x, 1.92f, z, 0.62f, 0.06f, 0.62f, color4(0.30f, 0.20f, 0.10f, 1.0f));
    drawCube(x, 1.38f, z, 0.62f, 0.06f, 0.62f, color4(0.30f, 0.20f, 0.10f, 1.0f));
}

void drawFanBlade(float angle)
{
    mat4 model =
        Translate(5.0f, 3.55f, 4.0f) *
        RotateY(angle + fanAngle) *
        Translate(0.82f, 0.0f, 0.0f) *
        Scale(1.55f, 0.08f, 0.24f);

    drawCubeModel(model, darkWood());
}

void drawCeilingFan()
{
    drawCube(5.0f, 3.78f, 4.0f, 0.16f, 0.46f, 0.16f, blackColor());
    drawCube(5.0f, 3.49f, 4.0f, 0.54f, 0.24f, 0.54f, color4(0.16f, 0.16f, 0.16f, 1.0f));

    drawFanBlade(0.0f);
    drawFanBlade(90.0f);
    drawFanBlade(180.0f);
    drawFanBlade(270.0f);
}

// ======================= DIEU HOA =======================

void drawAC()
{
    drawCube(8.35f, 3.28f, 0.16f, 1.85f, 0.50f, 0.28f, color4(0.58f, 0.76f, 0.84f, 1.0f));
    drawCube(8.35f, 3.32f, 0.02f, 1.65f, 0.34f, 0.04f, color4(0.78f, 0.90f, 0.94f, 1.0f));
    drawCube(8.35f, 3.57f, 0.04f, 1.75f, 0.08f, 0.06f, color4(0.32f, 0.50f, 0.60f, 1.0f));
    drawCube(8.35f, 3.08f, 0.02f, 1.55f, 0.08f, 0.05f, color4(0.14f, 0.20f, 0.24f, 1.0f));

    drawCube(
        9.05f, 3.32f, -0.02f,
        0.14f, 0.05f, 0.03f,
        acOn ? color4(0.10f, 0.85f, 0.30f, 1.0f) : color4(0.28f, 0.28f, 0.28f, 1.0f)
    );

    mat4 flap =
        Translate(8.35f, 3.03f, 0.11f) *
        RotateX(acFlapAngle) *
        Translate(0.0f, -0.03f, 0.08f) *
        Scale(1.58f, 0.06f, 0.18f);

    drawCubeModel(flap, acOn ? color4(0.36f, 0.60f, 0.72f, 1.0f) : color4(0.58f, 0.70f, 0.76f, 1.0f));
}

// ======================= BANG CONG TAC CHUNG =======================

void drawSwitchBoard()
{
    drawCube(9.92f, 1.58f, 4.82f, 0.08f, 1.05f, 1.22f, color4(0.80f, 0.78f, 0.68f, 1.0f));

    color4 border = color4(0.26f, 0.26f, 0.24f, 1.0f);

    drawCube(9.91f, 2.10f, 4.82f, 0.09f, 0.05f, 1.25f, border);
    drawCube(9.91f, 1.06f, 4.82f, 0.09f, 0.05f, 1.25f, border);
    drawCube(9.91f, 1.58f, 4.20f, 0.09f, 1.05f, 0.05f, border);
    drawCube(9.91f, 1.58f, 5.44f, 0.09f, 1.05f, 0.05f, border);

    color4 lightButton = mainLightOn ? color4(0.90f, 0.70f, 0.16f, 1.0f) : color4(0.28f, 0.28f, 0.28f, 1.0f);
    color4 acButton = acOn ? color4(0.16f, 0.62f, 0.30f, 1.0f) : color4(0.28f, 0.28f, 0.28f, 1.0f);

    color4 fanButton;
    if (fanLevel == 0) fanButton = color4(0.28f, 0.28f, 0.28f, 1.0f);
    else if (fanLevel == 1) fanButton = color4(0.24f, 0.56f, 0.78f, 1.0f);
    else if (fanLevel == 2) fanButton = color4(0.16f, 0.62f, 0.28f, 1.0f);
    else fanButton = color4(0.75f, 0.25f, 0.16f, 1.0f);

    drawCube(9.84f, 1.78f, 4.55f, 0.08f, 0.28f, 0.22f, lightButton);
    drawCube(9.84f, 1.78f, 5.09f, 0.08f, 0.28f, 0.22f, fanButton);
    drawCube(9.84f, 1.32f, 4.82f, 0.08f, 0.28f, 0.26f, acButton);

    color4 active = color4(1.0f, 0.88f, 0.22f, 1.0f);
    color4 inactive = color4(0.42f, 0.42f, 0.42f, 1.0f);

    drawCube(9.82f, 1.98f, 4.99f, 0.05f, 0.05f, 0.06f, fanLevel >= 1 ? active : inactive);
    drawCube(9.82f, 1.98f, 5.09f, 0.05f, 0.05f, 0.06f, fanLevel >= 2 ? active : inactive);
    drawCube(9.82f, 1.98f, 5.19f, 0.05f, 0.05f, 0.06f, fanLevel >= 3 ? active : inactive);
}

// ======================= UPDATE NGAY DEM =======================

void updateDayNight()
{
    if (gameHour >= 6.0f && gameHour < 12.0f) {
        skyR = 0.62f;
        skyG = 0.80f;
        skyB = 1.00f;
        roomBrightness = 0.96f;
    }
    else if (gameHour >= 12.0f && gameHour < 17.0f) {
        skyR = 0.55f;
        skyG = 0.76f;
        skyB = 1.00f;
        roomBrightness = 1.00f;
    }
    else if (gameHour >= 17.0f && gameHour < 19.0f) {
        skyR = 0.92f;
        skyG = 0.50f;
        skyB = 0.28f;
        roomBrightness = 0.82f;
    }
    else {
        skyR = 0.08f;
        skyG = 0.10f;
        skyB = 0.20f;
        roomBrightness = 0.42f;
    }

    if (lampOn) {
        roomBrightness += 0.06f;
    }

    if (roomBrightness > 1.05f) {
        roomBrightness = 1.05f;
    }
}

// ======================= UPDATE ANIMATION =======================

void updateDoor()
{
    if (doorOpen && doorAngle < 95.0f) doorAngle += 2.0f;
    if (!doorOpen && doorAngle > 0.0f) doorAngle -= 2.0f;
}

void updateFan()
{
    if (fanLevel == 1) fanAngle += 1.5f;
    else if (fanLevel == 2) fanAngle += 4.0f;
    else if (fanLevel == 3) fanAngle += 8.0f;

    if (fanAngle >= 360.0f) fanAngle -= 360.0f;
}

void updateCurtain()
{
    float target = curtainDown ? 1.45f : 0.22f;

    if (curtainHeight < target) curtainHeight += 0.02f;
    if (curtainHeight > target) curtainHeight -= 0.02f;

    if (curtainHeight < 0.22f) curtainHeight = 0.22f;
    if (curtainHeight > 1.45f) curtainHeight = 1.45f;
}

void updateAC()
{
    float target = acOn ? 28.0f : 0.0f;

    if (acFlapAngle < target) acFlapAngle += 1.0f;
    if (acFlapAngle > target) acFlapAngle -= 1.0f;
}

void updateDrawerAnim()
{
    float target = drawerOpen ? 0.22f : 0.0f;

    if (drawerOffset < target) drawerOffset += 0.01f;
    if (drawerOffset > target) drawerOffset -= 0.01f;
}

// ======================= VA CHAM =======================

bool isInsideBox(float x, float z, float minX, float maxX, float minZ, float maxZ)
{
    return x >= minX && x <= maxX && z >= minZ && z <= maxZ;
}

bool canMoveTo(float x, float z)
{
    if (x < 0.35f || x > 9.55f || z < 0.35f || z > 7.65f) {
        if (!(doorOpen && x <= 10.7f && x >= 9.45f && z >= 5.55f && z <= 7.25f)) {
            return false;
        }
    }

    if (isInsideBox(x, z, 1.95f, 6.60f, 6.05f, 7.45f)) return false;
    if (isInsideBox(x, z, 1.85f, 3.20f, 4.20f, 6.10f)) return false;

    if (isInsideBox(x, z, 3.40f, 5.90f, 4.05f, 5.60f)) return false;

    if (isInsideBox(x, z, 5.20f, 8.80f, 0.10f, 1.30f)) return false;

    if (isInsideBox(x, z, 2.70f, 3.85f, 0.10f, 0.95f)) return false;

    if (isInsideBox(x, z, 8.55f, 9.45f, 0.35f, 1.30f)) return false;

    if (isInsideBox(x, z, 0.75f, 1.45f, 5.00f, 5.75f)) return false;

    if (isInsideBox(x, z, 0.05f, 0.45f, 4.25f, 4.65f)) return false;

    if (!doorOpen && x > 9.45f && z >= 5.55f && z <= 7.25f) return false;

    return true;
}

void updateCamera()
{
    float speed = 0.018f;

    float yawRad = yawAngle * PI / 180.0f;
    float dirX = cos(yawRad);
    float dirZ = sin(yawRad);

    float rightX = cos(yawRad + PI / 2.0f);
    float rightZ = sin(yawRad + PI / 2.0f);

    float nextX = cameraX;
    float nextZ = cameraZ;

    if (keys['w'] || keys['W']) {
        nextX += dirX * speed;
        nextZ += dirZ * speed;
    }

    if (keys['s'] || keys['S']) {
        nextX -= dirX * speed;
        nextZ -= dirZ * speed;
    }

    if (keys['a'] || keys['A']) {
        nextX -= rightX * speed;
        nextZ -= rightZ * speed;
    }

    if (keys['d'] || keys['D']) {
        nextX += rightX * speed;
        nextZ += rightZ * speed;
    }

    if (canMoveTo(nextX, nextZ)) {
        cameraX = nextX;
        cameraZ = nextZ;
    }
}

// ======================= NHIN VAO VAT =======================

void getCameraForward(float& fx, float& fy, float& fz)
{
    float yawRad = yawAngle * PI / 180.0f;
    float pitchRad = pitchAngle * PI / 180.0f;

    fx = cos(yawRad) * cos(pitchRad);
    fy = sin(pitchRad);
    fz = sin(yawRad) * cos(pitchRad);
}

bool isLookingAt(float targetX, float targetY, float targetZ, float maxDistance, float minDot)
{
    float fx, fy, fz;
    getCameraForward(fx, fy, fz);

    float dx = targetX - cameraX;
    float dy = targetY - cameraY;
    float dz = targetZ - cameraZ;

    float distance = sqrt(dx * dx + dy * dy + dz * dz);
    if (distance > maxDistance || distance < 0.001f) return false;

    dx /= distance;
    dy /= distance;
    dz /= distance;

    float dot = fx * dx + fy * dy + fz * dz;
    return dot > minDot;
}

InteractionTarget getCurrentLookTarget()
{
    if (isLookingAt(9.8f, 1.5f, 6.4f, 3.0f, 0.82f)) return TARGET_DOOR;

    if (isLookingAt(9.85f, 1.78f, 4.55f, 3.0f, 0.70f)) return TARGET_LIGHT_SWITCH;
    if (isLookingAt(9.85f, 1.78f, 5.09f, 3.0f, 0.70f)) return TARGET_FAN_SWITCH;
    if (isLookingAt(9.85f, 1.32f, 4.82f, 3.0f, 0.70f)) return TARGET_AC_SWITCH;

    if (isLookingAt(0.18f, 1.55f, 4.45f, 2.4f, 0.70f)) return TARGET_CURTAIN_SWITCH;

    if (isLookingAt(7.0f, 1.55f, 0.14f, 2.5f, 0.95f)) return TARGET_TV;
    if (isLookingAt(2.10f, 2.65f, 0.25f, 3.5f, 0.82f)) return TARGET_CLOCK;
    if (isLookingAt(7.0f, 0.45f, 0.96f + drawerOffset, 2.0f, 0.8f)) return TARGET_DRAWER;

    return TARGET_NONE;
}

void interact()
{
    currentTarget = getCurrentLookTarget();

    switch (currentTarget) {
    case TARGET_DOOR:
        doorOpen = !doorOpen;
        break;

    case TARGET_LIGHT_SWITCH:
        mainLightOn = !mainLightOn;
        break;

    case TARGET_FAN_SWITCH:
        fanLevel++;
        if (fanLevel > 3) fanLevel = 0;
        break;

    case TARGET_AC_SWITCH:
        acOn = !acOn;
        break;

    case TARGET_CURTAIN_SWITCH:
        curtainDown = !curtainDown;
        break;

    case TARGET_TV:
        tvOn = !tvOn;
        break;

    case TARGET_CLOCK:
        gameHour += 1.0f;
        if (gameHour >= 24.0f) gameHour = 0.0f;
        updateDayNight();
        break;

    case TARGET_DRAWER:
        drawerOpen = !drawerOpen;
        break;

    default:
        break;
    }
}

// ======================= HUD =======================

void drawText2D(float x, float y, const char* text)
{
    glUseProgram(0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glColor3f(1.0f, 0.95f, 0.80f);
    glRasterPos2f(x, y);

    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glUseProgram(program);
}

const char* getPromptText()
{
    currentTarget = getCurrentLookTarget();

    switch (currentTarget) {
    case TARGET_DOOR:
        return doorOpen ? "Nhan E de dong cua" : "Nhan E de mo cua";

    case TARGET_LIGHT_SWITCH:
        return mainLightOn ? "Nhan E de tat den" : "Nhan E de bat den";

    case TARGET_FAN_SWITCH:
        if (fanLevel == 0) return "Nhan E de bat quat muc 1";
        if (fanLevel == 1) return "Nhan E de chuyen quat muc 2";
        if (fanLevel == 2) return "Nhan E de chuyen quat muc 3";
        return "Nhan E de tat quat";

    case TARGET_AC_SWITCH:
        return acOn ? "Nhan E de tat dieu hoa" : "Nhan E de bat dieu hoa";

    case TARGET_CURTAIN_SWITCH:
        return curtainDown ? "Nhan E de keo rem len" : "Nhan E de ha rem xuong";

    case TARGET_TV:
        return tvOn ? "Nhan E de tat TV" : "Nhan E de bat TV";

    case TARGET_CLOCK:
        return "Nhan E de tang gio";

    case TARGET_DRAWER:
        return drawerOpen ? "Nhan E de day ngan keo vao" : "Nhan E de keo ngan keo ra";

    default:
        return "";
    }
}

void drawCrosshair()
{
    drawText2D(windowWidth / 2 - 5, windowHeight / 2, "+");
}

void drawHUD()
{
    const char* prompt = getPromptText();

    drawCrosshair();

    if (prompt[0] != '\0') {
        drawText2D(windowWidth / 2 - 150, windowHeight / 2 - 45, prompt);
    }

    char status[256];
    sprintf(
        status,
        "Den: %s | Quat: muc %d | Dieu hoa: %s | TV: %s | Gio: %.0f:00",
        mainLightOn ? "Bat" : "Tat",
        fanLevel,
        acOn ? "Bat" : "Tat",
        tvOn ? "Bat" : "Tat",
        gameHour
    );

    drawText2D(20, windowHeight - 35, status);
    drawText2D(20, 25, "WASD: Di chuyen | Mouse: Xoay nhin | E: Tuong tac | ESC: Thoat");
}

void updateWindowTitle()
{
    char title[300];
    sprintf(
        title,
        "Phong khach 3D | Den: %s | Den cay: %s | Quat: muc %d | Dieu hoa: %s | TV: %s | Gio: %.0f:00",
        mainLightOn ? "Bat" : "Tat",
        lampOn ? "Bat" : "Tat",
        fanLevel,
        acOn ? "Bat" : "Tat",
        tvOn ? "Bat" : "Tat",
        gameHour
    );

    glutSetWindowTitle(title);
}

// ======================= DISPLAY =======================

void updateView()
{
    float yawRad = yawAngle * PI / 180.0f;
    float pitchRad = pitchAngle * PI / 180.0f;

    float lookX = cameraX + cos(yawRad) * cos(pitchRad);
    float lookY = cameraY + sin(pitchRad);
    float lookZ = cameraZ + sin(yawRad) * cos(pitchRad);

    point4 eye(cameraX, cameraY, cameraZ, 1.0f);
    point4 at(lookX, lookY, lookZ, 1.0f);
    vec4 up(0.0f, 1.0f, 0.0f, 0.0f);

    view = LookAt(eye, at, up);
}

void display(void)
{
    updateView();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawRoom();

    drawWindow();
    drawCurtain();
    drawCurtainSwitch();
    drawDoor();

    drawTVArea();
    drawBookshelf();
    drawClock();
    drawCornerPlant();
    drawDecorBlocks();

    drawSofa();
    drawCoffeeTable();

    drawCeilingLight();
    drawFloorLamp();
    drawCeilingFan();
    drawAC();
    drawSwitchBoard();

    drawHUD();

    glutSwapBuffers();
}

// ======================= CALLBACK =======================

void idle()
{
    updateCamera();
    updateDoor();
    updateFan();
    updateCurtain();
    updateAC();
    updateDrawerAnim();
    updateDayNight();
    updateWindowTitle();

    glutPostRedisplay();
}

void keyboardDown(unsigned char key, int x, int y)
{
    keys[key] = true;

    switch (key) {
    case 27:
        exit(0);
        break;

    case 'e':
    case 'E':
        interact();
        break;

    case 'l':
    case 'L':
        mainLightOn = !mainLightOn;
        break;

    case 'f':
    case 'F':
        lampOn = !lampOn;
        break;

    case 'r':
    case 'R':
        fanLevel++;
        if (fanLevel > 3) fanLevel = 0;
        break;

    case 't':
    case 'T':
        tvOn = !tvOn;
        break;

    case 'c':
    case 'C':
        gameHour += 1.0f;
        if (gameHour >= 24.0f) gameHour = 0.0f;
        updateDayNight();
        break;

    case 'v':
    case 'V':
        gameHour -= 1.0f;
        if (gameHour < 0.0f) gameHour = 23.0f;
        updateDayNight();
        break;
    }
}

void keyboardUp(unsigned char key, int x, int y)
{
    keys[key] = false;
}

void mouseMotion(int x, int y)
{
    if (!mouseLocked) return;

    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    if (firstMouse) {
        firstMouse = false;
        glutWarpPointer(centerX, centerY);
        return;
    }

    float sensitivity = 0.045f;

    int offsetX = x - centerX;
    int offsetY = centerY - y;

    yawAngle += offsetX * sensitivity;
    pitchAngle += offsetY * sensitivity;

    if (pitchAngle > 89.0f) pitchAngle = 89.0f;
    if (pitchAngle < -89.0f) pitchAngle = -89.0f;

    glutWarpPointer(centerX, centerY);
}

void reshape(int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);

    float aspect = (float)width / (float)height;
    projection = Perspective(60.0f, aspect, 0.1f, 100.0f);
}

// ======================= MAIN =======================

int main(int argc, char** argv)
{
    for (int i = 0; i < 256; i++) keys[i] = false;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(80, 50);
    glutCreateWindow("Phong khach 3D");

    glewInit();

    generateGeometry();
    initGPUBuffers();
    shaderSetup();

    projection = Perspective(60.0f, (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

    updateDayNight();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);

    glutPassiveMotionFunc(mouseMotion);
    glutMotionFunc(mouseMotion);

    glutSetCursor(GLUT_CURSOR_NONE);
    glutWarpPointer(windowWidth / 2, windowHeight / 2);

    glutMainLoop();
    return 0;
}//