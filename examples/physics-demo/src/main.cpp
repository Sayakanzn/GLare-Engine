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
std::shared_ptr<GLR::Entity> mainCamera;

std::vector<std::shared_ptr<GLR::Entity>> dynamicSpheres;
std::vector<std::shared_ptr<GLR::Entity>> dynamicBoxes;
std::shared_ptr<GLR::Entity> groundPlane;
std::vector<std::shared_ptr<GLR::Entity>> staticObstacles;

float cameraDistance = 25.0f;
float cameraHeight = 12.0f;
float cameraAngle = 0.0f;
float cameraRotationSpeed = 0.8f;
bool autoRotateCamera = false;

struct PhysicsDemoSettings {
    float timeScale = 1.0f;
    float defaultBounciness = 0.3f;
    float defaultFriction = 0.5f;
    float defaultMass = 1.0f;
} demoSettings;

struct PhysicsStats {
    int totalRigidBodies = 0;
    int activeRigidBodies = 0;
    int sleepingRigidBodies = 0;
    int collisionEvents = 0;
} physicsStats;

class DemoCollisionResponder : public GLR::CollisionResponder {
protected:
    void handleCollisionEnter(GLR::Entity* otherEntity, const GLR::CollisionEvent& event) override {
        physicsStats.collisionEvents++;
    }
};

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

void updateCamera() {
    if (autoRotateCamera) {
        cameraAngle += 0.3f * (currentFrame - lastFrame);
    }
    
    float x = cos(cameraAngle) * cameraDistance;
    float z = sin(cameraAngle) * cameraDistance;
    
    auto& cameraTransform = mainCamera->getComponent<GLR::Transform>();
    cameraTransform.setPosition(glm::vec3(x, cameraHeight, z));
    
    glm::vec3 cameraPos = glm::vec3(x, cameraHeight, z);
    glm::vec3 target = glm::vec3(0.0f, 5.0f, 0.0f);
    glm::vec3 direction = glm::normalize(target - cameraPos);
    
    float yaw = atan2(-direction.x, -direction.z);
    float pitch = asin(direction.y);
    
    cameraTransform.setRotation(glm::vec3(glm::degrees(pitch), glm::degrees(yaw), 0.0f));
}

void spawnDynamicObjects() {
    auto shader = renderer->getObjectShader();
    
    auto sphereMesh = std::make_shared<GLR::Mesh>();
    sphereMesh->create(GLR::Shape::generateSphere(1.0f, 32), GLR::Shape::getSphereIndices(32), GLR::Shape::getStandardLayout());
    
    auto cubeMesh = std::make_shared<GLR::Mesh>();
    cubeMesh->create(GLR::Shape::generateCube(), GLR::Shape::getCubeIndices(), GLR::Shape::getStandardLayout());
    
    auto redMaterial = std::make_shared<GLR::Material>(shader);
    redMaterial->setVector4("baseColorFactor", glm::vec4(0.8f, 0.2f, 0.2f, 1.0));
    
    auto blueMaterial = std::make_shared<GLR::Material>(shader);
    blueMaterial->setVector4("baseColorFactor", glm::vec4(0.2f, 0.2f, 0.8f, 1.0));
    
    for (int i = 0; i < 12; i++) {
        float x = ((i % 4) - 1.5f) * 2.0f;
        float y = 15.0f + (i / 4) * 3.0f;
        float z = ((i / 4) - 1) * 2.0f;
        
        auto sphere = scene->createEntity("Sphere_" + std::to_string(i));
        sphere->addComponent<GLR::Transform>(glm::vec3(x, y, z), glm::vec3(0.0f), glm::vec3(1.0f));
        sphere->addComponent<GLR::MeshRenderer>(sphereMesh, redMaterial);
        sphere->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::DYNAMIC, true);
        
        auto& rigidBody = sphere->getComponent<GLR::RigidBody>();
        rigidBody.setMass(demoSettings.defaultMass);
        rigidBody.setLinearDamping(0.1f);
        rigidBody.setAngularDamping(0.1f);
        
        sphere->addComponent<GLR::SphereCollider>(1.0f);
        auto& collider = sphere->getComponent<GLR::SphereCollider>();
        collider.setBounciness(demoSettings.defaultBounciness);
        collider.setFriction(demoSettings.defaultFriction);
        
        sphere->addComponent<DemoCollisionResponder>();
        
        dynamicSpheres.push_back(sphere);
    }
    
    for (int i = 0; i < 8; i++) {
        float x = ((i % 4) - 1.5f) * 3.0f;
        float y = 20.0f + (i / 4) * 4.0f;
        float z = ((i / 4) - 0.5f) * 3.0f;
        
        auto box = scene->createEntity("Box_" + std::to_string(i));
        box->addComponent<GLR::Transform>(glm::vec3(x, y, z), glm::vec3(0.0f), glm::vec3(1.5f));
        box->addComponent<GLR::MeshRenderer>(cubeMesh, blueMaterial);
        box->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::DYNAMIC, true);
        
        auto& rigidBody = box->getComponent<GLR::RigidBody>();
        rigidBody.setMass(demoSettings.defaultMass * 2.0f);
        rigidBody.setLinearDamping(0.05f);
        rigidBody.setAngularDamping(0.1f);
        
        box->addComponent<GLR::BoxCollider>(glm::vec3(0.75f));
        auto& collider = box->getComponent<GLR::BoxCollider>();
        collider.setBounciness(demoSettings.defaultBounciness * 0.7f);
        collider.setFriction(demoSettings.defaultFriction * 1.5f);
        
        box->addComponent<DemoCollisionResponder>();
        
        dynamicBoxes.push_back(box);
    }
}

