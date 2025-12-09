#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "SkyBox.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 windmillModel;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 lightPos;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint lightPosLoc;
GLint lightPosLoc2;
GLint lampOnLoc;
GLint lampOnLoc2;
GLint sunOnLoc;
GLint isShinyLoc;

// boolean
GLboolean lampOn = false;
GLboolean lampOn2 = false;
GLboolean sunOn = false;
GLboolean startAnimation = true;

// models
gps::Model3D static_scene;
gps::Model3D water;
gps::Model3D lamp;
gps::Model3D villageLamp;
gps::Model3D windmill;
gps::Model3D shiny_scene;

// camera and auxiliaries
gps::Camera myCamera(
    glm::vec3(-60.0f, 10.0f, -70.0f),
    glm::vec3(40.0f, 10.0f, -30.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 1.0f;
GLfloat windmillRotFactor = 1.0f;
GLfloat angle = 0.0f;
GLint frame = 0;

GLboolean pressedKeys[1024];
GLfloat lastX = 400, lastY = 300;
GLfloat yaw = -90.0f, pitch = 0.0f;
GLboolean firstMouse = true;
GLboolean isMouseGrabbed = false;

// shaders
gps::Shader myBasicShader;
gps::Shader skyboxShader;

// skybox
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;

GLenum glCheckError_(const char* file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

// windmill rotation
glm::mat4 windmill_anim() {
    windmillModel = glm::mat4(1.0f);
    windmillModel = glm::translate(windmillModel, glm::vec3(207.92f, 32.116f, 361.69f));
    windmillModel = glm::rotate(windmillModel, glm::radians(windmillRotFactor), glm::vec3(1.0f, 0.0f, 0.0f));
    windmillRotFactor += 1.0f;
    return windmillModel;
}

// update view after camera movement
void updateView() {
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

// process camera movement
void processCameraMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_Z]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_X]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
    }
    updateView();
}

// process mouse movement
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (isMouseGrabbed && glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        GLfloat xoffset = xpos - lastX;
        GLfloat yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        const GLfloat sensitivity = 0.05f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        myCamera.rotate(yaw, pitch);
        updateView();
    }
}

// window resize
void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
}

// process keyboard
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_1)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_LINE_SMOOTH);
    }
    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_2)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_3)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_4)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LINE_SMOOTH);
    }
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        if (isMouseGrabbed == true) isMouseGrabbed = false;
        else isMouseGrabbed = true;
        glfwSetInputMode(window, GLFW_CURSOR, isMouseGrabbed ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_P)) {
        glm::vec3 campoz = myCamera.getCameraPosition();
        printf("%f %f %f\n", campoz.x, campoz.y, campoz.z);
        glm::vec3 camtarg = myCamera.getCameraTarget();
        printf("%f %f %f\n", camtarg.x, camtarg.y, camtarg.z);
    }
    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_L)) {
        if (sunOn == true) sunOn = false;
        else sunOn = true;
        glUniform1i(sunOnLoc, sunOn);
    }
    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_K)) {
        if (lampOn == true) lampOn = false;
        else lampOn = true;

        glUniform1i(lampOnLoc, lampOn);
    }
}

// initialize window
void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

// window callbacks
void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

// initialize OpenGL state
void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

// initialize models
void initModels() {
    static_scene.LoadModel("models/static_scene/static_scene.obj");
    water.LoadModel("models/water/water.obj");
    lamp.LoadModel("models/lamp/lamp.obj");
    villageLamp.LoadModel("models/villageLamp/villageLamp.obj");
    windmill.LoadModel("models/windmill/windmill.obj");
    shiny_scene.LoadModel("models/shiny_scene/shiny_scene.obj");
}

// initialize shaders
void initShaders() {

    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");

    faces.push_back("skybox/right.tga");
    faces.push_back("skybox/left.tga");
    faces.push_back("skybox/top.tga");
    faces.push_back("skybox/bottom.tga");
    faces.push_back("skybox/back.tga");
    faces.push_back("skybox/front.tga");

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    mySkyBox.Load(faces);
}

// initialize uniform variables
void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix for static_scene
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");

    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for static_scene
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // set the light direction (direction towards the light)
    lightDir = glm::vec3(301.6f, 168.0f, -186.08f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    // set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // boolean
    isShinyLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isShiny");

    sunOnLoc = glGetUniformLocation(myBasicShader.shaderProgram, "sunOn");
    lampOnLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lampOn");

    // town lamp
    lightPos = glm::vec3(-82.21f, 12.47f, -58.23f);
    lightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPosition");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

    // village light
    lightPos = glm::vec3(162.38f, 26.14f, -71.27f);
    lightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPosition2");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
}

