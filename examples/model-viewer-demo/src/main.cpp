#include "GLare.hpp"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>
#include <iostream>
#include <fstream>

int width = 1280, height = 720;
float lastFrame, currentFrame;
int vsync = 1;
GLFWwindow* window = nullptr;

std::shared_ptr<GLR::Scene> scene;
std::shared_ptr<GLR::Renderer> renderer;

std::shared_ptr<GLR::Entity> mainCamera;
std::shared_ptr<GLR::Entity> directionalLight;
std::vector<std::shared_ptr<GLR::Entity>> pointLights;

std::shared_ptr<GLR::Entity> loadedModel;
std::shared_ptr<GLR::Model> modelAsset;
std::string currentModelPath;
bool modelLoaded = false;

float cameraDistance = 10.0f;
float cameraAngle = 0.0f;
glm::vec3 cameraTarget = glm::vec3(0.0f);
float cameraFOV = 45.0f;
float cameraNear = 0.1f;
float cameraFar = 100.0f;

bool mouseDragging = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;
float cameraAngleX = 0.0f;
float cameraAngleY = 0.0f;

struct DemoSettings {
    bool renderDebug = false;
    bool wireframeMode = false;
    bool enableFaceCulling = false;
    bool forceSingleSided = false;
    bool frustumCulling = false;
    bool enablePostProcessing = true;
    bool enableBloom = true;
    float bloomIntensity = 1.2f;
    float bloomThreshold = 1.0f;
        
    struct PostProcessingSettings {
        float gamma = 2.2f;
        float exposure = 1.0f;
        float saturation = 1.0f;
        float contrast = 1.0f;  
        float brightness = 0.0f;
        float vibrancy = 0.0f;  
        float colorBoost = 1.0f;
    } postProcessing;
    
    int renderMode = 0;
    
    bool showDirectionalLight = true;
    bool showDirectionalLightShadowMap = false;
    bool showPointLights = true;
    float globalLightIntensity = 1.0f;
    
    glm::vec3 modelPosition = glm::vec3(0.0f);
    glm::vec3 modelRotation = glm::vec3(0.0f);
    glm::vec3 modelScale = glm::vec3(1.0f);
    bool autoRotateModel = false;
    float modelRotationSpeed = 0.5f;
    
    float animationSpeed = 0.5f;
    float blendDuration = 0.25f;
    
    bool showPerformanceStats = true;
    bool useMouseLook = true;
} demoSettings;

struct AnimationControllerState {
    bool showController = true;
    bool isPaused = false;
    float scrubTime = 0.0f;
    bool isScrubbing = false;
    int selectedAnimationIndex = -1;
    float playbackRate = 1.0f;
    bool showTimeline = true;
    bool showBlendingControls = true;
    float customBlendDuration = 0.25f;
    bool autoSwitchAnimations = false;
    float autoSwitchInterval = 5.0f;
    float autoSwitchTimer = 0.0f;
    
    std::vector<int> animationQueue;
    int queuePosition = -1;
} animController;

std::string formatTime(float seconds) {
    int minutes = (int)(seconds / 60.0f);
    int secs = (int)(seconds) % 60;
    int millisecs = (int)((seconds - (int)seconds) * 100);
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%02d:%02d.%02d", minutes, secs, millisecs);
    return std::string(buffer);
}

float getCurrentAnimationDuration() {
    if (!modelAsset || !modelAsset->getAnimationManager()) return 0.0f;
    
    auto animationNames = modelAsset->getAnimationManager()->getAnimationNames();
    auto animations = modelAsset->getAnimationManager()->getAnimations();
    
    std::string currentAnim = modelAsset->getAnimationManager()->getCurrentAnimationName();
    for (size_t i = 0; i < animations.size(); i++) {
        if (animations[i].name == currentAnim) {
            return animations[i].duration;
        }
    }
    return 0.0f;
}

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

void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT && demoSettings.useMouseLook) {
        if (action == GLFW_PRESS) {
            mouseDragging = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        } else if (action == GLFW_RELEASE) {
            mouseDragging = false;
        }
    }
}

void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    cameraDistance -= yoffset * 0.5f;
    cameraDistance = glm::clamp(cameraDistance, 1.0f, 100.0f);
}