void createPhysicsObjects() {
    auto shader = renderer->getObjectShader();
    
    auto cubeMesh = std::make_shared<GLR::Mesh>();
    cubeMesh->create(GLR::Shape::generateCube(), GLR::Shape::getCubeIndices(), GLR::Shape::getStandardLayout());
    
    auto planeMesh = std::make_shared<GLR::Mesh>();
    planeMesh->create(GLR::Shape::generatePlane(1.0f, 1.0f, 1, 1, 1.0f, 1.0f), GLR::Shape::getPlaneIndices(1, 1), GLR::Shape::getStandardLayout());
    
    auto grayMaterial = std::make_shared<GLR::Material>(shader);
    grayMaterial->setVector4("baseColorFactor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0));
    
    groundPlane = scene->createEntity("Ground");
    groundPlane->addComponent<GLR::Transform>(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(50.0f, 1.0f, 50.0f));
    groundPlane->addComponent<GLR::MeshRenderer>(planeMesh, grayMaterial);
    groundPlane->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::STATIC, false);
    groundPlane->addComponent<GLR::BoxCollider>(glm::vec3(25.0f, 0.1f, 25.0f));
    
    for (int i = 0; i < 8; i++) {
        float angle = (i / 8.0f) * 2.0f * M_PI;
        float radius = 15.0f + (i % 2) * 5.0f;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;
        float height = 2.0f + (i % 3) * 2.0f;
        
        auto obstacle = scene->createEntity("Obstacle_" + std::to_string(i));
        obstacle->addComponent<GLR::Transform>(glm::vec3(x, (height / 2.0f) - 1.0f, z), glm::vec3(0.0f), glm::vec3(2.0f, height, 2.0f));
        obstacle->addComponent<GLR::MeshRenderer>(cubeMesh, grayMaterial);
        obstacle->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::STATIC, false);
        obstacle->addComponent<GLR::BoxCollider>(glm::vec3(1.0f, height / 2, 1.0f));
        staticObstacles.push_back(obstacle);
    }
    
    spawnDynamicObjects();
}

void createLighting() {
    directionalLight = scene->createEntity("Sun");
    directionalLight->addComponent<GLR::Transform>(glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -0.5), glm::vec3(1.0, 1.0, 1.0));
    directionalLight->addComponent<GLR::DirectionalLight>(glm::vec3(-1.0, -1.0, -0.5), glm::vec3(1.0f, 0.95f, 0.8f), 1.0f, true, 30.0f);
}

void resetPhysicsScene() {
    for (size_t i = 0; i < dynamicSpheres.size(); i++) {
        auto& transform = dynamicSpheres[i]->getComponent<GLR::Transform>();
        auto& rigidBody = dynamicSpheres[i]->getComponent<GLR::RigidBody>();
        
        float x = ((i % 4) - 1.5f) * 2.0f;
        float y = 15.0f + (i / 4) * 3.0f;
        float z = ((i / 4) - 1) * 2.0f;
        
        transform.setPosition(glm::vec3(x, y, z));
        transform.setRotation(glm::vec3(0.0f));
        rigidBody.setLinearVelocity(glm::vec3(0.0f));
        rigidBody.setAngularVelocity(glm::vec3(0.0f));
        rigidBody.setIsSleeping(false);
    }
    
    for (size_t i = 0; i < dynamicBoxes.size(); i++) {
        auto& transform = dynamicBoxes[i]->getComponent<GLR::Transform>();
        auto& rigidBody = dynamicBoxes[i]->getComponent<GLR::RigidBody>();
        
        float x = ((i % 4) - 1.5f) * 3.0f;
        float y = 20.0f + (i / 4) * 4.0f;
        float z = ((i / 4) - 0.5f) * 3.0f;
        
        transform.setPosition(glm::vec3(x, y, z));
        transform.setRotation(glm::vec3(0.0f));
        rigidBody.setLinearVelocity(glm::vec3(0.0f));
        rigidBody.setAngularVelocity(glm::vec3(0.0f));
        rigidBody.setIsSleeping(false);
    }
}

