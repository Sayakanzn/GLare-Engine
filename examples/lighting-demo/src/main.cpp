#include "GLare.hpp"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

int width = 1280, height = 720;
float lastFrame, currentFrame;
GLFWwindow* window = nullptr;

std::shared_ptr<GLR::Scene> scene;
std::shared_ptr<GLR::Renderer> renderer;

std::shared_ptr<GLR::Entity> directionalLight;
std::vector<std::shared_ptr<GLR::Entity>> pointLights;
std::vector<std::shared_ptr<GLR::Entity>> spotLights;
std::shared_ptr<GLR::Entity> mainCamera;

std::vector<std::shared_ptr<GLR::Entity>> demoSpheres;
std::vector<std::shared_ptr<GLR::Entity>> demoCubes;
std::shared_ptr<GLR::Entity> groundPlane;

float cameraDistance = 20.0f;
float cameraHeight = 8.0f;
float cameraAngle = 0.0f;
float cameraRotationSpeed = 0.5f;

struct DemoSettings {
    bool showDirectionalLight = true;
    bool showPointLights = true;
    bool showSpotLights = false;
    bool animateLights = true;
    float animationSpeed = 1.0f;
    float globalLightIntensity = 2.0f;
} demoSettings;

void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void glfwFramebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    glViewport(0, 0, width, height);
    scene->getMainCameraComponent()->setAspectRatio(newWidth, newHeight);
    renderer->resize(newWidth, newHeight);
}

