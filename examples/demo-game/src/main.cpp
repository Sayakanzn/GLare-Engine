#include "GLare.hpp"
#include "player.hpp"
#include "tree.hpp"
#include "managers.hpp"
#include "uiManager.hpp"

#include <algorithm>
#include <random>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

// Global variables
GameState currentGameState = GameState::START_SCREEN;
bool gameOver = false;
bool gameWon = false;
int width = 1280, height = 720;
int START_SCREEN_WIDTH = 400, START_SCREEN_HEIGHT = 500;
float lastFrame, currentFrame;
float lastPrint = 0.0f;
int frameCount = 0;
GLFWwindow* window = nullptr;

// Game objects
std::shared_ptr<GLR::Scene> scene;
std::shared_ptr<GLR::Renderer> renderer;
std::unique_ptr<GLR::PlayerController> playerController;
GLR::CampfireManager* campfireManager = nullptr;
GLR::LogCabinManager* logCabinManager = nullptr;
std::unique_ptr<GLR::TreeSpawner> treeSpawner;
std::shared_ptr<GLR::Shader> shaderBrokenBuilding;

// Game settings (configurable from start screen)
GameSettings gameSettings;

// Camera settings
float cameraHeight = 5.0f;
float cameraAngle = -45.0f;

// Day-Night Cycle System
DayNightCycle dayNightCycle;

// Cold/Freezing System
ColdSystem coldSystem;

// UI Manager
UiManager uiManager;

float blueprintProgress = 0.0f;
float programTime = 0.0f;

// Forward declarations
float smoothstep(float edge0, float edge1, float x);
void resetGame();
void initializeScene();

void updateColdSystem(float deltaTime) {
    bool nearWarmth = uiManager.isPlayerNearWarmth(scene, coldSystem);
    float coldRateMultiplier = dayNightCycle.isNight ? coldSystem.nightColdMultiplier : 1.0f;
    
    if (nearWarmth) {
        coldSystem.currentColdness -= coldSystem.warmUpRate * deltaTime;
        coldSystem.currentColdness = std::max(0.0f, coldSystem.currentColdness);
    } 
    else {
        coldSystem.currentColdness += coldSystem.coldIncreaseRate * coldRateMultiplier * deltaTime;
        coldSystem.currentColdness = std::min(coldSystem.maxColdness, coldSystem.currentColdness);
    }
    
    if (coldSystem.currentColdness >= coldSystem.maxColdness && !coldSystem.isFrozen) {
        coldSystem.isFrozen = true;
        gameOver = true;
        currentGameState = GameState::GAME_OVER;
    }
    
    if (playerController) {
        float speedMultiplier = coldSystem.getMovementSpeedMultiplier();
        playerController->setColdSpeedMultiplier(speedMultiplier);
    }
}

void applyColdVisualEffects() {
    if (!renderer) return;
    
    GLR::Renderer::Settings settings = renderer->getSettings();
    settings.postProcessing.vignetteIntensity = coldSystem.getVignetteIntensity();
    settings.postProcessing.vignetteColor = glm::vec3(0.2f, 0.8f, 1.0f);
    settings.postProcessing.saturation = coldSystem.getSaturation();
    renderer->updateSettings(settings);
}