void spawnNewObject(int objectType) {
    auto shader = renderer->getObjectShader();
    
    float randomX = (rand() / float(RAND_MAX)) * 2.0f - 1.0f;
    float randomZ = (rand() / float(RAND_MAX)) * 2.0f - 1.0f;
    
    if (objectType == 0) {
        auto sphereMesh = std::make_shared<GLR::Mesh>();
        sphereMesh->create(GLR::Shape::generateSphere(1.0f, 32), GLR::Shape::getSphereIndices(32), GLR::Shape::getStandardLayout());
        
        auto material = std::make_shared<GLR::Material>(shader);
        material->setVector4("baseColorFactor", glm::vec4(rand() / float(RAND_MAX),rand() / float(RAND_MAX), rand() / float(RAND_MAX), 1.0f));
        
        auto sphere = scene->createEntity("SpawnedSphere_" + std::to_string(dynamicSpheres.size()));
        sphere->addComponent<GLR::Transform>(glm::vec3(randomX, 15.0f, randomZ), glm::vec3(0.0f), glm::vec3(1.0f));
        sphere->addComponent<GLR::MeshRenderer>(sphereMesh, material);
        sphere->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::DYNAMIC, true);
        sphere->addComponent<GLR::SphereCollider>(1.0f);
        sphere->addComponent<DemoCollisionResponder>();
        
        auto& rigidBody = sphere->getComponent<GLR::RigidBody>();
        rigidBody.setMass(demoSettings.defaultMass);
        
        auto& collider = sphere->getComponent<GLR::SphereCollider>();
        collider.setBounciness(demoSettings.defaultBounciness);
        collider.setFriction(demoSettings.defaultFriction);
        
        dynamicSpheres.push_back(sphere);
    }
    else if (objectType == 1) {
        auto cubeMesh = std::make_shared<GLR::Mesh>();
        cubeMesh->create(GLR::Shape::generateCube(), GLR::Shape::getCubeIndices(), GLR::Shape::getStandardLayout());
        
        auto material = std::make_shared<GLR::Material>(shader);
        material->setVector4("baseColorFactor", glm::vec4(rand() / float(RAND_MAX),rand() / float(RAND_MAX), rand() / float(RAND_MAX), 1.0f));
        
        auto box = scene->createEntity("SpawnedBox_" + std::to_string(dynamicBoxes.size()));
        box->addComponent<GLR::Transform>(glm::vec3(randomX, 15.0f, randomZ), glm::vec3(0.0f), glm::vec3(1.5f));
        box->addComponent<GLR::MeshRenderer>(cubeMesh, material);
        box->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::DYNAMIC, true);
        box->addComponent<GLR::BoxCollider>(glm::vec3(0.75f));
        box->addComponent<DemoCollisionResponder>();
        
        auto& rigidBody = box->getComponent<GLR::RigidBody>();
        rigidBody.setMass(demoSettings.defaultMass * 2.0f);
        
        auto& collider = box->getComponent<GLR::BoxCollider>();
        collider.setBounciness(demoSettings.defaultBounciness * 0.7f);
        collider.setFriction(demoSettings.defaultFriction * 1.5f);
        
        dynamicBoxes.push_back(box);
    }
}

void updatePhysicsStats() {
    physicsStats.totalRigidBodies = dynamicSpheres.size() + dynamicBoxes.size() + staticObstacles.size() + 1;
    physicsStats.activeRigidBodies = 0;
    physicsStats.sleepingRigidBodies = 0;
    
    for (auto& sphere : dynamicSpheres) {
        if (sphere->hasComponent<GLR::RigidBody>()) {
            auto& rb = sphere->getComponent<GLR::RigidBody>();
            if (rb.isSleeping()) {
                physicsStats.sleepingRigidBodies++;
            } 
            else {
                physicsStats.activeRigidBodies++;
            }
        }
    }
    
    for (auto& box : dynamicBoxes) {
        if (box->hasComponent<GLR::RigidBody>()) {
            auto& rb = box->getComponent<GLR::RigidBody>();
            if (rb.isSleeping()) {
                physicsStats.sleepingRigidBodies++;
            }
            else {
                physicsStats.activeRigidBodies++;
            }
        }
    }
}