void processInput(GLFWwindow* window, float deltaTime) {
    static bool spacePressed = false;
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cameraAngle += cameraRotationSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cameraAngle -= cameraRotationSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        cameraHeight = glm::clamp(cameraHeight + 5.0f * deltaTime, 2.0f, 20.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        cameraHeight = glm::clamp(cameraHeight - 5.0f * deltaTime, 2.0f, 20.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraDistance = glm::clamp(cameraDistance - 10.0f * deltaTime, 5.0f, 50.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraDistance = glm::clamp(cameraDistance + 10.0f * deltaTime, 5.0f, 50.0f);
    }
}

void updateCamera() {    
    float x = cos(cameraAngle) * cameraDistance;
    float z = sin(cameraAngle) * cameraDistance;
    
    auto& cameraTransform = mainCamera->getComponent<GLR::Transform>();
    cameraTransform.setPosition(glm::vec3(x, cameraHeight, z));
    
    glm::vec3 cameraPos = glm::vec3(x, cameraHeight, z);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 direction = glm::normalize(target - cameraPos);
    
    float yaw = atan2(-direction.x, -direction.z);
    float pitch = asin(direction.y);
    
    cameraTransform.setRotation(glm::vec3(glm::degrees(pitch), glm::degrees(yaw), 0.0f));
}

void createDemoObjects() {
    auto shader = renderer->getObjectShader();
    
    auto sphereMesh = std::make_shared<GLR::Mesh>();
    sphereMesh->create(GLR::Shape::generateSphere(1.0f, 32), GLR::Shape::getSphereIndices(32), GLR::Shape::getStandardLayout());
    
    auto cubeMesh = std::make_shared<GLR::Mesh>();
    cubeMesh->create(GLR::Shape::generateCube(), GLR::Shape::getCubeIndices(), GLR::Shape::getStandardLayout());
    
    auto planeMesh = std::make_shared<GLR::Mesh>();
    planeMesh->create(GLR::Shape::generatePlane(1.0f, 1.0f, 1, 1, 5.0f, 5.0f), GLR::Shape::getPlaneIndices(1, 1), GLR::Shape::getStandardLayout());

    auto normalTexture = std::make_shared<GLR::Texture>("./res/texture/TestNormalMap.png");
    
    auto whiteMaterial = std::make_shared<GLR::Material>(shader);
    whiteMaterial->setVector4("baseColorFactor", glm::vec4(1.0, 1.0, 1.0, 1.0));

    auto redMaterial = std::make_shared<GLR::Material>(shader);
    redMaterial->setVector4("baseColorFactor", glm::vec4(1.0, 0.0, 0.0, 1.0));

    auto greenMaterial = std::make_shared<GLR::Material>(shader);
    greenMaterial->setVector4("baseColorFactor", glm::vec4(0.0, 1.0, 0.0, 1.0));

    auto blueMaterial = std::make_shared<GLR::Material>(shader);
    blueMaterial->setVector4("baseColorFactor", glm::vec4(0.0, 0.0, 1.0, 1.0));

    auto groundMaterial = std::make_shared<GLR::Material>(shader);
    groundMaterial->setVector4("baseColorFactor", glm::vec4(1.0, 1.0, 1.0, 1.0));
    groundMaterial->setTexture("normalTexture", normalTexture);
    
    const int numSpheres = 8;
    const float radius = 6.0f;
    for (int i = 0; i < numSpheres; i++) {
        float angle = (i / float(numSpheres)) * 2.0f * M_PI;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;
        
        auto sphere = scene->createEntity("DemoSphere_" + std::to_string(i));
        sphere->addComponent<GLR::Transform>(glm::vec3(x, 1.0f, z), glm::vec3(0.0f), glm::vec3(1.0f));
        
        auto material = (i % 4 == 0) ? redMaterial : (i % 4 == 1) ? greenMaterial : (i % 4 == 2) ? blueMaterial : whiteMaterial;
        sphere->addComponent<GLR::MeshRenderer>(sphereMesh, material);
        demoSpheres.push_back(sphere);
    }
    
    const int numCubes = 6;
    for (int i = 0; i < numCubes; i++) {
        float angle = (i / float(numCubes)) * 2.0f * M_PI;
        float x = cos(angle) * 3.0f;
        float z = sin(angle) * 3.0f;
        float y = 0.5f + i * 0.5f;
        
        auto cube = scene->createEntity("DemoCube_" + std::to_string(i));
        cube->addComponent<GLR::Transform>(glm::vec3(x, y, z), glm::vec3(0.0f), glm::vec3(0.8f));
        cube->addComponent<GLR::MeshRenderer>(cubeMesh, whiteMaterial);
        demoCubes.push_back(cube);
    }
    
    groundPlane = scene->createEntity("Ground");
    groundPlane->addComponent<GLR::Transform>(glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(0.0f), glm::vec3(20.0f, 1.0f, 20.0f));
    groundPlane->addComponent<GLR::MeshRenderer>(planeMesh, groundMaterial);
}

void createLights() {
    directionalLight = scene->createEntity("DirectionalLight");
    directionalLight->addComponent<GLR::Transform>(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(-45.0f, 45.0f, 0.0f), glm::vec3(1.0f));
    directionalLight->addComponent<GLR::DirectionalLight>(glm::vec3(-1.0f, -1.0f, -0.5f), glm::vec3(1.0f, 0.95f, 0.8f), 0.35f, true, 15.0f, 50.0f, 200.0f, 2048);
    
    const float pointLightRadius = 8.0f;
    const glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 0.3f, 0.3f),
        glm::vec3(0.3f, 1.0f, 0.3f),
        glm::vec3(0.3f, 0.3f, 1.0f),
        glm::vec3(1.0f, 1.0f, 0.3f),
        glm::vec3(1.0f, 0.3f, 1.0f),
        glm::vec3(0.3f, 1.0f, 1.0f)
    };
    
    for (int i = 0; i < 6; i++) {
        float angle = (i / 6.0f) * 2.0f * M_PI;
        float x = cos(angle) * pointLightRadius;
        float z = sin(angle) * pointLightRadius;
        
        auto pointLight = scene->createEntity("PointLight_" + std::to_string(i));
        pointLight->addComponent<GLR::Transform>(glm::vec3(x, 2.0f + i * 0.5f, z), glm::vec3(0.0f), glm::vec3(1.0f));
        pointLight->addComponent<GLR::PointLight>(20.0f, pointLightColors[i], 2.0f, i < 2, GLR::PointLight::FalloffType::SHARP, 1024);
        pointLights.push_back(pointLight);
    }
    
    const glm::vec3 spotLightColors[] = {
        glm::vec3(1.0f, 0.8f, 0.6f),
        glm::vec3(0.6f, 0.8f, 1.0f),
        glm::vec3(1.0f, 0.6f, 0.8f)
    };
    
    for (int i = 0; i < 3; i++) {
        float angle = (i / 3.0f) * 2.0f * M_PI;
        float x = cos(angle) * 10.0f;
        float z = sin(angle) * 10.0f;
        
        auto spotLight = scene->createEntity("SpotLight_" + std::to_string(i));
        spotLight->addComponent<GLR::Transform>(glm::vec3(x, 6.0f, z), glm::vec3(-30.0f, 0.0f, 0.0f), glm::vec3(1.0f));
        spotLight->addComponent<GLR::SpotLight>(15.0f, 25.0f, 15.0f, spotLightColors[i], 3.0f, GLR::SpotLight::FalloffType::NORMAL);
        
        auto& transform = spotLight->getComponent<GLR::Transform>();
        glm::vec3 direction = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - glm::vec3(x, 6.0f, z));
        float yaw = atan2(-direction.x, -direction.z);
        float pitch = asin(direction.y);
        transform.setRotation(glm::vec3(glm::degrees(pitch), glm::degrees(yaw), 0.0f));
        
        spotLights.push_back(spotLight);
    }
}