void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    if (mouseDragging && demoSettings.useMouseLook) {
        double deltaX = xpos - lastMouseX;
        double deltaY = ypos - lastMouseY;
        
        cameraAngleY -= deltaX * 0.01f;
        cameraAngleX += deltaY * 0.01f;
        
        cameraAngleX = glm::clamp(cameraAngleX, -1.5f, 1.5f);
        
        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

void updateCamera() {
    auto& cameraTransform = mainCamera->getComponent<GLR::Transform>();
    
    if (demoSettings.useMouseLook) {
        float x = cameraDistance * cos(cameraAngleX) * sin(cameraAngleY);
        float y = cameraDistance * sin(cameraAngleX);
        float z = cameraDistance * cos(cameraAngleX) * cos(cameraAngleY);
        
        glm::vec3 cameraPos(x, y, z);
        cameraPos += cameraTarget;
        cameraTransform.setPosition(cameraPos);
        
        glm::vec3 direction = glm::normalize(cameraTarget - cameraPos);
        float yaw = atan2(-direction.x, -direction.z);
        float pitch = asin(direction.y);
        
        cameraTransform.setRotation(glm::vec3(glm::degrees(pitch), glm::degrees(yaw), 0.0f));
    } 
    else {
        float x = cos(cameraAngle) * cameraDistance;
        float z = sin(cameraAngle) * cameraDistance;
        
        cameraTransform.setPosition(glm::vec3(x, 5.0f, z) + cameraTarget);
        
        glm::vec3 cameraPos = glm::vec3(x, 5.0f, z) + cameraTarget;
        glm::vec3 direction = glm::normalize(cameraTarget - cameraPos);
        
        float yaw = atan2(-direction.x, -direction.z);
        float pitch = asin(direction.y);
        
        cameraTransform.setRotation(glm::vec3(glm::degrees(pitch), glm::degrees(yaw), 0.0f));
    }
    
    auto& camera = mainCamera->getComponent<GLR::CameraComponent>();
    camera.setFOV(cameraFOV);
    camera.setNear(cameraNear);
    camera.setFar(cameraFar);
}

void createLights() {
    directionalLight = scene->createEntity("DirectionalLight");
    directionalLight->addComponent<GLR::Transform>(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(-45.0f, 45.0f, 0.0f), glm::vec3(1.0f));
    directionalLight->addComponent<GLR::DirectionalLight>(glm::vec3(-0.5f, -1.0f, -0.3f), glm::vec3(1.0f, 0.98f, 0.9f), 0.8f, false, 20.0f, 50.0f, 200.0f, 4096);

    for (int i = 0; i < 3; i++) {
        float angle = (i / 3.0f) * 2.0f * M_PI;
        float x = cos(angle) * 15.0f;
        float z = sin(angle) * 15.0f;
        
        auto pointLight = scene->createEntity("FillLight_" + std::to_string(i));
        pointLight->addComponent<GLR::Transform>(glm::vec3(x, 8.0f, z), glm::vec3(0.0f), glm::vec3(1.0f));
        
        glm::vec3 colors[] = {glm::vec3(1.0f, 0.9f, 0.8f), glm::vec3(0.8f, 0.9f, 1.0f), glm::vec3(0.9f, 0.95f, 0.9f) };
        
        pointLight->addComponent<GLR::PointLight>(25.0f, colors[i], 0.3f, false, GLR::PointLight::FalloffType::SMOOTH, 512);
        pointLights.push_back(pointLight);
    }
}

void updateLights(float deltaTime) {
    auto& dirLight = directionalLight->getComponent<GLR::DirectionalLight>();
    dirLight.setIntensity(0.8f * demoSettings.globalLightIntensity);
    
    for (auto& light : pointLights) {
        auto& pointLight = light->getComponent<GLR::PointLight>();
        pointLight.setIntensity(0.3f * demoSettings.globalLightIntensity);
        pointLight.setActive(demoSettings.showPointLights);
    }
    
    dirLight.setActive(demoSettings.showDirectionalLight);
    dirLight.setCastShadows(demoSettings.showDirectionalLightShadowMap);
}

bool loadModel(const std::string& path) {
    try {
        if (loadedModel) {
            scene->removeEntity(loadedModel);
            loadedModel.reset();
        }

        auto shader = renderer->getObjectShader();
        
        modelAsset = std::make_shared<GLR::Model>();
        modelAsset->create(path, shader);
        
        if (!modelAsset) {
            std::cerr << "Failed to load model: " << path << std::endl;
            return false;
        }
        
        loadedModel = scene->createEntity("LoadedModel");
        loadedModel->addComponent<GLR::Transform>(demoSettings.modelPosition, demoSettings.modelRotation, demoSettings.modelScale);
        loadedModel->addComponent<GLR::ModelRenderer>(modelAsset);
        
        if (modelAsset->getAnimationManager()) {
            modelAsset->getAnimationManager()->setBlendDuration(demoSettings.blendDuration);
            modelAsset->getAnimationManager()->setSpeed(demoSettings.animationSpeed);
            
            auto animNames = modelAsset->getAnimationManager()->getAnimationNames();
            if (!animNames.empty()) {
                modelAsset->getAnimationManager()->playAnimation(animNames[0], true);
            }
        }
        
        currentModelPath = path;
        modelLoaded = true;
        
        auto bounds = modelAsset->getBoundingBox();
        if (bounds.isValid()) {
            glm::vec3 size = bounds.getSize();
            float maxDimension = glm::max(glm::max(size.x, size.y), size.z);
            cameraDistance = maxDimension * 2.5f;
            cameraTarget = bounds.getCenter();
        }
        
        std::cout << "Successfully loaded model: " << path << std::endl;
        if (modelAsset->getAnimationManager()) {
            auto animNames = modelAsset->getAnimationManager()->getAnimationNames();
            std::cout << "Model has " << animNames.size() << " animations" << std::endl;
        }
        
        return true;
        
    } 
    catch (const std::exception& e) {
        std::cerr << "Exception loading model: " << e.what() << std::endl;
        return false;
    }
}

void updateModel(float deltaTime) {
    if (!loadedModel) return;
    
    auto& transform = loadedModel->getComponent<GLR::Transform>();
    
    transform.setPosition(demoSettings.modelPosition);
    transform.setScale(demoSettings.modelScale);
    
    glm::vec3 rotation = demoSettings.modelRotation;
    if (demoSettings.autoRotateModel) {
        rotation.y += demoSettings.modelRotationSpeed * deltaTime * 57.2958f;
        demoSettings.modelRotation.y = rotation.y;
    }
    transform.setRotation(rotation);
    
    if (modelAsset && modelAsset->getAnimationManager()) {
        modelAsset->getAnimationManager()->setSpeed(demoSettings.animationSpeed);
        modelAsset->getAnimationManager()->setBlendDuration(demoSettings.blendDuration);
        modelAsset->getAnimationManager()->update(deltaTime);
    }
}

void updateRendererSettings() {
    auto settings = renderer->getSettings();
    
    settings.renderDebug = demoSettings.renderDebug;
    settings.wireframeMode = demoSettings.wireframeMode;
    settings.enableFaceCulling = demoSettings.enableFaceCulling;
    settings.forceSingleSided = demoSettings.forceSingleSided;
    settings.frustumCulling = demoSettings.frustumCulling;
    settings.enablePostProcessing = demoSettings.enablePostProcessing;
    settings.enableBloom = demoSettings.enableBloom;
    settings.bloomIntensity = demoSettings.bloomIntensity;
    settings.bloomThreshold = demoSettings.bloomThreshold;
    
    settings.postProcessing.gamma = demoSettings.postProcessing.gamma;
    settings.postProcessing.exposure = demoSettings.postProcessing.exposure;
    settings.postProcessing.saturation = demoSettings.postProcessing.saturation;
    settings.postProcessing.contrast = demoSettings.postProcessing.contrast;
    settings.postProcessing.brightness = demoSettings.postProcessing.brightness;
    settings.postProcessing.vibrancy = demoSettings.postProcessing.vibrancy;
    settings.postProcessing.colorBoost = demoSettings.postProcessing.colorBoost;
    
    switch (demoSettings.renderMode) {
        case 0: settings.renderMode = GLR::Renderer::Settings::RenderMode::DEFAULT; break;
        case 1: settings.renderMode = GLR::Renderer::Settings::RenderMode::ALBEDO; break;
        case 2: settings.renderMode = GLR::Renderer::Settings::RenderMode::NORMAL; break;
        case 3: settings.renderMode = GLR::Renderer::Settings::RenderMode::ROUGHNESS; break;
        case 4: settings.renderMode = GLR::Renderer::Settings::RenderMode::METALLIC; break;
        case 5: settings.renderMode = GLR::Renderer::Settings::RenderMode::LIGHT; break;
        case 6: settings.renderMode = GLR::Renderer::Settings::RenderMode::SHADOW; break;
    }
    
    renderer->updateSettings(settings);
}

void renderAnimationController() {
    if (!modelAsset || !modelAsset->getAnimationManager() || !animController.showController) return;
    
    auto animManager = modelAsset->getAnimationManager();
    auto animationNames = animManager->getAnimationNames();
    auto animations = animManager->getAnimations();
    
    if (animationNames.empty()) {
        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Animation Controller", &animController.showController)) {
            ImGui::Text("No animations available in this model.");
            ImGui::End();
        }
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(450, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Animation Controller", &animController.showController, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        ImGui::Text("Animation Controller");
        ImGui::Separator();
        
        std::string currentAnim = animManager->getCurrentAnimationName();
        float currentTime = animManager->getCurrentTime();
        float duration = getCurrentAnimationDuration();
        bool isPlaying = animManager->getIsPlaying() && !animController.isPaused;
        bool isBlending = animManager->getIsBlending();
        
        ImGui::Text("Current: %s", currentAnim.empty() ? "None" : currentAnim.c_str());
        ImGui::SameLine();
        ImGui::Text("| Status: %s%s", isPlaying ? "Playing" : "Stopped", isBlending ? " (Blending)" : "");
        
        ImGui::Text("Time: %s / %s", formatTime(currentTime).c_str(), formatTime(duration).c_str());
        
        ImGui::Separator();
        ImGui::Text("Playback Controls");
        
        float buttonWidth = 50.0f;
        ImVec2 buttonSize(buttonWidth, 30.0f);
        
        if (ImGui::Button("<<", buttonSize)) {
            if (animController.selectedAnimationIndex > 0) {
                animController.selectedAnimationIndex--;
                animManager->playAnimation(animationNames[animController.selectedAnimationIndex], true);
                animController.isPaused = false;
            }
        }
        ImGui::SameLine();
        
        const char* playPauseText = isPlaying ? "||" : ">";
        if (ImGui::Button(playPauseText, buttonSize)) {
            if (currentAnim.empty() && !animationNames.empty()) {
                animController.selectedAnimationIndex = 0;
                animManager->playAnimation(animationNames[0], true);
                animController.isPaused = false;
            } 
            else {
                animController.isPaused = !animController.isPaused;
            }
        }
        ImGui::SameLine();
        
        if (ImGui::Button("[]", buttonSize)) {
            animManager->stopAnimation();
            animController.isPaused = false;
            animController.scrubTime = 0.0f;
        }
        ImGui::SameLine();
        
        if (ImGui::Button(">>", buttonSize)) {
            if (animController.selectedAnimationIndex < (int)animationNames.size() - 1) {
                animController.selectedAnimationIndex++;
                animManager->playAnimation(animationNames[animController.selectedAnimationIndex], true);
                animController.isPaused = false;
            }
        }
        ImGui::SameLine();
        
        ImGui::Text("Speed:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        if (ImGui::SliderFloat("##speed", &animController.playbackRate, 0.1f, 3.0f, "%.1fx")) {
            demoSettings.animationSpeed = animController.playbackRate;
        }
        
        if (animController.showTimeline && duration > 0.0f) {
            ImGui::Separator();
            ImGui::Text("Timeline");
            
            float timelineValue = animController.isScrubbing ? animController.scrubTime : currentTime;
            
            ImGui::SetNextItemWidth(-1);
            if (ImGui::SliderFloat("##timeline", &timelineValue, 0.0f, duration, formatTime(timelineValue).c_str())) {
                animController.scrubTime = timelineValue;
                animController.isScrubbing = true;
            }
            
            if (ImGui::IsItemActive()) {
                animController.isScrubbing = true;
            } 
            else if (animController.isScrubbing) {
                animController.isScrubbing = false;
            }
        }
        
        ImGui::Separator();
        ImGui::Text("Animation List (%d total)", (int)animationNames.size());
        
        ImGui::BeginChild("AnimationListChild", ImVec2(0, 150), true);
        
        for (size_t i = 0; i < animationNames.size(); i++) {
            const std::string& animName = animationNames[i];
            
            bool isCurrentAnim = (currentAnim == animName);
            if (isCurrentAnim) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
            }
            
            ImGui::PushID((int)i);
            
            if (ImGui::Button(">", ImVec2(20, 20))) {
                animController.selectedAnimationIndex = (int)i;
                animManager->playAnimation(animName, true);
                animController.isPaused = false;
            }
            ImGui::SameLine();
            
            float animDuration = (i < animations.size()) ? animations[i].duration : 0.0f;
            ImGui::Text("%s (%s)", animName.c_str(), formatTime(animDuration).c_str());
            
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Play Once")) {
                    animManager->playAnimation(animName, false);
                }
                if (ImGui::MenuItem("Loop")) {
                    animManager->playAnimation(animName, true);
                }
                if (ImGui::MenuItem("Add to Queue")) {
                    animController.animationQueue.push_back((int)i);
                }
                ImGui::EndPopup();
            }
            
            ImGui::PopID();
            
            if (isCurrentAnim) {
                ImGui::PopStyleColor();
            }
        }
        
        ImGui::EndChild();
        
        if (animController.showBlendingControls) {
            if (ImGui::CollapsingHeader("Blending & Transitions")) {
                ImGui::Text("Blend Duration:");
                ImGui::SetNextItemWidth(150);
                if (ImGui::SliderFloat("##blendDuration", &animController.customBlendDuration, 0.0f, 2.0f, "%.2fs")) {
                    demoSettings.blendDuration = animController.customBlendDuration;
                }
                
                ImGui::Checkbox("Auto-switch Animations", &animController.autoSwitchAnimations);
                if (animController.autoSwitchAnimations) {
                    ImGui::SetNextItemWidth(150);
                    ImGui::SliderFloat("Switch Interval", &animController.autoSwitchInterval, 1.0f, 30.0f, "%.1fs");
                    ImGui::Text("Next switch in: %.1fs", animController.autoSwitchInterval - animController.autoSwitchTimer);
                }
            }
        }
        
        ImGui::End();
    }
}