void processInput(GLFWwindow* window, float deltaTime) {
    static bool spacePressed = false;
    static bool rPressed = false;
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
        
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        cameraAngle -= cameraRotationSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        cameraAngle += cameraRotationSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        cameraHeight = glm::clamp(cameraHeight + 8.0f * deltaTime, 5.0f, 30.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        cameraHeight = glm::clamp(cameraHeight - 8.0f * deltaTime, 5.0f, 30.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraDistance = glm::clamp(cameraDistance - 15.0f * deltaTime, 10.0f, 80.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraDistance = glm::clamp(cameraDistance + 15.0f * deltaTime, 10.0f, 80.0f);
    }
    
    bool spaceCurrentlyPressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (spaceCurrentlyPressed && !spacePressed) {
        autoRotateCamera = !autoRotateCamera;
    }
    spacePressed = spaceCurrentlyPressed;
    
    bool rCurrentlyPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (rCurrentlyPressed && !rPressed) {
        resetPhysicsScene();
    }
    rPressed = rCurrentlyPressed;
}

void renderImGui() {
    if (ImGui::Begin("Physics Demo Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::SliderFloat("Time Scale", &demoSettings.timeScale, 0.0f, 3.0f);
        
        ImGui::Separator();
        
        ImGui::Text("Spawn Objects:");
        if (ImGui::Button("Spawn Sphere")) {
            spawnNewObject(0);
        }
        ImGui::SameLine();
        if (ImGui::Button("Spawn Box")) {
            spawnNewObject(1);
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Reset Scene (R)")) {
            resetPhysicsScene();
        }
        
        ImGui::Separator();
        
        if (ImGui::CollapsingHeader("Material Settings")) {
            bool materialChanged = false;
            materialChanged |= ImGui::SliderFloat("Bounciness", &demoSettings.defaultBounciness, 0.0f, 1.0f);
            materialChanged |= ImGui::SliderFloat("Friction", &demoSettings.defaultFriction, 0.0f, 2.0f);
            materialChanged |= ImGui::SliderFloat("Mass", &demoSettings.defaultMass, 0.1f, 10.0f);
            
            if (materialChanged) {
                for (auto& sphere : dynamicSpheres) {
                    if (sphere->hasComponent<GLR::SphereCollider>()) {
                        auto& collider = sphere->getComponent<GLR::SphereCollider>();
                        collider.setBounciness(demoSettings.defaultBounciness);
                        collider.setFriction(demoSettings.defaultFriction);
                    }
                    if (sphere->hasComponent<GLR::RigidBody>()) {
                        auto& rb = sphere->getComponent<GLR::RigidBody>();
                        rb.setMass(demoSettings.defaultMass);
                    }
                }
                
                for (auto& box : dynamicBoxes) {
                    if (box->hasComponent<GLR::BoxCollider>()) {
                        auto& collider = box->getComponent<GLR::BoxCollider>();
                        collider.setBounciness(demoSettings.defaultBounciness * 0.7f);
                        collider.setFriction(demoSettings.defaultFriction * 1.5f);
                    }
                    if (box->hasComponent<GLR::RigidBody>()) {
                        auto& rb = box->getComponent<GLR::RigidBody>();
                        rb.setMass(demoSettings.defaultMass * 2.0f);
                    }
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
        updatePhysicsStats();
        
        ImGui::Text("Performance:");
        ImGui::Text("  FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("  Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        
        if (renderer) {
            ImGui::Text("  Rendered Meshes: %d", renderer->getRenderedMeshCount());
        }
        
        ImGui::Separator();
        
        ImGui::Text("Physics:");
        ImGui::Text("  Total Bodies: %d", physicsStats.totalRigidBodies);
        ImGui::Text("  Active Bodies: %d", physicsStats.activeRigidBodies);
        ImGui::Text("  Sleeping Bodies: %d", physicsStats.sleepingRigidBodies);
        ImGui::Text("  Spheres: %zu", dynamicSpheres.size());
        ImGui::Text("  Boxes: %zu", dynamicBoxes.size());
        ImGui::Text("  Collision Events: %d", physicsStats.collisionEvents);
        
        ImGui::End();
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

    window = glfwCreateWindow(width, height, "GLare Engine - Physics Demo", nullptr, nullptr);
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
    settings.renderDebug = true;
    settings.wireframeMode = false;
    settings.enableFaceCulling = true;
    settings.frustumCulling = true;
    settings.enablePostProcessing = true;
    settings.enableBloom = false;
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
    mainCamera->addComponent<GLR::Transform>(glm::vec3(0.0f, 12.0f, 25.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    mainCamera->addComponent<GLR::CameraComponent>(width, height, 45.0f, 0.1f, 200.0f);
    scene->setMainCamera(mainCamera);

    createPhysicsObjects();
    createLighting();

    lastFrame = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        currentFrame = glfwGetTime();
        GLR::TimeStep ts;
        ts.updateTimeStep(lastFrame, currentFrame);
        float deltaTime = ts.getDeltaTime() * demoSettings.timeScale;
        lastFrame = currentFrame;

        processInput(window, deltaTime);
        updateCamera();

        GLR::GetPhysicsWorld().update(deltaTime);

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