// Day-Night Cycle Functions
void updateDayNightCycle(float deltaTime) {
    dayNightCycle.currentTime += deltaTime;
    
    if (dayNightCycle.currentTime >= dayNightCycle.cycleDuration) {
        dayNightCycle.currentTime -= dayNightCycle.cycleDuration;
    }
    
    float cycleProgress = dayNightCycle.currentTime / dayNightCycle.cycleDuration;
    float dayRatio = dayNightCycle.dayDuration / dayNightCycle.cycleDuration;
    float halfTransition = (dayNightCycle.transitionDuration / dayNightCycle.cycleDuration) * 0.5f;
    
    float dayToNightStart = dayRatio - halfTransition;
    float dayToNightEnd = dayRatio + halfTransition;
    float nightToDayStart = 1.0f - halfTransition;
    float nightToDayEnd = halfTransition;
    
    if (cycleProgress >= nightToDayStart || cycleProgress <= nightToDayEnd) {
        dayNightCycle.isNight = (cycleProgress >= nightToDayStart);
        
        float t;
        if (cycleProgress >= nightToDayStart) {
            t = (cycleProgress - nightToDayStart) / (halfTransition * 2.0f);
        } else {
            t = 0.5f + (cycleProgress / (halfTransition * 2.0f));
        }
        dayNightCycle.nightIntensity = 1.0f - smoothstep(0.0f, 1.0f, t);
        
    } else if (cycleProgress >= dayToNightStart && cycleProgress <= dayToNightEnd) {
        float t = (cycleProgress - dayToNightStart) / (halfTransition * 2.0f);
        dayNightCycle.nightIntensity = smoothstep(0.0f, 1.0f, t);
        dayNightCycle.isNight = (cycleProgress >= dayRatio);
        
    } else if (cycleProgress < dayToNightStart) {
        dayNightCycle.isNight = false;
        dayNightCycle.nightIntensity = 0.0f;
        
    } else {
        dayNightCycle.isNight = true;
        dayNightCycle.nightIntensity = 1.0f;
    }
    
    dayNightCycle.nightIntensity = std::clamp(dayNightCycle.nightIntensity, 0.0f, 1.0f);
}

float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

void applyDayNightLighting(std::shared_ptr<GLR::Scene> scene) {
    auto sunEntity = scene->findEntityByName("Sun");
    if (sunEntity && sunEntity->hasComponent<GLR::DirectionalLight>()) {
        auto& directionalLight = sunEntity->getComponent<GLR::DirectionalLight>();
        
        glm::vec3 currentColor = glm::mix(
            dayNightCycle.dayLightColor,
            dayNightCycle.nightLightColor,
            dayNightCycle.nightIntensity
        );
        
        float currentIntensity = glm::mix(
            dayNightCycle.dayLightIntensity,
            dayNightCycle.nightLightIntensity,
            dayNightCycle.nightIntensity
        );
        
        directionalLight.setColor(currentColor);
        directionalLight.setIntensity(currentIntensity);
    }
    
    auto pitFireLight = scene->findEntityByName("pitFireLight");
    if (pitFireLight && pitFireLight->hasComponent<GLR::PointLight>()) {
        auto& pointLight = pitFireLight->getComponent<GLR::PointLight>();
        
        float currentRadius = glm::mix(
            dayNightCycle.baseCampfireRadius,
            dayNightCycle.nightCampfireRadius,
            dayNightCycle.nightIntensity
        );
        
        pointLight.setRadius(currentRadius);
    }
}

// GLFW Callbacks
void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void glfwFramebufferSizeCallback(GLFWwindow* window, int newWidth, int newHeight) {
    width = newWidth;
    height = newHeight;
    glViewport(0, 0, width, height);
    if (scene && scene->getMainCameraComponent()) {
        scene->getMainCameraComponent()->setAspectRatio(newWidth, newHeight);
    }
    if (renderer) {
        renderer->resize(newWidth, newHeight);
    }
}

void glfwWindowCloseCallback(GLFWwindow* window) {
    glfwSetWindowShouldClose(window, true);
}

// Input Processing
void processInput(GLFWwindow* window) {
    if (currentGameState == GameState::START_SCREEN) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        return;
    }
    
    if (currentGameState != GameState::PLAYING) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        return;
    }

    glm::vec3 moveInput(0.0f);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        moveInput.z = -1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        moveInput.z = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        moveInput.x = -1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        moveInput.x = 1.0f;
    }
    
    bool attackInput = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
    bool interactInput = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    bool sprintInput = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    
    if (playerController) {
        playerController->setMoveInput(moveInput);
        playerController->setAttackInput(attackInput);
        playerController->setInteractInput(interactInput);
        playerController->setSprintInput(sprintInput);
    }
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