void updateLights(float deltaTime) {
    if (!demoSettings.animateLights) return;
    
    float time = currentFrame * demoSettings.animationSpeed;
    
    for (size_t i = 0; i < pointLights.size(); i++) {
        auto& transform = pointLights[i]->getComponent<GLR::Transform>();
        float baseAngle = (i / float(pointLights.size())) * 2.0f * M_PI;
        float angle = baseAngle + time * 0.5f;
        float radius = 8.0f + sin(time + i) * 2.0f;
        float height = 2.0f + sin(time * 0.7f + i) * 1.5f;
        
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;
        
        transform.setPosition(glm::vec3(x, height, z));
        
        auto& pointLight = pointLights[i]->getComponent<GLR::PointLight>();
        float intensity = 2.0f + sin(time * 2.0f + i) * 0.5f;
        pointLight.setIntensity(intensity * demoSettings.globalLightIntensity);
    }
    
    for (size_t i = 0; i < spotLights.size(); i++) {
        auto& transform = spotLights[i]->getComponent<GLR::Transform>();
        float baseAngle = (i / float(spotLights.size())) * 2.0f * M_PI;
        float angle = baseAngle + time * 0.3f;
        float radius = 10.0f + sin(time * 0.5f + i) * 3.0f;
        float height = 6.0f + sin(time * 0.4f + i) * 2.0f;
        
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;
        
        transform.setPosition(glm::vec3(x, height, z));
        
        glm::vec3 target = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 direction = glm::normalize(target - glm::vec3(x, height, z));
        float yaw = atan2(-direction.x, -direction.z);
        float pitch = asin(direction.y);
        transform.setRotation(glm::vec3(glm::degrees(pitch), glm::degrees(yaw), 0.0f));
        
        auto& spotLight = spotLights[i]->getComponent<GLR::SpotLight>();
        float intensity = 3.0f + sin(time * 1.5f + i) * 1.0f;
        spotLight.setIntensity(intensity * demoSettings.globalLightIntensity);
    }
    
    auto& dirTransform = directionalLight->getComponent<GLR::Transform>();
    auto& dirLight = directionalLight->getComponent<GLR::DirectionalLight>();
    
    float sunAngle = time * 0.5f;
    glm::vec3 sunDirection = glm::vec3(sin(sunAngle), -0.8f, cos(sunAngle));
    dirLight.setDirection(sunDirection);    
}