void updateAnimationController(float deltaTime) {
    if (!modelAsset || !modelAsset->getAnimationManager()) return;
    
    auto animManager = modelAsset->getAnimationManager();
    auto animationNames = animManager->getAnimationNames();
    
    if (animController.isPaused) {
        if (animController.isPaused && demoSettings.animationSpeed > 0.001f) {
            animController.playbackRate = demoSettings.animationSpeed;
            demoSettings.animationSpeed = 0.001f;
        }
    } 
    else if (demoSettings.animationSpeed < 0.01f && animController.playbackRate > 0.01f) {
        demoSettings.animationSpeed = animController.playbackRate;
    }
    
    if (animController.autoSwitchAnimations && !animationNames.empty()) {
        animController.autoSwitchTimer += deltaTime;
        
        if (animController.autoSwitchTimer >= animController.autoSwitchInterval) {
            animController.autoSwitchTimer = 0.0f;
            
            animController.selectedAnimationIndex = 
                (animController.selectedAnimationIndex + 1) % (int)animationNames.size();
            animManager->playAnimation(animationNames[animController.selectedAnimationIndex], true);
        }
    }
    
    if (std::abs(demoSettings.animationSpeed - animController.playbackRate) > 0.001f && 
        !animController.isPaused) {
        animController.playbackRate = demoSettings.animationSpeed;
    }
}