// Camera Update
void updateCamera(std::shared_ptr<GLR::Entity> camera, std::shared_ptr<GLR::Entity> player) {
    auto& playerTransform = player->getComponent<GLR::Transform>();
    auto& cameraTransform = camera->getComponent<GLR::Transform>();
    
    glm::vec3 playerPos = playerTransform.getWorldPosition();
    
    float radians = glm::radians(cameraAngle);
    float horizontalDistance = cameraHeight * tan(-radians);
    
    glm::vec3 cameraPos = playerPos;
    cameraPos.y += cameraHeight;
    cameraPos.z += horizontalDistance;
    
    cameraTransform.setPosition(cameraPos);
    cameraTransform.setRotation(glm::vec3(cameraAngle, 0.0f, 0.0f));
}

// Game Reset and Initialization
void resetGame() {
    gameOver = false;
    gameWon = false;
    
    dayNightCycle.currentTime = 0.0f;
    dayNightCycle.isNight = false;
    dayNightCycle.nightIntensity = 0.0f;
    
    coldSystem.currentColdness = 0.0f;
    coldSystem.isFrozen = false;
    
    blueprintProgress = 0.0f;
    
    if (scene) {
        auto playerEntity = scene->findEntityByName("PlayerCollider");
        if (playerEntity) {
            if (playerEntity->hasComponent<GLR::PlayerInventory>()) {
                auto& inventory = playerEntity->getComponent<GLR::PlayerInventory>();
                while (inventory.getLogs() > 0) {
                    inventory.removeLogs(1);
                }
                inventory.addLogs(5);
            }
            
            auto& transform = playerEntity->getComponent<GLR::Transform>();
            transform.setPosition(glm::vec3(-2.0, -2.0, 0.0));
        }
        
        if (campfireManager) {
            campfireManager->setCurrentFuel(campfireManager->getMaxFuel() * 0.5f);
        }
        
        if (logCabinManager) {
            logCabinManager->resetCabin();
        }
        
        if (playerController) {
            playerController->setColdSpeedMultiplier(1.0f);
        }
    }
}

void createInvisibleWall(std::shared_ptr<GLR::Scene> scene) {
    const float wallHeight = -1.5f;
    const float wallThickness = 2.0f;
    const float wallOffset = 25.0f;
    const float planeSize = 25.0f;
    
    std::vector<std::pair<glm::vec3, glm::vec3>> walls = {
        {glm::vec3(0.0f, wallHeight, wallOffset), glm::vec3(planeSize * 2.2f, 3.0f, wallThickness)},
        {glm::vec3(0.0f, wallHeight, -wallOffset), glm::vec3(planeSize * 2.2f, 3.0f, wallThickness)},
        {glm::vec3(wallOffset, wallHeight, 0.0f), glm::vec3(wallThickness, 3.0f, planeSize * 2.2f)},
        {glm::vec3(-wallOffset, wallHeight, 0.0f), glm::vec3(wallThickness, 3.0f, planeSize * 2.2f)}
    };
    
    for (size_t i = 0; i < walls.size(); i++) {
        auto wall = scene->createEntity("InvisibleWall_" + std::to_string(i));
        wall->addComponent<GLR::Transform>(walls[i].first, glm::vec3(0.0f), glm::vec3(1.0f));
        wall->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::STATIC);
        wall->addComponent<GLR::BoxCollider>(walls[i].second);
    }
}

