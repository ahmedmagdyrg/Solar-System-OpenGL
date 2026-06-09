#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Sphere.h"
#include <iostream>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

enum class AppState {
    MAIN_MENU,
    TUTORIAL,
    SIMULATION
};
AppState currentState = AppState::MAIN_MENU; 

struct PlanetData {
    std::string name;
    std::string type;
    std::string gravity;
    std::string fact;
};

const int NUM_PLANETS = 13;

std::map<int, PlanetData> solarSystemData = {
    {0, {"Mercury", "Terrestrial", "3.7 m/s^2", "Closest planet to the Sun. Super fast orbit."}},
    {1, {"Venus", "Terrestrial", "8.87 m/s^2", "Hottest planet due to a dense toxic atmosphere."}},
    {2, {"Earth", "Terrestrial", "9.81 m/s^2", "Our home. The only known planet with life."}},
    {3, {"Mars", "Terrestrial", "3.71 m/s^2", "Home to Olympus Mons, the largest volcano."}},
    {4, {"Jupiter", "Gas Giant", "24.79 m/s^2", "Has a Great Red Spot storm raging for centuries."}},
    {5, {"Saturn", "Gas Giant", "10.44 m/s^2", "Famous for its spectacular ring system."}},
    {6, {"Uranus", "Ice Giant", "8.69 m/s^2", "Rotates completely on its side."}},
    {7, {"Neptune", "Ice Giant", "11.15 m/s^2", "The windiest planet in the solar system."}},
    {8, {"Ceres", "Dwarf Planet", "0.28 m/s^2", "Largest object in the asteroid belt."}},
    {9, {"Haumea", "Dwarf Planet", "0.40 m/s^2", "Known for its extremely elongated shape."}},
    {10, {"Makemake", "Dwarf Planet", "0.50 m/s^2", "A prominent Kuiper belt object."}},
    {11, {"Eris", "Dwarf Planet", "0.82 m/s^2", "One of the most massive dwarf planets."}},
    {12, {"Pluto", "Dwarf Planet", "0.62 m/s^2", "The most famous dwarf planet in the Kuiper belt."}}
};

glm::vec3 cameraPos = glm::vec3(0.0f, 100.0f, 350.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.3f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool firstMouse = true;
float yaw = -90.0f, pitch = -20.0f, lastX = 600.0f, lastY = 450.0f, fov = 45.0f;
float deltaTime = 0.0f, lastFrame = 0.0f;
float simSpeed = 0.5f;
float simTime = 0.0f;

bool isLanded = false;
int targetPlanetIndex = -1;
float targetPlanetSize = 0.0f;
float orbitYaw = 0.0f;
float orbitPitch = 15.0f;
//MO
glm::vec3 currentPositions[NUM_PLANETS];
float currentSizes[NUM_PLANETS] = { 1.0f, 1.8f, 2.2f, 1.5f, 6.0f, 5.0f, 3.5f, 3.5f, 0.5f, 0.8f, 0.9f, 1.1f, 0.7f };

float flightProgress = 1.0f;
glm::vec3 flightStartPos;

bool showUI = true;
bool showOrbits = true;
bool tabKeyPressed = false;

bool isCollidingWarning = false;
float collisionWarningTimer = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (currentState != AppState::SIMULATION) return;

    float xpos = static_cast<float>(xposIn); float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }

    float xoff = xpos - lastX;
    float yoff = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (isLanded && flightProgress >= 1.0f) {
        xoff *= 0.3f;
        yoff *= 0.3f;
        orbitYaw -= xoff;
        orbitPitch += yoff;
        if (orbitPitch > 89.0f) orbitPitch = 89.0f;
        if (orbitPitch < -89.0f) orbitPitch = -89.0f;
        return;
    }

    if (!isLanded) {
        xoff *= 0.1f; yoff *= 0.1f;
        yaw += xoff; pitch += yoff;
        if (pitch > 89.0f) pitch = 89.0f; if (pitch < -89.0f) pitch = -89.0f;
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (currentState != AppState::SIMULATION) return;
    fov -= (float)yoffset;
    if (fov < 1.0f) fov = 1.0f; if (fov > 45.0f) fov = 45.0f;
}