void renderModelControls() {
    if (!modelLoaded) return;
    
    ImGui::SetNextWindowSize(ImVec2(320, 450), ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    
    if (ImGui::Begin("Model Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.3f, 0.8f, 0.3f));
            
            ImGui::Text("X Position");
            ImGui::PushItemWidth(-50);
            ImGui::DragFloat("##posX", &demoSettings.modelPosition.x, 0.1f, -10.0f, 10.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::SmallButton("0##posX")) demoSettings.modelPosition.x = 0.0f;
            
            ImGui::Text("Y Position");
            ImGui::PushItemWidth(-50);
            ImGui::DragFloat("##posY", &demoSettings.modelPosition.y, 0.1f, -10.0f, 10.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::SmallButton("0##posY")) demoSettings.modelPosition.y = 0.0f;
            
            ImGui::Text("Z Position");
            ImGui::PushItemWidth(-50);
            ImGui::DragFloat("##posZ", &demoSettings.modelPosition.z, 0.1f, -10.0f, 10.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::SmallButton("0##posZ")) demoSettings.modelPosition.z = 0.0f;
            
            ImGui::PopStyleColor();
        }
        
        ImGui::Spacing();
        
        if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.8f, 0.3f, 0.2f, 0.3f));
            
            ImGui::Text("X Rotation");
            ImGui::PushItemWidth(-50);
            ImGui::DragFloat("##rotX", &demoSettings.modelRotation.x, 1.0f, -180.0f, 180.0f, "%.1f°");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::SmallButton("0##rotX")) demoSettings.modelRotation.x = 0.0f;
            
            ImGui::Text("Y Rotation");
            ImGui::PushItemWidth(-50);
            ImGui::DragFloat("##rotY", &demoSettings.modelRotation.y, 1.0f, -180.0f, 180.0f, "%.1f°");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::SmallButton("0##rotY")) demoSettings.modelRotation.y = 0.0f;
            
            ImGui::Text("Z Rotation");
            ImGui::PushItemWidth(-50);
            ImGui::DragFloat("##rotZ", &demoSettings.modelRotation.z, 1.0f, -180.0f, 180.0f, "%.1f°");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::SmallButton("0##rotZ")) demoSettings.modelRotation.z = 0.0f;
            
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            ImGui::Text("Quick Rotations:");
            
            ImGui::BeginTable("RotationTable", 3, ImGuiTableFlags_SizingFixedFit);
            ImGui::TableSetupColumn("Axis", ImGuiTableColumnFlags_WidthFixed, 30);
            ImGui::TableSetupColumn("90°", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("180°", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableHeadersRow();
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("X");
            ImGui::TableNextColumn();
            if (ImGui::Button("90°##X", ImVec2(55, 22))) {
                demoSettings.modelRotation.x = fmod(demoSettings.modelRotation.x + 90.0f + 180.0f, 360.0f) - 180.0f;
            }
            ImGui::TableNextColumn();
            if (ImGui::Button("180°##X", ImVec2(55, 22))) {
                demoSettings.modelRotation.x = fmod(demoSettings.modelRotation.x + 180.0f + 180.0f, 360.0f) - 180.0f;
            }
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Y");
            ImGui::TableNextColumn();
            if (ImGui::Button("90°##Y", ImVec2(55, 22))) {
                demoSettings.modelRotation.y = fmod(demoSettings.modelRotation.y + 90.0f + 180.0f, 360.0f) - 180.0f;
            }
            ImGui::TableNextColumn();
            if (ImGui::Button("180°##Y", ImVec2(55, 22))) {
                demoSettings.modelRotation.y = fmod(demoSettings.modelRotation.y + 180.0f + 180.0f, 360.0f) - 180.0f;
            }
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Z");
            ImGui::TableNextColumn();
            if (ImGui::Button("90°##Z", ImVec2(55, 22))) {
                demoSettings.modelRotation.z = fmod(demoSettings.modelRotation.z + 90.0f + 180.0f, 360.0f) - 180.0f;
            }
            ImGui::TableNextColumn();
            if (ImGui::Button("180°##Z", ImVec2(55, 22))) {
                demoSettings.modelRotation.z = fmod(demoSettings.modelRotation.z + 180.0f + 180.0f, 360.0f) - 180.0f;
            }
            
            ImGui::EndTable();
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Checkbox("Auto Rotate", &demoSettings.autoRotateModel);
            if (demoSettings.autoRotateModel) {
                ImGui::Text("Speed:");
                ImGui::SameLine();
                ImGui::PushItemWidth(150);
                ImGui::DragFloat("##rotSpeed", &demoSettings.modelRotationSpeed, 0.01f, 0.1f, 5.0f, "%.2fx");
                ImGui::PopItemWidth();
            }
        }
        
        ImGui::Spacing();
        
        if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.8f, 0.3f, 0.3f));
            
            static bool uniformScale = true;
            ImGui::Checkbox("Uniform Scale", &uniformScale);
            
            if (uniformScale) {
                float uniformScaleValue = demoSettings.modelScale.x;
                ImGui::Text("Scale");
                ImGui::PushItemWidth(-50);
                if (ImGui::DragFloat("##scaleUniform", &uniformScaleValue, 0.01f, 0.01f, 10.0f, "%.2fx")) {
                    demoSettings.modelScale = glm::vec3(uniformScaleValue);
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::SmallButton("1##scaleUniform")) {
                    demoSettings.modelScale = glm::vec3(1.0f);
                }
            } else {
                ImGui::Text("X Scale");
                ImGui::PushItemWidth(-50);
                ImGui::DragFloat("##scaleX", &demoSettings.modelScale.x, 0.01f, 0.01f, 10.0f, "%.2fx");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::SmallButton("1##scaleX")) demoSettings.modelScale.x = 1.0f;
                
                ImGui::Text("Y Scale");
                ImGui::PushItemWidth(-50);
                ImGui::DragFloat("##scaleY", &demoSettings.modelScale.y, 0.01f, 0.01f, 10.0f, "%.2fx");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::SmallButton("1##scaleY")) demoSettings.modelScale.y = 1.0f;
                
                ImGui::Text("Z Scale");
                ImGui::PushItemWidth(-50);
                ImGui::DragFloat("##scaleZ", &demoSettings.modelScale.z, 0.01f, 0.01f, 10.0f, "%.2fx");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::SmallButton("1##scaleZ")) demoSettings.modelScale.z = 1.0f;
            }
            
            ImGui::PopStyleColor();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Reset Options:");
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.2f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.3f, 0.9f));
        
        if (ImGui::Button("Reset All", ImVec2(100, 30))) {
            demoSettings.modelPosition = glm::vec3(0.0f);
            demoSettings.modelRotation = glm::vec3(0.0f);
            demoSettings.modelScale = glm::vec3(1.0f);
            demoSettings.autoRotateModel = false;
            demoSettings.modelRotationSpeed = 1.0f;
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Reset Pos", ImVec2(80, 30))) {
            demoSettings.modelPosition = glm::vec3(0.0f);
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Reset Rot", ImVec2(80, 30))) {
            demoSettings.modelRotation = glm::vec3(0.0f);
            demoSettings.autoRotateModel = false;
        }
        
        ImGui::PopStyleColor(2);
        
        ImGui::End();
    }
    
    ImGui::PopStyleVar();
}