void initializeScene() {
    scene = std::make_shared<GLR::Scene>();
    
    // Create shaders
    auto shader = std::make_shared<GLR::Shader>(
        "./res/shader/main.vert",
        "./res/shader/main.frag"
    );

    auto shaderModified = std::make_shared<GLR::Shader>(
        "./res/shader/main.vert",
        "./res/shader/main.frag"
    );

    shaderBrokenBuilding = std::make_shared<GLR::Shader>(
        "./res/shader/main.vert",
        "./res/shader/building.frag"
    );

    // Load models
    auto playerModel = std::make_shared<GLR::Model>();
    playerModel->create("./res/models/charctersModels/GLBformat/character-male-b.glb", shader);

    auto campFirePitModel = std::make_shared<GLR::Model>();
    campFirePitModel->create("./res/models/survival/GLBformat/campfire-pit.glb", shader);

    auto snowTreeModel = std::make_shared<GLR::Model>();
    snowTreeModel->create("./res/models/holidaykit/GLBformat/tree-snow-a.glb", shader);

    auto fire_anim = std::make_shared<GLR::Model>();
    fire_anim->create("./res/models/fire_animation/scene.gltf", shader);

    auto logModel = std::make_shared<GLR::Model>();
    logModel->create("./res/models/survival/GLBformat/tree-log-small.glb", shader);

    auto cabin = std::make_shared<GLR::Model>(true, true);
    cabin->create("./res/models/Cabin.gltf", shaderBrokenBuilding);
    
    auto rockWall = std::make_shared<GLR::Model>();
    rockWall->create("./res/models/RockWall.gltf", shader);
    rockWall->getBoundingBox().expandToInclude(glm::vec3(-25.0f, 0.0f, -25.0f));
    rockWall->getBoundingBox().expandToInclude(glm::vec3(25.0f, 0.0f, 25.0f));

    // Create meshes
    auto cube = std::make_shared<GLR::Mesh>();
    cube->create(GLR::Shape::generateCube(), GLR::Shape::getCubeIndices(), GLR::Shape::getStandardLayout());

    auto plane = std::make_shared<GLR::Mesh>();
    plane->create(GLR::Shape::generatePlane(1.0f, 1.0f, 1, 1, 10.0f, 10.0f), GLR::Shape::getPlaneIndices(1, 1), GLR::Shape::getStandardLayout());

    // Create materials
    auto floor_diff = std::make_shared<GLR::Texture>("./res/texture/ground/Cartoon_green_texture_grass.jpg");
    auto snow_norm = std::make_shared<GLR::Texture>("./res/texture/ground/Cartoon_snow_texture_grass.jpg");

    auto material = std::make_shared<GLR::Material>(shader);
    material->setTexture("baseColorTexture", floor_diff);

    auto materialsnow = std::make_shared<GLR::Material>(shaderModified);
    materialsnow->setTexture("baseColorTexture", snow_norm);
    materialsnow->setFloat("width", 0.315f);

    // Setup tree spawner
    treeSpawner = std::make_unique<GLR::TreeSpawner>(scene);

    GLR::TreeSpawner::TreeType snowTreeType = {
        snowTreeModel,
        "SnowTree",
        glm::vec3(2.0f, 2.0f, 2.0f),
        0.3f,
        glm::vec3(0.5f, 1.0f, 0.5f),
        glm::vec3(1.0f, 2.0f, 1.0f)
    };

    treeSpawner->addTreeType(snowTreeType);

    GLR::TreeSpawner::SpawnParameters spawnParams;
    spawnParams.spawnAreaMin = glm::vec2(-20.0f, -20.0f);
    spawnParams.spawnAreaMax = glm::vec2(20.0f, 20.0f);
    spawnParams.groundHeight = -3.0f;
    spawnParams.minDistanceBetweenTrees = gameSettings.minTreeDistance;
    spawnParams.maxTrees = gameSettings.maxTrees;
    spawnParams.seed = gameSettings.treeSeed;
    spawnParams.exclusionZones.push_back(glm::vec2(0.0f, 0.0f));
    spawnParams.exclusionZones.push_back(glm::vec2(-4.0f, -4.0f));
    spawnParams.exclusionRadius = 4.0f;

    treeSpawner->setSpawnParameters(spawnParams);
    treeSpawner->generateTrees();

    createInvisibleWall(scene);

    // Create lighting
    auto directionalLight = scene->createEntity("Sun");
    directionalLight->addComponent<GLR::Transform>(glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0, -1.0, -0.5), glm::vec3(1.0, 1.0, 1.0));
    directionalLight->addComponent<GLR::DirectionalLight>(glm::vec3(-1.0, -1.0, -0.5), glm::vec3(1.0f, 0.95f, 0.8f), 1.0f, true, 13.0f, 50.0f, 200.0f, 4096);

    // Create camera
    auto mainCamera = scene->createEntity("Main Camera");
    mainCamera->addComponent<GLR::Transform>(glm::vec3(0.0, cameraHeight, 5.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0));
    mainCamera->addComponent<GLR::CameraComponent>(width, height, 50.0f, 0.1f, 100.0f);
    scene->setMainCamera(mainCamera);

    // Create rock walls
    auto rockWallEntityOne = scene->createEntity("RockWallOne");
    rockWallEntityOne->addComponent<GLR::Transform>(glm::vec3(25.0f, -3.0f, 0.0f), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0));
    rockWallEntityOne->addComponent<GLR::ModelRenderer>(rockWall);
    
    auto rockWallEntityTwo = scene->createEntity("RockWallTwo");
    rockWallEntityTwo->addComponent<GLR::Transform>(glm::vec3(-25.0f, -3.0f, 0.0f), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0));
    rockWallEntityTwo->addComponent<GLR::ModelRenderer>(rockWall);
    
    auto rockWallEntityThree = scene->createEntity("RockWallThree");
    rockWallEntityThree->addComponent<GLR::Transform>(glm::vec3(0.0f, -3.0f, -25.0f), glm::vec3(0.0, 90.0, 0.0), glm::vec3(1.0, 1.0, 1.0));
    rockWallEntityThree->addComponent<GLR::ModelRenderer>(rockWall);
    
    auto rockWallEntityFour = scene->createEntity("RockWallFour");
    rockWallEntityFour->addComponent<GLR::Transform>(glm::vec3(0.0f, -3.0f, 25.0f), glm::vec3(0.0, 270.0, 0.0), glm::vec3(1.0, 1.0, 1.0));
    rockWallEntityFour->addComponent<GLR::ModelRenderer>(rockWall);

    // Create cabin
    auto cabinEntity = scene->createEntity("cabinEntity");
    cabinEntity->addComponent<GLR::Transform>(glm::vec3(-4.0f, -3.0f, -4.0f), glm::vec3(0.0, 45.0, 0.0), glm::vec3(1.0, 1.0, 1.0));
    cabinEntity->addComponent<GLR::ModelRenderer>(cabin);
    cabinEntity->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::STATIC);
    cabinEntity->addComponent<GLR::BoxCollider>(cabin->getHalfExtents() * 2.0f);

    auto cabinTrigger = scene->createEntity("cabinTrigger");
    cabinTrigger->addComponent<GLR::Transform>(glm::vec3(-4.0f, -2.5f, -4.0f), glm::vec3(0.0, 45.0, 0.0), glm::vec3(3.0, 3.0, 3.0));
    cabinTrigger->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::STATIC);
    cabinTrigger->addComponent<GLR::BoxCollider>(glm::vec3(2.2f, 1.5f, 2.2f)).setIsTrigger(true);

    GLR::LogCabinManager::CabinSettings cabinSettings;
    cabinSettings.maxLogs = 20;
    cabinSettings.logsToProgressRatio = 1.0f / float(cabinSettings.maxLogs);
    cabinSettings.progressChangeSpeed = 2.0f;

    auto& cabinManagerComponent = cabinTrigger->addComponent<GLR::LogCabinManager>(
        &blueprintProgress,
        cabinSettings
    );
    logCabinManager = &cabinManagerComponent;

    // Create ground layers
    auto snowLayer = scene->createEntity("SnowLayer");
    snowLayer->addComponent<GLR::Transform>(glm::vec3(0.0, -2.99, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(50.0, 0.1, 50.0));
    snowLayer->addComponent<GLR::MeshRenderer>(plane, materialsnow);

    auto ground = scene->createEntity("Ground");
    ground->addComponent<GLR::Transform>(glm::vec3(0.0, -3.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(50.0, 0.1, 50.0));
    ground->addComponent<GLR::MeshRenderer>(plane, material);
    ground->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::STATIC);
    ground->addComponent<GLR::BoxCollider>(glm::vec3(50.0f, 0.1f, 50.0f)).setFriction(5.0f);

    // Create campfire
    auto campFirePit = scene->createEntity("campFirePit");
    campFirePit->addComponent<GLR::Transform>(glm::vec3(0.0, -3.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(4.0, 4.0, 4.0));
    campFirePit->addComponent<GLR::ModelRenderer>(campFirePitModel);
    campFirePit->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::STATIC);
    campFirePit->addComponent<GLR::BoxCollider>(glm::vec3(0.5f, 0.5f, 0.5f));

    auto pitFire = scene->createEntity("pitFire");
    pitFire->addComponent<GLR::Transform>(glm::vec3(0.0, -2.2, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.5, 0.5, 0.5));
    pitFire->addComponent<GLR::ModelRenderer>(fire_anim);
    fire_anim->getAnimationManager()->playAnimation(fire_anim->getAnimationManager()->getAnimationNames()[0], true);

    auto pitFireLight = scene->createEntity("pitFireLight");
    pitFireLight->addComponent<GLR::Transform>(glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0));
    pitFireLight->addComponent<GLR::PointLight>(5.0, glm::vec3(1.0, 0.4, 0.0), 5.0, false, GLR::PointLight::FalloffType::SHARP);

    // Create campfire trigger and manager
    auto campFireTrigger = scene->createEntity("campFireTrigger");
    campFireTrigger->addComponent<GLR::Transform>(glm::vec3(0.0, -2.5, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(4.0, 4.0, 4.0));
    campFireTrigger->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::STATIC);
    campFireTrigger->addComponent<GLR::BoxCollider>(glm::vec3(1.5f, 1.0f, 1.5f)).setIsTrigger(true);

    GLR::CampfireManager::CampfireSettings campfireSettings;
    campfireSettings.maxFuel = 200.0f;
    campfireSettings.fuelBurnRate = 5.0f;
    campfireSettings.logsToFuelRatio = 15.0f;
    campfireSettings.minFireScale = 0.1f;
    campfireSettings.maxFireScale = 1.2f;
    campfireSettings.minLightRadius = 1.0f;
    campfireSettings.maxLightRadius = 12.0f;

    auto& campfireManagerComponent = campFireTrigger->addComponent<GLR::CampfireManager>(
        pitFire,
        pitFireLight,
        materialsnow,
        campfireSettings
    );
    campfireManager = &campfireManagerComponent;

    // Create player
    auto playercollider = scene->createEntity("PlayerCollider");
    playercollider->addComponent<GLR::Transform>(glm::vec3(-2.0, -2.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0));
    playercollider->addComponent<GLR::RigidBody>(GLR::RigidBody::BodyType::DYNAMIC);
    playercollider->addComponent<GLR::CapsuleCollider>(0.25f, 0.5f);
    playercollider->addComponent<GLR::PlayerInventory>();

    auto playermodel = scene->createEntity("PlayerModel");
    playermodel->setParent(playercollider);
    playermodel->addComponent<GLR::Transform>(glm::vec3(0.0, -0.5f, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 1.0));
    playermodel->addComponent<GLR::ModelRenderer>(playerModel);

    // Setup log stack manager
    auto& PlayerLogStack = playercollider->addComponent<GLR::PlayerLogStack>(logModel, playercollider);
    PlayerLogStack.setStackHeight(0.20f);
    PlayerLogStack.setBaseOffset(glm::vec3(0.0f, -0.2f, -0.25f));
    PlayerLogStack.setLogScale(glm::vec3(0.8, 0.8, 0.8));
    PlayerLogStack.setMaxVisibleLogs(20);
    PlayerLogStack.setRotationVariation(7.50f);
    PlayerLogStack.setSwayIntensity(0.04f);
    PlayerLogStack.setSwaySpeed(15.0f);
    PlayerLogStack.setJiggleIntensity(0.07f);
    PlayerLogStack.setJiggleSpeed(25.0f);
    PlayerLogStack.setDampingFactor(0.8f);
    PlayerLogStack.setStackPhaseOffset(0.1f);
    PlayerLogStack.setHeightMultiplier(2.0f);

    if (playercollider->hasComponent<GLR::PlayerInventory>()) {
        auto& inventory = playercollider->getComponent<GLR::PlayerInventory>();
        inventory.setPlayerLogStack(&PlayerLogStack);
        inventory.addLogs(5);
    }

    // Create player controller
    playerController = std::make_unique<GLR::PlayerController>(playercollider.get(), playermodel.get());

    // Initialize campfire fuel
    campfireManager->setCurrentFuel(campfireSettings.maxFuel * 0.5f);
}