void processInput(GLFWwindow* window) {
    float moveSpeed = 100.0f * deltaTime;

    if (currentState == AppState::SIMULATION) {
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
            if (!tabKeyPressed) { showUI = !showUI; tabKeyPressed = true; }
        }
        else { tabKeyPressed = false; }

        if (!isLanded) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += moveSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= moveSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * moveSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * moveSpeed;
        }

        float sunSafeDist = 18.0f;
        glm::vec3 sunDiff = cameraPos - glm::vec3(0.0f);
        float sunDist = glm::length(sunDiff);
        if (sunDist < sunSafeDist) {
            if (sunDist == 0.0f) sunDiff = glm::vec3(0.0f, 1.0f, 0.0f);
            cameraPos = glm::vec3(0.0f) + glm::normalize(sunDiff) * sunSafeDist;
            if (!isLanded) { isCollidingWarning = true; collisionWarningTimer = 1.5f; }
        }

        for (int i = 0; i < NUM_PLANETS; i++) {
            float planetSafeDist = currentSizes[i] + 3.0f;
            glm::vec3 planetDiff = cameraPos - currentPositions[i];
            float planetDist = glm::length(planetDiff);

            if (planetDist < planetSafeDist) {
                if (planetDist == 0.0f) planetDiff = glm::vec3(0.0f, 1.0f, 0.0f);
                cameraPos = currentPositions[i] + glm::normalize(planetDiff) * planetSafeDist;
                if (!isLanded) { isCollidingWarning = true; collisionWarningTimer = 1.5f; }
            }
        }

        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !isLanded) {
            for (int i = 0; i < NUM_PLANETS; i++) {
                float dist = glm::distance(cameraPos, currentPositions[i]);
                if (dist < (currentSizes[i] + 40.0f)) {
                    isLanded = true; targetPlanetIndex = i; targetPlanetSize = currentSizes[i];
                    flightProgress = 0.0f; flightStartPos = cameraPos;
                    glm::vec3 relPos = glm::normalize(cameraPos - currentPositions[i]);
                    orbitPitch = glm::clamp(glm::degrees(asin(relPos.y)), -89.0f, 89.0f);
                    orbitYaw = glm::degrees(atan2(relPos.z, relPos.x));
                    break;
                }
            }
        }

        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS && isLanded) {
            isLanded = false; targetPlanetIndex = -1;
        }
    }
}