void renderRenderingControls() {
    ImGui::SetNextWindowSize(ImVec2(380, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Rendering & Post Processing", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        if (ImGui::CollapsingHeader("Basic Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool settingsChanged = false;
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
            ImGui::Text("Debug & Visualization:");
            ImGui::PopStyleColor();
            
            if (ImGui::Checkbox("Render Debug", &demoSettings.renderDebug)) {
                settingsChanged = true;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Enable debug rendering overlays");
            }
            
            if (ImGui::Checkbox("Wireframe Mode", &demoSettings.wireframeMode)) {
                settingsChanged = true;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Render models in wireframe instead of solid");
            }
            
            ImGui::Separator();
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 1.0f, 1.0f));
            ImGui::Text("Culling Options:");
            ImGui::PopStyleColor();
            
            if (ImGui::Checkbox("Face Culling", &demoSettings.enableFaceCulling)) {
                settingsChanged = true;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Cull back-facing triangles (improves performance)");
            }
            
            if (ImGui::Checkbox("Force Single Sided", &demoSettings.forceSingleSided)) {
                settingsChanged = true;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Force all materials to be single-sided");
            }
            
            if (ImGui::Checkbox("Frustum Culling", &demoSettings.frustumCulling)) {
                settingsChanged = true;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Cull objects outside camera view (improves performance)");
            }
            
            ImGui::Separator();
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 1.0f, 1.0f));
            ImGui::Text("Render Mode:");
            ImGui::PopStyleColor();
            
            const char* renderModeNames[] = {
                "Default", "Albedo", "Normal", "Roughness", "Metallic", "Light", "Shadow"
            };
            const char* renderModeTooltips[] = {
                "Standard PBR rendering",
                "Show only albedo/diffuse colors",
                "Visualize surface normals",
                "Show roughness values (white = rough, black = smooth)",
                "Show metallic values (white = metallic, black = dielectric)",
                "Show lighting contribution only",
                "Show shadow information"
            };
            
            if (ImGui::Combo("##renderMode", &demoSettings.renderMode, renderModeNames, 7)) {
                settingsChanged = true;
            }
            if (ImGui::IsItemHovered() && demoSettings.renderMode < 7) {
                ImGui::SetTooltip("%s", renderModeTooltips[demoSettings.renderMode]);
            }
            
            if (settingsChanged) {
                updateRendererSettings();
            }
        }
        
        if (ImGui::CollapsingHeader("Post Processing", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool ppChanged = false;
            
            if (ImGui::Checkbox("Enable Post Processing", &demoSettings.enablePostProcessing)) {
                ppChanged = true;
            }
            
            if (demoSettings.enablePostProcessing) {                
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.3f, 0.8f, 0.5f));
                if (ImGui::CollapsingHeader("Tone Mapping", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Indent();
                    ImGui::Text("Gamma Correction:");
                    ImGui::SetNextItemWidth(200);
                    if (ImGui::SliderFloat("##gamma", &demoSettings.postProcessing.gamma, 1.0f, 3.5f, "%.2f")) {
                        ppChanged = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Reset##gamma")) {
                        demoSettings.postProcessing.gamma = 2.2f;
                        ppChanged = true;
                    }
                    
                    ImGui::Text("Exposure:");
                    ImGui::SetNextItemWidth(200);
                    if (ImGui::SliderFloat("##exposure", &demoSettings.postProcessing.exposure, 0.1f, 5.0f, "%.2f")) {
                        ppChanged = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Reset##exposure")) {
                        demoSettings.postProcessing.exposure = 1.0f;
                        ppChanged = true;
                    }
                    ImGui::Unindent();

                }
                ImGui::PopStyleColor();
                
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.4f, 0.2f, 0.5f));
                if (ImGui::CollapsingHeader("Color Enhancement", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Indent();
                    ImGui::Text("Saturation:");
                    ImGui::SetNextItemWidth(200);
                    if (ImGui::SliderFloat("##saturation", &demoSettings.postProcessing.saturation, 0.0f, 2.0f, "%.2f")) {
                        ppChanged = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Reset##saturation")) {
                        demoSettings.postProcessing.saturation = 1.0f;
                        ppChanged = true;
                    }
                    
                    ImGui::Text("Contrast:");
                    ImGui::SetNextItemWidth(200);
                    if (ImGui::SliderFloat("##contrast", &demoSettings.postProcessing.contrast, 0.0f, 2.0f, "%.2f")) {
                        ppChanged = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Reset##contrast")) {
                        demoSettings.postProcessing.contrast = 1.0f;
                        ppChanged = true;
                    }
                    
                    ImGui::Text("Brightness:");
                    ImGui::SetNextItemWidth(200);
                    if (ImGui::SliderFloat("##brightness", &demoSettings.postProcessing.brightness, -1.0f, 1.0f, "%.2f")) {
                        ppChanged = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Reset##brightness")) {
                        demoSettings.postProcessing.brightness = 0.0f;
                        ppChanged = true;
                    }
                    
                    ImGui::Text("Vibrancy:");
                    ImGui::SetNextItemWidth(200);
                    if (ImGui::SliderFloat("##vibrancy", &demoSettings.postProcessing.vibrancy, 0.0f, 2.0f, "%.2f")) {
                        ppChanged = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Reset##vibrancy")) {
                        demoSettings.postProcessing.vibrancy = 0.0f;
                        ppChanged = true;
                    }
                    
                    ImGui::Text("Color Boost:");
                    ImGui::SetNextItemWidth(200);
                    if (ImGui::SliderFloat("##colorBoost", &demoSettings.postProcessing.colorBoost, 0.0f, 3.0f, "%.2f")) {
                        ppChanged = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Reset##colorBoost")) {
                        demoSettings.postProcessing.colorBoost = 1.0f;
                        ppChanged = true;
                    }
                    ImGui::Unindent();
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.29f, 0.62f, 0.72f, 1.0f));
                if (ImGui::CollapsingHeader("Bloom Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Indent();
                    bool bloomChanged = false;
            
                    if (ImGui::Checkbox("Enable Bloom", &demoSettings.enableBloom)) {
                        bloomChanged = true;
                    }
                    
                    if (demoSettings.enableBloom) {                        
                        ImGui::Text("Bloom Intensity:");
                        ImGui::SetNextItemWidth(200);
                        if (ImGui::SliderFloat("##bloomIntensity", &demoSettings.bloomIntensity, 0.0f, 3.0f, "%.2f")) {
                            bloomChanged = true;
                        }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("Reset##bloomIntensity")) {
                            demoSettings.bloomIntensity = 1.2f;
                            bloomChanged = true;
                        }
                        
                        ImGui::Text("Bloom Threshold:");
                        ImGui::SetNextItemWidth(200);
                        if (ImGui::SliderFloat("##bloomThreshold", &demoSettings.bloomThreshold, 0.0f, 3.0f, "%.2f")) {
                            bloomChanged = true;
                        }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("Reset##bloomThreshold")) {
                            demoSettings.bloomThreshold = 1.0f;
                            bloomChanged = true;
                        }
                    }
                    
                    if (bloomChanged) {
                        updateRendererSettings();
                    }
                    ImGui::Unindent();
                }
                ImGui::PopStyleColor();
                                
                ImGui::Separator();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.4f, 0.4f, 0.9f));
                if (ImGui::Button("Reset All Post Processing", ImVec2(220, 30))) {
                    demoSettings.postProcessing.gamma = 2.2f;
                    demoSettings.postProcessing.exposure = 1.0f;
                    demoSettings.postProcessing.saturation = 1.0f;
                    demoSettings.postProcessing.contrast = 1.0f;
                    demoSettings.postProcessing.brightness = 0.0f;
                    demoSettings.postProcessing.vibrancy = 0.0f;
                    demoSettings.postProcessing.colorBoost = 1.0f;
                    ppChanged = true;
                }
                ImGui::PopStyleColor(2);
            }
            
            if (ppChanged) {
                updateRendererSettings();
            }
        }
        
        if (ImGui::CollapsingHeader("Lighting")) {
            ImGui::Checkbox("Show Directional Light", &demoSettings.showDirectionalLight);
            ImGui::Checkbox("Show Directional Light Shadow", &demoSettings.showDirectionalLightShadowMap);
            ImGui::Checkbox("Show Point Lights", &demoSettings.showPointLights);
            ImGui::SliderFloat("Global Light Intensity", &demoSettings.globalLightIntensity, 0.0f, 3.0f);
        }
    
        ImGui::End();
    }
}