void renderImGui() {
    if (ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        if (ImGui::CollapsingHeader("Demo Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Show Directional Light", &demoSettings.showDirectionalLight);
            ImGui::Checkbox("Show Point Lights", &demoSettings.showPointLights);
            ImGui::Checkbox("Show Spot Lights", &demoSettings.showSpotLights);
            ImGui::Checkbox("Animate Lights", &demoSettings.animateLights);
            ImGui::SliderFloat("Animation Speed", &demoSettings.animationSpeed, 0.0f, 3.0f);
            ImGui::SliderFloat("Global Light Intensity", &demoSettings.globalLightIntensity, 0.0f, 3.0f);
        }
        
        if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& dirLight = directionalLight->getComponent<GLR::DirectionalLight>();
            
            glm::vec3 direction = dirLight.getDirection();
            if (ImGui::SliderFloat3("Direction", &direction.x, -1.0f, 1.0f)) {
                dirLight.setDirection(direction);
            }
            
            glm::vec3 color = dirLight.getColor();
            if (ImGui::ColorEdit3("Color", &color.x)) {
                dirLight.setColor(color);
            }
            
            float intensity = dirLight.getIntensity();
            if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 3.0f)) {
                dirLight.setIntensity(intensity);
            }
            
            bool castShadows = dirLight.getCastShadows();
            if (ImGui::Checkbox("Cast Shadows", &castShadows)) {
                dirLight.setCastShadows(castShadows);
            }
            
            if (castShadows) {
                float size = dirLight.getShadowOrthoSize();
                if (ImGui::SliderFloat("Shadow Map Size", &size, 5.0f, 50.0f)) {
                    dirLight.setShadowOrthoSize(size);
                }
                
                int resolution = dirLight.getShadowMapResolution();
                if (ImGui::SliderInt("Shadow Resolution", &resolution, 512, 4096)) {
                    dirLight.setShadowMapResolution(resolution);
                }
                
                float bias = dirLight.getShadowBias();
                if (ImGui::SliderFloat("Shadow Bias", &bias, 0.0001f, 0.01f, "%.5f")) {
                    dirLight.setShadowBias(bias);
                }
            }
        }
        
        if (ImGui::CollapsingHeader("Point Lights")) {
            for (size_t i = 0; i < pointLights.size(); i++) {
                if (ImGui::TreeNode(("Point Light " + std::to_string(i)).c_str())) {
                    auto& pointLight = pointLights[i]->getComponent<GLR::PointLight>();
                    auto& transform = pointLights[i]->getComponent<GLR::Transform>();
                    
                    glm::vec3 position = transform.getPosition();
                    if (ImGui::SliderFloat3("Position", &position.x, -20.0f, 20.0f)) {
                        transform.setPosition(position);
                    }
                    
                    glm::vec3 color = pointLight.getColor();
                    if (ImGui::ColorEdit3("Color", &color.x)) {
                        pointLight.setColor(color);
                    }
                    
                    float intensity = pointLight.getIntensity();
                    if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 5.0f)) {
                        pointLight.setIntensity(intensity);
                    }
                    
                    float radius = pointLight.getRadius();
                    if (ImGui::SliderFloat("Radius", &radius, 1.0f, 30.0f)) {
                        pointLight.setRadius(radius);
                    }
                    
                    const char* falloffTypes[] = {"Custom", "Sharp", "Normal", "Smooth", "Linear", "Quadratic"};
                    int currentFalloff = static_cast<int>(pointLight.getFalloffType());
                    if (ImGui::Combo("Falloff Type", &currentFalloff, falloffTypes, 6)) {
                        pointLight.setFalloffType(static_cast<GLR::PointLight::FalloffType>(currentFalloff));
                    }
                    
                    if (pointLight.getFalloffType() == GLR::PointLight::FalloffType::CUSTOM) {
                        float constant, linear, quadratic;
                        pointLight.getAttenuationFactors(constant, linear, quadratic);
                        
                        if (ImGui::SliderFloat("Constant", &constant, 0.0f, 2.0f)) {
                            pointLight.setConstant(constant);
                        }
                        if (ImGui::SliderFloat("Linear", &linear, 0.0f, 1.0f)) {
                            pointLight.setLinear(linear);
                        }
                        if (ImGui::SliderFloat("Quadratic", &quadratic, 0.0f, 1.0f)) {
                            pointLight.setQuadratic(quadratic);
                        }
                    }
                    
                    bool castShadows = pointLight.getCastShadows();
                    if (ImGui::Checkbox("Cast Shadows", &castShadows)) {
                        pointLight.setCastShadows(castShadows);
                    }
                    
                    ImGui::TreePop();
                }
            }
        }
        
        if (ImGui::CollapsingHeader("Spot Lights")) {
            for (size_t i = 0; i < spotLights.size(); i++) {
                if (ImGui::TreeNode(("Spot Light " + std::to_string(i)).c_str())) {
                    auto& spotLight = spotLights[i]->getComponent<GLR::SpotLight>();
                    auto& transform = spotLights[i]->getComponent<GLR::Transform>();
                    
                    glm::vec3 position = transform.getPosition();
                    if (ImGui::SliderFloat3("Position", &position.x, -20.0f, 20.0f)) {
                        transform.setPosition(position);
                    }
                    
                    glm::vec3 rotation = transform.getRotation();
                    if (ImGui::SliderFloat3("Rotation", &rotation.x, -180.0f, 180.0f)) {
                        transform.setRotation(rotation);
                    }
                    
                    glm::vec3 color = spotLight.getColor();
                    if (ImGui::ColorEdit3("Color", &color.x)) {
                        spotLight.setColor(color);
                    }
                    
                    float intensity = spotLight.getIntensity();
                    if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 5.0f)) {
                        spotLight.setIntensity(intensity);
                    }
                    
                    float radius = spotLight.getRadius();
                    if (ImGui::SliderFloat("Radius", &radius, 1.0f, 30.0f)) {
                        spotLight.setRadius(radius);
                    }
                    
                    float innerCutoff = spotLight.getInnerCutoffDegrees();
                    float outerCutoff = spotLight.getOuterCutoffDegrees();
                    if (ImGui::SliderFloat("Inner Cutoff", &innerCutoff, 0.0f, 89.0f)) {
                        spotLight.setCutoffAngles(innerCutoff, outerCutoff);
                    }
                    if (ImGui::SliderFloat("Outer Cutoff", &outerCutoff, innerCutoff + 0.1f, 90.0f)) {
                        spotLight.setCutoffAngles(innerCutoff, outerCutoff);
                    }
                    
                    const char* falloffTypes[] = {"Custom", "Sharp", "Normal", "Smooth", "Linear", "Quadratic"};
                    int currentFalloff = static_cast<int>(spotLight.getFalloffType());
                    if (ImGui::Combo("Falloff Type", &currentFalloff, falloffTypes, 6)) {
                        spotLight.setFalloffType(static_cast<GLR::SpotLight::FalloffType>(currentFalloff));
                    }
                    
                    ImGui::TreePop();
                }
            }
        }
        
        ImGui::End();
    }
    
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    ImVec2 window_pos = ImVec2(viewport->Pos.x + viewport->Size.x - 10.0f, viewport->Pos.y + 10.0f);
    ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f);
    
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.8f);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    
    if (ImGui::Begin("Performance", nullptr, window_flags)) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::End();
    }
}