unsigned int loadTexture(const char* path, bool hasAlpha = false) {
    unsigned int tID; glGenTextures(1, &tID);
    int w, h, c; stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &c, 0);
    if (data) {
        GLenum f = (c == 4 || hasAlpha) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, tID);
        glTexImage2D(GL_TEXTURE_2D, 0, f, w, h, 0, f, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    return tID;
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 900, "Solar System Ultimate", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    Shader ourShader("Shaders/vertex_shader.vs", "Shaders/fragment_shader.fs");
    Sphere planet(1.0f, 64, 64);
    Sphere skybox(5000.0f, 64, 64);

    unsigned int sunT = loadTexture("Textures/sun.jpg");
    unsigned int glowT = loadTexture("Textures/sun_glow.png", true);
    unsigned int mercT = loadTexture("Textures/mercury.jpg");
    unsigned int venT = loadTexture("Textures/venus.jpg");
    unsigned int earthT = loadTexture("Textures/earth.jpg");
    unsigned int earthNightT = loadTexture("Textures/earth_night.jpg");
    unsigned int cloudsT = loadTexture("Textures/earth_clouds.jpg");
    unsigned int moonT = loadTexture("Textures/moon.jpg");
    unsigned int marsT = loadTexture("Textures/mars.jpg");
    unsigned int jupT = loadTexture("Textures/jupiter.jpg");
    unsigned int satT = loadTexture("Textures/saturn.jpg");
    unsigned int satRingT = loadTexture("Textures/saturn_ring.png", true);
    unsigned int uraT = loadTexture("Textures/uranus.jpg");
    unsigned int nepT = loadTexture("Textures/neptune.jpg");
    unsigned int starT = loadTexture("Textures/stars.jpg");
    unsigned int ceresT = loadTexture("Textures/4k_ceres_fictional.jpg");
    unsigned int haumeaT = loadTexture("Textures/4k_haumea_fictional.jpg");
    unsigned int makemakeT = loadTexture("Textures/4k_makemake_fictional.jpg");
    unsigned int erisT = loadTexture("Textures/4k_eris_fictional.jpg");
    unsigned int plutoT = loadTexture("Textures/pluto.jpg");

    unsigned int orbitVAO, orbitVBO;
    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);
    std::vector<float> orbitVertices;
    int segments = 120;
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / (float)segments * 2.0f * 3.14159265f;
        orbitVertices.push_back(cos(angle)); orbitVertices.push_back(0.0f); orbitVertices.push_back(sin(angle));
        orbitVertices.push_back(0.0f); orbitVertices.push_back(1.0f); orbitVertices.push_back(0.0f);
        orbitVertices.push_back(0.0f); orbitVertices.push_back(0.0f);
    }
    glBindVertexArray(orbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, orbitVertices.size() * sizeof(float), orbitVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    Sphere asteroid(1.0f, 10, 10);
    int asteroidCount = 700;
    glm::mat4* asteroidModels = new glm::mat4[asteroidCount];
    for (int i = 0; i < asteroidCount; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        float angle = (float)(rand() % 360);
        float radius = 120.0f + (rand() % 25);
        float yOffset = ((rand() % 50) / 10.0f) - 2.5f;
        float x = radius * cos(glm::radians(angle));
        float z = radius * sin(glm::radians(angle));
        model = glm::translate(model, glm::vec3(x, yOffset, z));
        float scaleX = ((rand() % 20) / 100.0f) + 0.05f;
        float scaleY = ((rand() % 20) / 100.0f) + 0.05f;
        float scaleZ = ((rand() % 20) / 100.0f) + 0.05f;
        model = glm::scale(model, glm::vec3(scaleX, scaleY, scaleZ));
        float rotAngle = (float)(rand() % 360);
        model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.4f, 0.6f, 0.8f));
        asteroidModels[i] = model;
    }

    int kuiperCount = 1000;
    glm::mat4* kuiperModels = new glm::mat4[kuiperCount];
    for (int i = 0; i < kuiperCount; i++) {
        glm::mat4 model = glm::mat4(1.0f);
        float angle = (float)(rand() % 360);
        float radius = 330.0f + (rand() % 120);
        float yOffset = ((rand() % 100) / 10.0f) - 5.0f;
        float x = radius * cos(glm::radians(angle));
        float z = radius * sin(glm::radians(angle));
        model = glm::translate(model, glm::vec3(x, yOffset, z));
        float scaleX = ((rand() % 30) / 100.0f) + 0.05f;
        float scaleY = ((rand() % 30) / 100.0f) + 0.05f;
        float scaleZ = ((rand() % 30) / 100.0f) + 0.05f;
        model = glm::scale(model, glm::vec3(scaleX, scaleY, scaleZ));
        float rotAngle = (float)(rand() % 360);
        model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.4f, 0.6f, 0.8f));
        kuiperModels[i] = model;
    }
    //MO
    float orbitSpeeds[NUM_PLANETS] = { 0.7f, 0.5f, 0.4f, 0.3f, 0.15f, 0.1f, 0.07f, 0.05f,   0.25f, 0.04f, 0.035f, 0.03f, 0.038f };
    float distances[NUM_PLANETS] = { 35.0f, 55.0f, 85.0f, 110.0f, 160.0f, 210.0f, 260.0f, 300.0f,   135.0f, 340.0f, 380.0f, 420.0f, 360.0f };

    while (!glfwWindowShouldClose(window)) {
        float realTime = (float)glfwGetTime();
        deltaTime = realTime - lastFrame;
        lastFrame = realTime;
        simTime += deltaTime * simSpeed;

        if (collisionWarningTimer > 0.0f) {
            collisionWarningTimer -= deltaTime;
        }
        else {
            isCollidingWarning = false;
        }

        if (currentState == AppState::SIMULATION) {
            if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                firstMouse = true;
            }
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true;
        }

        for (int i = 0; i < NUM_PLANETS; i++) {
            glm::mat4 m = glm::rotate(glm::mat4(1.0f), simTime * orbitSpeeds[i], glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::translate(m, glm::vec3(distances[i], 0.0f, 0.0f));
            currentPositions[i] = glm::vec3(m[3]);
        }

        if (isLanded && targetPlanetIndex != -1 && currentState == AppState::SIMULATION) {
            glm::vec3 pPos = currentPositions[targetPlanetIndex];
            float radius = targetPlanetSize * 3.5f;
            glm::vec3 targetOrbitPos;
            targetOrbitPos.x = pPos.x + radius * cos(glm::radians(orbitPitch)) * cos(glm::radians(orbitYaw));
            targetOrbitPos.y = pPos.y + radius * sin(glm::radians(orbitPitch));
            targetOrbitPos.z = pPos.z + radius * cos(glm::radians(orbitPitch)) * sin(glm::radians(orbitYaw));

            if (flightProgress < 1.0f) {
                flightProgress += deltaTime * 0.35f;
                if (flightProgress > 1.0f) flightProgress = 1.0f;
                float t = 1.0f - pow(1.0f - flightProgress, 3.0f);
                glm::vec3 basePos = glm::mix(flightStartPos, targetOrbitPos, t);
                float dist = glm::distance(flightStartPos, targetOrbitPos);
                float hopHeight = dist * 0.5f;
                if (hopHeight < 150.0f) hopHeight = 150.0f;
                float currentHop = sin(t * 3.14159265f) * hopHeight;
                cameraPos = basePos + glm::vec3(0.0f, currentHop, 0.0f);
                glm::vec3 targetFront = glm::normalize(pPos - cameraPos);
                cameraFront = glm::normalize(glm::mix(cameraFront, targetFront, 5.0f * deltaTime));
            }
            else {
                cameraPos = glm::mix(cameraPos, targetOrbitPos, 15.0f * deltaTime);
                glm::vec3 targetFront = glm::normalize(pPos - cameraPos);
                cameraFront = glm::normalize(glm::mix(cameraFront, targetFront, 15.0f * deltaTime));
            }
            pitch = glm::degrees(asin(cameraFront.y));
            yaw = glm::degrees(atan2(cameraFront.z, cameraFront.x));
        }

        processInput(window);

        // --- ImGui ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 center = ImVec2(screenSize.x * 0.5f, screenSize.y * 0.5f);

        if (currentState == AppState::MAIN_MENU) {
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.03f, 0.05f, 0.85f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.7f, 1.0f, 0.5f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40.0f, 40.0f));

            ImGui::Begin("MainMenu", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::SetWindowFontScale(2.5f);
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "SOLAR SYSTEM ULTIMATE");

            ImGui::SetWindowFontScale(1.2f);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Interactive 3D Simulation");
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

            if (ImGui::Button("START SIMULATION", ImVec2(450, 60))) currentState = AppState::SIMULATION;
            ImGui::Spacing();
            if (ImGui::Button("HOW TO PLAY (TUTORIAL)", ImVec2(450, 60))) currentState = AppState::TUTORIAL;
            ImGui::Spacing(); ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            if (ImGui::Button("EXIT GAME", ImVec2(450, 60))) glfwSetWindowShouldClose(window, true);
            ImGui::PopStyleColor(3);

            ImGui::End();
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);

        }
        else if (currentState == AppState::TUTORIAL) {
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.03f, 0.05f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.7f, 1.0f, 0.8f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40.0f, 40.0f));

            ImGui::Begin("Tutorial", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::SetWindowFontScale(1.8f);
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "[ MISSION BRIEFING & CONTROLS ]");
            ImGui::Separator(); ImGui::Spacing();

            ImGui::SetWindowFontScale(1.3f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "W A S D :"); ImGui::SameLine(120); ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Navigate Spacecraft (Camera)");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Mouse   :"); ImGui::SameLine(120); ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Look Around");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "ALT Key :"); ImGui::SameLine(120); ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Hold to show cursor & interact with UI");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "L Key   :"); ImGui::SameLine(120); ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Auto-Land on the nearest planet");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "U Key   :"); ImGui::SameLine(120); ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Disengage Orbit (Free Roam)");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "TAB Key :"); ImGui::SameLine(120); ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Hide / Show HUD");

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

            if (ImGui::Button("RESUME / START", ImVec2(500, 60))) currentState = AppState::SIMULATION;
            ImGui::Spacing();
            if (ImGui::Button("MAIN MENU", ImVec2(500, 40))) currentState = AppState::MAIN_MENU;

            ImGui::End();
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);

        }
        else if (currentState == AppState::SIMULATION) {
            if (isCollidingWarning && !isLanded) {
                ImGui::SetNextWindowPos(ImVec2(screenSize.x * 0.5f, screenSize.y - 120.0f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
                float pulse = (sin(glfwGetTime() * 8.0f) + 1.0f) * 0.5f;

                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.02f, 0.02f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.2f, 0.2f, 0.5f + pulse * 0.5f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 15.0f));

                ImGui::Begin("ProximityAlert", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::SetWindowFontScale(1.4f);
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[!] PROXIMITY ALERT");
                ImGui::Separator();
                ImGui::SetWindowFontScale(1.1f);
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.8f, 1.0f), "Collision trajectory detected.");
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 0.6f + pulse * 0.4f), "Auto-correction thrusters engaged...");
                ImGui::End();

                ImGui::PopStyleVar(3);
                ImGui::PopStyleColor(2);
            }

            if (showUI) {
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.02f, 0.04f, 0.06f, 0.95f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.7f, 0.9f, 0.8f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));

                ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(420.0f, 0.0f), ImGuiCond_Always);

                ImGui::Begin("Command Center", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
                ImGui::SetWindowFontScale(1.2f);
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "[ TIME CONTROLLER ]");
                ImGui::Separator(); ImGui::Spacing();
                ImGui::PushItemWidth(-1.0f);
                ImGui::SliderFloat("##Speed", &simSpeed, 0.0f, 5.0f, "Speed: %.2f x");
                ImGui::PopItemWidth(); ImGui::Spacing();

                float spacingX = ImGui::GetStyle().ItemSpacing.x;
                float btnW = (ImGui::GetContentRegionAvail().x - (spacingX * 2.0f)) / 3.0f;

                if (ImGui::Button("Pause", ImVec2(btnW, 35))) simSpeed = 0.0f; ImGui::SameLine();
                if (ImGui::Button("Normal", ImVec2(btnW, 35))) simSpeed = 0.5f; ImGui::SameLine();
                if (ImGui::Button("Reset", ImVec2(btnW, 35))) { simTime = 0.0f; simSpeed = 0.5f; }
                ImGui::Spacing(); ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "[ SETTINGS ]");
                ImGui::Separator(); ImGui::Spacing();
                ImGui::Checkbox("Show Planetary Orbits", &showOrbits);
                ImGui::Spacing(); ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "[ NAVIGATION SYSTEM ]");
                ImGui::Separator(); ImGui::Spacing();

                if (ImGui::BeginTable("PlanetsGrid", 2)) {
                    for (int i = 0; i < NUM_PLANETS; i++) {
                        ImGui::TableNextColumn();
                        if (ImGui::Button(solarSystemData[i].name.c_str(), ImVec2(-FLT_MIN, 35))) {
                            isLanded = true; targetPlanetIndex = i; targetPlanetSize = currentSizes[i];
                            flightProgress = 0.0f; flightStartPos = cameraPos;
                            glm::vec3 relPos = glm::normalize(cameraPos - currentPositions[i]);
                            orbitPitch = glm::clamp(glm::degrees(asin(relPos.y)), -89.0f, 89.0f);
                            orbitYaw = glm::degrees(atan2(relPos.z, relPos.x));
                        }
                    }
                    ImGui::EndTable();
                }
                ImGui::Spacing();

                if (ImGui::Button("Disengage / Free Roam", ImVec2(-1.0f, 40))) {
                    isLanded = false; targetPlanetIndex = -1;
                }
                ImGui::Spacing(); ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "[ SYSTEM CONTROLS ]");
                ImGui::Separator(); ImGui::Spacing();

                if (ImGui::Button("How to Play (Help)", ImVec2(-1.0f, 35))) currentState = AppState::TUTORIAL;

                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                if (ImGui::Button("Exit Application", ImVec2(-1.0f, 35))) glfwSetWindowShouldClose(window, true);
                ImGui::PopStyleColor(3);

                ImGui::Spacing(); ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                ImGui::TextWrapped("Hold 'ALT' to use cursor | Press 'TAB' to hide UI");
                ImGui::PopStyleColor();

                ImGui::End();

                if (isLanded && targetPlanetIndex != -1 && flightProgress >= 1.0f) {
                    PlanetData data = solarSystemData[targetPlanetIndex];
                    float windowWidth = 550.0f;
                    ImGui::SetNextWindowPos(ImVec2(screenSize.x - windowWidth - 40.0f, 40.0f), ImGuiCond_Always);

                    ImGui::Begin("Orbital HUD", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
                    ImGui::SetWindowFontScale(1.8f);
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "[ ORBITAL DATA LINK ESTABLISHED ]");
                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                    if (ImGui::BeginTable("PlanetDataTbl", 2)) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "DESIGNATION:");
                        ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", data.name.c_str());
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "CLASS:");
                        ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", data.type.c_str());
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "GRAVITY:");
                        ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", data.gravity.c_str());
                        ImGui::EndTable();
                    }
                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.0f, 0.7f, 0.9f, 1.0f), "ARCHIVE LOG:");
                    ImGui::TextWrapped("%s", data.fact.c_str());
                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                    float pulse = (sin(glfwGetTime() * 4.0f) + 1.0f) * 0.5f;
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 0.5f + (pulse * 0.5f)), ">> PRESS 'U' OR USE MENU TO DISENGAGE <<");
                    ImGui::End();
                }

                ImGui::PopStyleVar(3);
                ImGui::PopStyleColor(2);
            }
        }

        int w, h; glfwGetFramebufferSize(window, &w, &h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)w / h, 1.0f, 10000.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        ourShader.setVec3("lightPos", glm::vec3(0.0f));

        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);
        ourShader.setInt("isSkybox", 1);
        ourShader.setInt("isSun", 0);
        ourShader.setMat4("model", glm::translate(glm::mat4(1.0f), cameraPos));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, starT);
        ourShader.setInt("texture_day", 0);
        skybox.draw();
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);

        if (showOrbits) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);
            ourShader.setInt("isSun", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, moonT);

            glBindVertexArray(orbitVAO);
            for (int i = 0; i < NUM_PLANETS; i++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::scale(model, glm::vec3(distances[i], distances[i], distances[i]));
                ourShader.setMat4("model", model);
                glDrawArrays(GL_LINE_LOOP, 0, segments);
            }
            glBindVertexArray(0);

            glEnable(GL_CULL_FACE);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        ourShader.setInt("isSkybox", 0);
        ourShader.setInt("isSun", 1);
        ourShader.setInt("isEarth", 0);

        glm::mat4 sunCtx = glm::rotate(glm::mat4(1.0f), simTime * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 sunM = glm::scale(sunCtx, glm::vec3(15.0f));
        ourShader.setMat4("model", sunM);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunT);
        planet.draw();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glm::mat4 glowM = glm::scale(sunCtx, glm::vec3(16.0f));
        glowM = glm::rotate(glowM, simTime * 0.15f, glm::vec3(1.0f, 1.0f, 0.0f));
        ourShader.setMat4("model", glowM);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glowT);
        planet.draw();

        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        auto drawPlanet = [&](float orbitSpeed, float dist, float size, float selfRot, unsigned int tex, bool earth = false) {
            glm::mat4 m = glm::rotate(glm::mat4(1.0f), simTime * orbitSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::translate(m, glm::vec3(dist, 0.0f, 0.0f));
            glm::mat4 orbitContext = m;
            m = glm::scale(m, glm::vec3(size));
            m = glm::rotate(m, simTime * selfRot, glm::vec3(0.0f, 1.0f, 0.0f));
            ourShader.setMat4("model", m);
            ourShader.setInt("isSun", 0);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);
            ourShader.setInt("texture_day", 0);

            if (earth) {
                ourShader.setInt("isEarth", 1);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, earthNightT);
                ourShader.setInt("texture_night", 1);
            }
            else {
                ourShader.setInt("isEarth", 0);
            }
            planet.draw();
            return orbitContext;
            };

        drawPlanet(orbitSpeeds[0], distances[0], currentSizes[0], 1.0f, mercT);
        glm::mat4 venusCtx = drawPlanet(orbitSpeeds[1], distances[1], currentSizes[1], 0.8f, venT);
        glm::mat4 earthCtx = drawPlanet(orbitSpeeds[2], distances[2], currentSizes[2], 2.0f, earthT, true);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        ourShader.setInt("isEarth", 0);
        ourShader.setInt("isSun", 1);
        glm::mat4 cloudM = earthCtx;
        cloudM = glm::scale(cloudM, glm::vec3(2.23f));
        cloudM = glm::rotate(cloudM, simTime * 2.1f, glm::vec3(0.0f, 1.0f, 0.0f));
        ourShader.setMat4("model", cloudM);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cloudsT);
        planet.draw();
        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        ourShader.setInt("isEarth", 0);
        ourShader.setInt("isSun", 0);
        glm::mat4 moonM = glm::rotate(earthCtx, simTime * 2.5f, glm::vec3(0.0f, 1.0f, 0.0f));
        moonM = glm::translate(moonM, glm::vec3(5.0f, 0.0f, 0.0f));
        moonM = glm::scale(moonM, glm::vec3(0.6f));
        ourShader.setMat4("model", moonM);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonT);
        planet.draw();

        drawPlanet(orbitSpeeds[3], distances[3], currentSizes[3], 1.8f, marsT);

        ourShader.setInt("isSun", 0);
        ourShader.setInt("isEarth", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonT);
        glm::mat4 beltOrbit = glm::rotate(glm::mat4(1.0f), simTime * 0.15f, glm::vec3(0.0f, 1.0f, 0.0f));
        for (int i = 0; i < asteroidCount; i++) {
            ourShader.setMat4("model", beltOrbit * asteroidModels[i]);
            asteroid.draw();
        }

        glm::mat4 jupCtx = drawPlanet(orbitSpeeds[4], distances[4], currentSizes[4], 0.5f, jupT);

        ourShader.setInt("isSun", 0);
        ourShader.setInt("isEarth", 0);
        float jupMoonSpeeds[4] = { 4.0f, 3.2f, 2.5f, 1.8f };
        float jupMoonDist[4] = { 8.5f, 10.0f, 12.0f, 14.0f };
        float jupMoonSize[4] = { 0.25f, 0.20f, 0.30f, 0.15f };
        unsigned int jupMoonTex[4] = { venT, uraT, mercT, moonT };

        for (int m = 0; m < 4; m++) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, jupMoonTex[m]);
            glm::mat4 gMoonM = glm::rotate(jupCtx, simTime * jupMoonSpeeds[m], glm::vec3(0.0f, 1.0f, 0.0f));
            gMoonM = glm::translate(gMoonM, glm::vec3(jupMoonDist[m], 0.0f, 0.0f));
            gMoonM = glm::scale(gMoonM, glm::vec3(jupMoonSize[m]));
            ourShader.setMat4("model", gMoonM);
            planet.draw();
        }

        glm::mat4 satCtx = drawPlanet(orbitSpeeds[5], distances[5], currentSizes[5], 0.4f, satT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glm::mat4 ringM = glm::rotate(satCtx, glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ringM = glm::scale(ringM, glm::vec3(11.0f, 0.01f, 11.0f));
        ourShader.setMat4("model", ringM);
        ourShader.setInt("isSun", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, satRingT);
        planet.draw();
        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        drawPlanet(orbitSpeeds[6], distances[6], currentSizes[6], 0.3f, uraT);
        drawPlanet(orbitSpeeds[7], distances[7], currentSizes[7], 0.3f, nepT);

        drawPlanet(orbitSpeeds[8], distances[8], currentSizes[8], 1.2f, ceresT);
        drawPlanet(orbitSpeeds[9], distances[9], currentSizes[9], 1.1f, haumeaT);
        drawPlanet(orbitSpeeds[10], distances[10], currentSizes[10], 0.9f, makemakeT);
        drawPlanet(orbitSpeeds[11], distances[11], currentSizes[11], 0.8f, erisT);
        drawPlanet(orbitSpeeds[12], distances[12], currentSizes[12], 0.6f, plutoT);

        ourShader.setInt("isSun", 0);
        ourShader.setInt("isEarth", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, uraT);
        glm::mat4 kuiperOrbit = glm::rotate(glm::mat4(1.0f), simTime * 0.02f, glm::vec3(0.0f, 1.0f, 0.0f));
        for (int i = 0; i < kuiperCount; i++) {
            ourShader.setMat4("model", kuiperOrbit * kuiperModels[i]);
            asteroid.draw();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    delete[] asteroidModels;
    delete[] kuiperModels;
    glDeleteVertexArrays(1, &orbitVAO);
    glDeleteBuffers(1, &orbitVBO);
    glfwTerminate();
    return 0;
}