void renderPerformanceStats() {
    if (!demoSettings.showPerformanceStats) return;
    
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 window_pos = ImVec2(viewport->Pos.x + viewport->Size.x - 10.0f, viewport->Pos.y + 10.0f);
    ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f);
    
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.8f);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    
    if (ImGui::Begin("Performance", &demoSettings.showPerformanceStats, window_flags)) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::End();
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <model_path>" << std::endl;
        std::cerr << "Supported formats: .gltf, .glb" << std::endl;
        return -1;
    }
    
    std::string modelPath = argv[1];
    
    if (!std::filesystem::exists(modelPath)) {
        std::cerr << "Error: Model file not found: " << modelPath << std::endl;
        return -1;
    }
    
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
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(width, height, "GLare Engine - Model Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, glfwFramebufferSizeCallback);
    glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
    glfwSetScrollCallback(window, glfwScrollCallback);
    glfwSetCursorPosCallback(window, glfwCursorPosCallback);
    glfwSwapInterval(vsync);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
    settings.renderDebug = demoSettings.renderDebug;
    settings.wireframeMode = demoSettings.wireframeMode;
    settings.enableFaceCulling = demoSettings.enableFaceCulling;
    settings.forceSingleSided = demoSettings.forceSingleSided;
    settings.frustumCulling = demoSettings.frustumCulling;
    settings.enablePostProcessing = demoSettings.enablePostProcessing;
    settings.enableBloom = demoSettings.enableBloom;
    settings.bloomIntensity = demoSettings.bloomIntensity;
    settings.bloomThreshold = demoSettings.bloomThreshold;
    
    settings.postProcessing.gamma = 2.2f;
    settings.postProcessing.exposure = 1.0f;
    settings.postProcessing.saturation = 1.0f;
    settings.postProcessing.contrast = 1.0f;
    settings.postProcessing.brightness = 0.0f;
    settings.postProcessing.vibrancy = 0.0f;
    settings.postProcessing.colorBoost = 1.0f;
    
    renderer->updateSettings(settings);

    mainCamera = scene->createEntity("MainCamera");
    mainCamera->addComponent<GLR::Transform>(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    mainCamera->addComponent<GLR::CameraComponent>(width, height, 45.0f, 0.1f, 100.0f);
    scene->setMainCamera(mainCamera);

    createLights();

    if (!loadModel(modelPath)) {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
    }

    std::vector<std::string> faces = {
        "./res/skybox/ocean/px.png",
        "./res/skybox/ocean/nx.png",
        "./res/skybox/ocean/py.png",
        "./res/skybox/ocean/ny.png",
        "./res/skybox/ocean/pz.png",
        "./res/skybox/ocean/nz.png"
    };
    auto skybox = scene->createEntity("skybox");
    skybox->addComponent<GLR::SkyboxRenderer>(faces);

    lastFrame = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        currentFrame = glfwGetTime();
        GLR::TimeStep ts;
        ts.updateTimeStep(lastFrame, currentFrame);
        float deltaTime = ts.getDeltaTime();
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        updateCamera();
        updateLights(deltaTime);
        updateModel(deltaTime);
        updateAnimationController(deltaTime);
        scene->update(deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderAnimationController();
        renderModelControls();
        renderRenderingControls();
        renderPerformanceStats();

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
    modelAsset.reset();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}