// render skybox
void renderSkybox() {

    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));
    mySkyBox.Draw(skyboxShader, view, projection);
}

// render static scene
void renderStaticScene() {

    myBasicShader.useShaderProgram();
    glUniform1i(isShinyLoc, false);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    static_scene.Draw(myBasicShader);
}

// render shiny objects
void renderShiny() {

    myBasicShader.useShaderProgram();
    glUniform1i(isShinyLoc, true);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    shiny_scene.Draw(myBasicShader);
}

// render water
void renderWater() {

    myBasicShader.useShaderProgram();
    glUniform1i(isShinyLoc, true);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    water.Draw(myBasicShader);
}

// render town lamp
void renderLamp() {

    myBasicShader.useShaderProgram();
    glUniform1i(isShinyLoc, true);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    lamp.Draw(myBasicShader);
}

// render village lamp
void renderVillageLamp() {

    myBasicShader.useShaderProgram();
    glUniform1i(isShinyLoc, true);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    villageLamp.Draw(myBasicShader);
}

// render windmill wings
void renderWindmill() {

    myBasicShader.useShaderProgram();
    windmillModel = windmill_anim();
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(windmillModel));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    windmill.Draw(myBasicShader);
}

// render scene
void renderScene() {

    // scene presentation
    if (startAnimation == true) {
        frame++;
        if (frame == 10) {
            myCamera.setCameraPosition(glm::vec3(-60.0f, 10.0f, -70.0f));
            myCamera.setCameraTarget(glm::vec3(-59.0f, 10.0f, -70.0f));
        }
        if (frame > 10 && frame <= 169) {
            pressedKeys[GLFW_KEY_W] = true;
        }
        if (frame == 170) {
            pressedKeys[GLFW_KEY_W] = false;
        }
        if (frame == 171) {
            myCamera.setCameraPosition(glm::vec3(-29.16f, 25.67f, -10.27f));
            myCamera.setCameraTarget(glm::vec3(-29.83f, 25.23f, -10.87f));
        }
        if (frame == 200) {
            myCamera.setCameraPosition(glm::vec3(-41.91f, 18.76f, -64.53f));
            myCamera.setCameraTarget(glm::vec3(-42.49f, 18.46f, -63.77f));
        }
        if (frame == 230) {
            myCamera.setCameraPosition(glm::vec3(-311.97f, 13.23f, -69.40f));
            myCamera.setCameraTarget(glm::vec3(-310.98f, 13.16f, -69.42f));
        }
        if (frame > 230 && frame <= 282) {
            pressedKeys[GLFW_KEY_Z] = true;
        }
        if (frame == 283) {
            pressedKeys[GLFW_KEY_Z] = false;
        }
        if (frame > 283 && frame < 310) {
            myCamera.setCameraTarget(glm::vec3(-311.65f, 65.26f, -68.45f));
        }
        if (frame == 310) {
            myCamera.setCameraPosition(glm::vec3(-379.76f, 28.93f, -111.14f));
            myCamera.setCameraTarget(glm::vec3(-379.01f, 28.95f, -110.48f));
        }
        if (frame > 310 && frame < 680) {
            pressedKeys[GLFW_KEY_W] = true;
            pressedKeys[GLFW_KEY_D] = true;
        }
        if (frame == 680) {
            pressedKeys[GLFW_KEY_W] = false;
            pressedKeys[GLFW_KEY_D] = false;
        }
        if (frame > 680 && frame < 730) {
            myCamera.setCameraPosition(glm::vec3(-449.02f, 96.34f, -197.84f));
            myCamera.setCameraTarget(glm::vec3(-448.23f, 96.25f, -197.23f));
        }
        if (frame == 730) {
            myCamera.setCameraPosition(glm::vec3(-60.0f, 10.0f, -70.0f));
            myCamera.setCameraTarget(glm::vec3(-59.0f, 10.0f, -70.0f));
            startAnimation = false;
        }
    }
    else {
        frame = 0;
    }

    // Clear color and depth buffer for the skybox
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the windmill
    renderWindmill();

    // Render the lamps
    renderLamp();
    renderVillageLamp();

    // Render the water
    renderWater();

    // Render the skybox
    renderSkybox();

    // Render the static scene
    renderStaticScene();

    // Render the shiny objects
    renderShiny();

    // Swap the back buffer with the front buffer
    glfwSwapBuffers(myWindow.getWindow());
}

void cleanup() {
    myWindow.Delete();
    // cleanup code for your own data
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processCameraMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