int main(int argc, char* argv[]) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwSetErrorCallback(glfwErrorCallback);

    // Set window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Start as non-resizable
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 0);

    // Create window with start screen size
    window = glfwCreateWindow(START_SCREEN_WIDTH, START_SCREEN_HEIGHT, "Survival Game Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set initial window dimensions
    width = START_SCREEN_WIDTH;
    height = START_SCREEN_HEIGHT;

    // Center the window on screen
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int xPos = (mode->width - START_SCREEN_WIDTH) / 2;
    int yPos = (mode->height - START_SCREEN_HEIGHT) / 2;
    glfwSetWindowPos(window, xPos, yPos);

    glfwMakeContextCurrent(window);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, glfwFramebufferSizeCallback);
    glfwSetWindowCloseCallback(window, glfwWindowCloseCallback);

    glfwSwapInterval(gameSettings.vsync);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Initialize UI Manager
    if (!uiManager.initialize(window)) {
        std::cerr << "Failed to initialize UI Manager" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Initialize renderer (needed for start screen)
    GLR::TimeStep ts;

    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    renderer = std::make_shared<GLR::Renderer>(framebuffer_width, framebuffer_height);
    
    GLR::Renderer::Settings settings = renderer->getSettings();
    settings.renderDebug = false;
    settings.wireframeMode = false;
    settings.frustumCulling = true;
    settings.postProcessing.saturation = 0.0f;
    renderer->updateSettings(settings);
    renderer->applyPostProcessingPreset("stylized");

    lastFrame = glfwGetTime();

    // Main game loop
    while (!glfwWindowShouldClose(window)) {
        currentFrame = glfwGetTime();
        ts.updateTimeStep(lastFrame, currentFrame);
        lastFrame = currentFrame;
        float deltaTime = ts.getDeltaTime();
        programTime += deltaTime;

        processInput(window);

        // Handle state transitions
        static GameState previousState = GameState::START_SCREEN;
        if (currentGameState != previousState) {
            if (currentGameState == GameState::PLAYING && previousState == GameState::START_SCREEN) {
                // Make window resizable again and resize to game size
                glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_TRUE);
                
                // Apply settings and start game
                dayNightCycle.dayDuration = gameSettings.dayDuration;
                dayNightCycle.nightDuration = gameSettings.nightDuration;
                dayNightCycle.cycleDuration = gameSettings.dayDuration + gameSettings.nightDuration;
                
                // Resize window back to game size and center it
                glfwSetWindowSize(window, 1280, 720);
                width = 1280;
                height = 720;
                
                // Center the window on screen
                const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
                int xPos = (mode->width - 1280) / 2;
                int yPos = (mode->height - 720) / 2;
                glfwSetWindowPos(window, xPos, yPos);
                
                // Update renderer size
                int framebuffer_width, framebuffer_height;
                glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
                renderer = std::make_shared<GLR::Renderer>(framebuffer_width, framebuffer_height);
                GLR::Renderer::Settings settings = renderer->getSettings();
                settings.renderDebug = false;
                settings.wireframeMode = false;
                settings.frustumCulling = true;
                settings.postProcessing.saturation = 0.0f;
                renderer->updateSettings(settings);
                renderer->applyPostProcessingPreset("stylized");
                
                initializeScene();
            }
            previousState = currentGameState;
        }

        // Update game based on state
        if (currentGameState == GameState::PLAYING) {
            // Check for game over conditions
            if (campfireManager && campfireManager->getCurrentFuel() <= 0.0f) {
                gameOver = true;
                currentGameState = GameState::GAME_OVER;
            }

            updateDayNightCycle(deltaTime);
            applyDayNightLighting(scene);
            
            updateColdSystem(deltaTime);
            applyColdVisualEffects();

            if (playerController) {
                playerController->update(deltaTime);
            }

            scene->update(deltaTime);

            // Update sun position to follow player
            auto playerEntity = scene->findEntityByName("PlayerCollider");
            if (playerEntity) {
                glm::vec3 playerpos = playerEntity->getComponent<GLR::Transform>().getPosition();
                playerpos.x += 4.0f;
                auto sunEntity = scene->findEntityByName("Sun");
                if (sunEntity) {
                    sunEntity->getComponent<GLR::Transform>().setPosition(playerpos);
                }
                
                auto mainCamera = scene->findEntityByName("Main Camera");
                if (mainCamera) {
                    updateCamera(mainCamera, playerEntity);
                }
            }
        }

        // ImGui rendering
        uiManager.beginFrame();
        
        // Handle UI button presses and draw appropriate screens
        switch (currentGameState) {
            case GameState::START_SCREEN:
                uiManager.drawStartScreen(width, height, gameSettings, window);
                if (uiManager.startGamePressed) {
                    currentGameState = GameState::PLAYING;
                }
                break;
            case GameState::PLAYING:
                uiManager.drawGameHUD(width, height, scene, playerController.get(), campfireManager, logCabinManager, dayNightCycle, coldSystem, gameWon, currentGameState);
                break;
            case GameState::GAME_WON:
                uiManager.drawGameHUD(width, height, scene, playerController.get(), campfireManager, logCabinManager, dayNightCycle, coldSystem, gameWon, currentGameState);
                uiManager.drawWinScreen(width, height, window, currentGameState);
                if (uiManager.playAgainPressed) {
                    currentGameState = GameState::START_SCREEN;
                    resetGame();
                }
                break;
            case GameState::GAME_OVER:
                uiManager.drawGameHUD(width, height, scene, playerController.get(), campfireManager, logCabinManager, dayNightCycle, coldSystem, gameWon, currentGameState);
                uiManager.drawGameOverScreen(width, height, coldSystem, window, currentGameState);
                if (uiManager.restartPressed) {
                    resetGame();  // Reset the game state
                    currentGameState = GameState::PLAYING;  // Go directly back to playing, not start screen
                }
                break;
        }

        // Render the scene (only when playing)
        if (currentGameState == GameState::PLAYING || 
            currentGameState == GameState::GAME_OVER || 
            currentGameState == GameState::GAME_WON) {
            
            if (scene && scene->getMainCameraEntity()) {
                shaderBrokenBuilding->bind();
                shaderBrokenBuilding->setFloat("blueprintProgress", blueprintProgress);
                shaderBrokenBuilding->setFloat("time", programTime);
                
                renderer->render(*scene, GLR::Color::Black());
            }
        } 
        else {
            // Clear screen for start screen
            glClearColor(0.12f, 0.14f, 0.18f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        uiManager.endFrame();

        if (currentGameState == GameState::PLAYING) {
            frameCount++;
            if (currentFrame - lastPrint >= 1.0f) {
                std::cout << "FPS: " << frameCount << std::endl;
                frameCount = 0;
                lastPrint = currentFrame;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    uiManager.shutdown();

    if (scene) {
        scene->clearEntities();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}