void toggleLightVisibility() {
    if (directionalLight && directionalLight->hasComponent<GLR::DirectionalLight>()) {
        auto& dirLight = directionalLight->getComponent<GLR::DirectionalLight>();
        dirLight.setActive(demoSettings.showDirectionalLight);
    }
    
    for (auto& light : pointLights) {
        if (light && light->hasComponent<GLR::PointLight>()) {
            auto& pointLight = light->getComponent<GLR::PointLight>();
            pointLight.setActive(demoSettings.showPointLights);
        }
    }
    
    for (auto& light : spotLights) {
        if (light && light->hasComponent<GLR::SpotLight>()) {
            auto& spotLight = light->getComponent<GLR::SpotLight>();
            spotLight.setActive(demoSettings.showSpotLights);
        }
    }
}

int main(int argc, char* argv[]) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwSetErrorCallback(glfwErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

    window = glfwCreateWindow(width, height, "GLare Engine - Lighting Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, glfwFramebufferSizeCallback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    scene = std::make_shared<GLR::Scene>();
    
    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    renderer = std::make_shared<GLR::Renderer>(framebuffer_width, framebuffer_height);

    GLR::Renderer::Settings settings = renderer->getSettings();
    settings.renderDebug = false;
    settings.wireframeMode = false;
    settings.enableFaceCulling = true;
    settings.frustumCulling = true;
    settings.enablePostProcessing = true;
    settings.enableBloom = true;
    settings.bloomIntensity = 1.2f;
    settings.bloomThreshold = 1.0f;
    settings.postProcessing.exposure = 0.5f;
    renderer->updateSettings(settings);

    std::vector<std::string> skyboxFaces = {
        "./res/skybox/ocean/px.png",
        "./res/skybox/ocean/nx.png",
        "./res/skybox/ocean/py.png",
        "./res/skybox/ocean/ny.png",
        "./res/skybox/ocean/pz.png",
        "./res/skybox/ocean/nz.png"
    };
    auto skybox = scene->createEntity("Skybox");
    skybox->addComponent<GLR::SkyboxRenderer>(skyboxFaces);

    mainCamera = scene->createEntity("MainCamera");
    mainCamera->addComponent<GLR::Transform>(glm::vec3(0.0f, 8.0f, 15.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    mainCamera->addComponent<GLR::CameraComponent>(width, height, 45.0f, 0.1f, 100.0f);
    scene->setMainCamera(mainCamera);

    createDemoObjects();
    createLights();

    lastFrame = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        currentFrame = glfwGetTime();
        GLR::TimeStep ts;
        ts.updateTimeStep(lastFrame, currentFrame);
        float deltaTime = ts.getDeltaTime();
        lastFrame = currentFrame;

        processInput(window, deltaTime);
        updateCamera();
        updateLights(deltaTime);
        toggleLightVisibility();
        scene->update(deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderImGui();

        renderer->render(*scene, GLR::Color::Black());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    scene->clearEntities();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}