#pragma once

#include <memory>
#include <vector>
#include <random>
#include <unordered_map>
#include <algorithm>
#include <queue>
#include <cmath>
#include <glm/glm.hpp>
#include "GLare.hpp"

namespace GLR {

class TreeRemovalMarker : public Component {};

class TreeCutter : public CollisionResponder {
public:
    enum class TreeState {
        INTACT,
        SHAKING,
        FALLING,
        SHRINKING,
        REMOVED
    };

    TreeCutter() 
        : isPlayerNearby(false)
        , nearbyPlayer(nullptr)
        , chopTime(0.0f)
        , chopDuration(2.0f)
        , chopInProgress(false)
        , minLogs(1)
        , maxLogs(3)
        , treeState(TreeState::INTACT)
        , animationTime(0.0f)
        , fallDuration(1.5f)
        , shrinkDuration(0.8f)
        , shakeIntensity(0.02f)
        , shakeSpeed(20.0f)
        , originalPosition(0.0f)
        , originalRotation(0.0f)
        , originalScale(1.0f)
        , fallDirection(1.0f, 0.0f, 0.0f)
        , logsGiven(false)
        , initialized(false)
        , treeSizeValue(1.0f) {
        
        std::random_device rd;
        randomGenerator.seed(rd());
    }

    void init() override {
        CollisionResponder::init();
        
        Entity* treeModelEntity = getTreeModelEntity();
        
        if (treeModelEntity && treeModelEntity->hasComponent<Transform>()) {
            auto& treeTransform = treeModelEntity->getComponent<Transform>();
            originalPosition = treeTransform.getPosition();
            originalRotation = treeTransform.getRotation();
            originalScale = treeTransform.getScale();
            
            treeSizeValue = (originalScale.x + originalScale.y + originalScale.z) / 3.0f;
            setSizeBasedParameters();
            initialized = true;
            
            std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
            float fallAngle = angleDist(randomGenerator);
            fallDirection = glm::vec3(cos(fallAngle), 0.0f, sin(fallAngle));
        }
    }

    void update(float deltaTime) override {
        if (!initialized) {
            init();
            if (!initialized) {
                return;
            }
        }
        
        switch (treeState) {
            case TreeState::INTACT:
                updateIntactState(deltaTime);
                break;
            case TreeState::SHAKING:
                updateShakingState(deltaTime);
                break;
            case TreeState::FALLING:
                updateFallingState(deltaTime);
                break;
            case TreeState::SHRINKING:
                updateShrinkingState(deltaTime);
                break;
            case TreeState::REMOVED:
                break;
        }
    }

    void setChopDuration(float duration) { chopDuration = duration; }
    float getChopDuration() const { return chopDuration; }
    
    void setLogRange(int minLogsParam, int maxLogsParam) { 
        minLogs = minLogsParam; 
        maxLogs = maxLogsParam; 
    }
    
    void setAnimationSettings(float fallDurationParam, float shrinkDurationParam, float shakeIntensityParam) {
        fallDuration = fallDurationParam;
        shrinkDuration = shrinkDurationParam;
        shakeIntensity = shakeIntensityParam;
    }
    
    bool isBeingChopped() const { return chopInProgress; }
    float getChopProgress() const { return chopTime / chopDuration; }
    TreeState getTreeState() const { return treeState; }
    float getTreeSize() const { return treeSizeValue; }

protected:
    void handleTriggerEnter(Entity* otherEntity, const CollisionEvent& event) override {
        if (treeState == TreeState::INTACT && isPlayerEntity(otherEntity)) {
            isPlayerNearby = true;
            nearbyPlayer = otherEntity;
        }
    }
    
    void handleTriggerExit(Entity* otherEntity, const CollisionEvent& event) override {
        if (isPlayerEntity(otherEntity)) {
            isPlayerNearby = false;
            nearbyPlayer = nullptr;
            
            if (treeState == TreeState::SHAKING) {
                chopInProgress = false;
                chopTime = 0.0f;
                treeState = TreeState::INTACT;
                resetTreeTransform();
            }
        }
    }

private:
    Entity* getTreeModelEntity() {
        if (auto entity = getEntity()) {
            auto parent = entity->getParent();
            if (parent) {
                for (auto& child : parent->getChildren()) {
                    if (child->getName().find("_Model") != std::string::npos) {
                        return child.get();
                    }
                }
            }
        }
        return nullptr;
    }
    
    Entity* getTreeEntity() {
        if (auto entity = getEntity()) {
            return entity->getParent().get();
        }
        return nullptr;
    }

    void updateIntactState(float deltaTime) {
        if (isPlayerNearby && nearbyPlayer) {
            if (isPlayerHoldingE()) {
                chopInProgress = true;
                chopTime = 0.0f;
                treeState = TreeState::SHAKING;
            }
        }
    }
    
    void updateShakingState(float deltaTime) {
        if (!isPlayerNearby || !isPlayerHoldingE()) {
            chopInProgress = false;
            chopTime = 0.0f;
            treeState = TreeState::INTACT;
            resetTreeTransform();
            return;
        }
        
        chopTime += deltaTime;
        applyShakeAnimation(deltaTime);
        
        if (chopTime >= chopDuration) {
            treeState = TreeState::FALLING;
            animationTime = 0.0f;
            chopInProgress = false;
            giveLogsToPlayer();
            disableTreeCollision();
        }
    }
    
    void updateFallingState(float deltaTime) {
        animationTime += deltaTime;
        float progress = animationTime / fallDuration;
        
        if (progress >= 1.0f) {
            treeState = TreeState::SHRINKING;
            animationTime = 0.0f;
            return;
        }
        
        applyFallAnimation(progress);
    }
    
    void updateShrinkingState(float deltaTime) {
        animationTime += deltaTime;
        float progress = animationTime / shrinkDuration;
        
        if (progress >= 1.0f) {
            treeState = TreeState::REMOVED;
            markTreeForRemoval();
            return;
        }
        
        applyShrinkAnimation(progress);
    }
    
    void applyShakeAnimation(float deltaTime) {
        Entity* treeModelEntity = getTreeModelEntity();
        if (!treeModelEntity || !treeModelEntity->hasComponent<Transform>()) return;
        
        auto& treeTransform = treeModelEntity->getComponent<Transform>();
        
        static float shakeTime = 0.0f;
        shakeTime += deltaTime * shakeSpeed;
        
        float progressIntensity = 0.5f + (chopTime / chopDuration) * 1.5f;
        float sizeIntensity = 0.5f + (treeSizeValue * 0.5f);
        float currentIntensity = shakeIntensity * progressIntensity * sizeIntensity;
        
        glm::vec3 shakeOffset(
            sin(shakeTime) * currentIntensity,
            sin(shakeTime * 1.3f) * currentIntensity * 0.3f,
            cos(shakeTime * 0.8f) * currentIntensity
        );
        
        glm::vec3 shakenPosition = originalPosition + shakeOffset;
        treeTransform.setPosition(shakenPosition);
        
        glm::vec3 rotationShake(
            sin(shakeTime * 1.1f) * currentIntensity * 20.0f,
            0.0f,
            cos(shakeTime * 1.4f) * currentIntensity * 20.0f
        );
        glm::vec3 shakenRotation = originalRotation + rotationShake;
        treeTransform.setRotation(shakenRotation);
    }
    
    void applyFallAnimation(float progress) {
        Entity* treeModelEntity = getTreeModelEntity();
        if (!treeModelEntity || !treeModelEntity->hasComponent<Transform>()) return;
        
        auto& treeTransform = treeModelEntity->getComponent<Transform>();
        
        float easedProgress = 1.0f - (1.0f - progress) * (1.0f - progress);
        
        glm::vec3 fallAxis = glm::normalize(glm::cross(glm::vec3(0, 1, 0), fallDirection));
        float fallAngle = easedProgress * 90.0f;
        
        glm::vec3 currentRotation = originalRotation;
        currentRotation += fallAxis * fallAngle;
        treeTransform.setRotation(currentRotation);
        
        glm::vec3 fallPosition = originalPosition;
        fallPosition += fallDirection * easedProgress * 0.5f;
        fallPosition.y -= easedProgress * 0.2f;
        treeTransform.setPosition(fallPosition);
    }
    
    void applyShrinkAnimation(float progress) {
        Entity* treeModelEntity = getTreeModelEntity();
        if (!treeModelEntity || !treeModelEntity->hasComponent<Transform>()) return;
        
        auto& treeTransform = treeModelEntity->getComponent<Transform>();
        
        float easedProgress = progress * progress;
        float shrinkFactor = 1.0f - easedProgress;
        
        glm::vec3 currentScale = originalScale * shrinkFactor;
        treeTransform.setScale(currentScale);
        
        glm::vec3 currentPosition = treeTransform.getPosition();
        currentPosition.y = originalPosition.y - easedProgress * 0.5f;
        treeTransform.setPosition(currentPosition);
    }
    
    void resetTreeTransform() {
        Entity* treeModelEntity = getTreeModelEntity();
        if (!treeModelEntity || !treeModelEntity->hasComponent<Transform>()) return;
        
        auto& treeTransform = treeModelEntity->getComponent<Transform>();
        treeTransform.setPosition(originalPosition);
        treeTransform.setRotation(originalRotation);
        treeTransform.setScale(originalScale);
    }
    
    void disableTreeCollision() {
        Entity* treeEntity = getTreeEntity();
        if (!treeEntity) return;
        
        if (treeEntity->hasComponent<BoxCollider>()) {
            treeEntity->removeComponent<BoxCollider>();
        }
        if (treeEntity->hasComponent<RigidBody>()) {
            treeEntity->removeComponent<RigidBody>();
        }
    }
    
    void giveLogsToPlayer() {
        if (logsGiven) return;
        
        std::uniform_int_distribution<int> logDist(minLogs, maxLogs);
        int logsGained = logDist(randomGenerator);
        
        addLogsToPlayer(logsGained);
        logsGiven = true;
    }

    bool isPlayerEntity(Entity* entity) {
        return entity && (entity->getName() == "PlayerCollider" || 
                         entity->getName().find("Player") != std::string::npos);
    }
    
    bool isPlayerHoldingE() {
        GLFWwindow* window = glfwGetCurrentContext();
        return window && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
    }
    
    void addLogsToPlayer(int logs) {
        if (nearbyPlayer) {
            if (nearbyPlayer->hasComponent<PlayerInventory>()) {
                auto& inventory = nearbyPlayer->getComponent<PlayerInventory>();
                inventory.addLogs(logs);
            }
        }
    }
    
    void markTreeForRemoval() {
        if (getEntity() && !getEntity()->hasComponent<TreeRemovalMarker>()) {
            getEntity()->addComponent<TreeRemovalMarker>();
        }
        
        Entity* treeEntity = getTreeEntity();
        if (treeEntity && !treeEntity->hasComponent<TreeRemovalMarker>()) {
            treeEntity->addComponent<TreeRemovalMarker>();
        }
        
        Entity* treeModelEntity = getTreeModelEntity();
        if (treeModelEntity && !treeModelEntity->hasComponent<TreeRemovalMarker>()) {
            treeModelEntity->addComponent<TreeRemovalMarker>();
        }
    }

    void setSizeBasedParameters() {
        if (treeSizeValue <= 1.8f) {
            minLogs = 1;
            maxLogs = 1;
            chopDuration = 1.0f;
        } 
        else if (treeSizeValue <= 2.1f) {
            minLogs = 2;
            maxLogs = 2;
            chopDuration = 1.5f;
        } 
        else {
            minLogs = 3;
            maxLogs = 3;
            chopDuration = 2.0f;
        }
    }

    bool isPlayerNearby;
    Entity* nearbyPlayer;
    float chopTime;
    float chopDuration;
    bool chopInProgress;  // Renamed from m_isBeingChopped to avoid conflict with isBeingChopped()
    int minLogs;
    int maxLogs;
    std::mt19937 randomGenerator;  // Renamed from m_rng to avoid potential conflicts
    
    TreeState treeState;
    float animationTime;
    float fallDuration;
    float shrinkDuration;
    float shakeIntensity;
    float shakeSpeed;
    bool logsGiven;
    bool initialized;
    float treeSizeValue;  // Renamed from m_treeSize to avoid conflict with getTreeSize()
    
    glm::vec3 originalPosition;
    glm::vec3 originalRotation;
    glm::vec3 originalScale;
    glm::vec3 fallDirection;
};

class TreeSpawner {
public:
    struct TreeType {
        std::shared_ptr<Model> model;
        std::string name;
        glm::vec3 baseScale;
        float scaleVariation;
        glm::vec3 colliderSize;
        glm::vec3 triggerSize;
    };

    struct SpawnParameters {
        glm::vec2 spawnAreaMin = glm::vec2(-50.0f, -50.0f);
        glm::vec2 spawnAreaMax = glm::vec2(50.0f, 50.0f);
        float groundHeight = -3.0f;
        float minDistanceBetweenTrees = 3.0f;
        int maxTrees = 100;
        unsigned int seed = 12345;
        std::vector<glm::vec2> exclusionZones;
        float exclusionRadius = 5.0f;
    };

private:
    std::shared_ptr<Scene> scene;
    std::vector<TreeType> treeTypes;
    std::vector<std::shared_ptr<Entity>> spawnedTrees;
    std::vector<std::shared_ptr<Entity>> treeTriggers;
    std::mt19937 randomGenerator;  // Renamed from rng to match TreeCutter naming
    SpawnParameters parameters;    // Renamed from params to avoid conflict with function parameters
    
    struct GridCell {
        std::vector<glm::vec2> positions;
    };
    std::unordered_map<int, GridCell> spatialGrid;
    float gridCellSize;

public:
    TreeSpawner(std::shared_ptr<Scene> scene)
        : scene(scene), gridCellSize(5.0f) {
        randomGenerator.seed(parameters.seed);
    }

    ~TreeSpawner() {
        clearTrees();
    }

    void addTreeType(const TreeType& treeType) {
        treeTypes.push_back(treeType);
    }
    
    void setSpawnParameters(const SpawnParameters& newParams) {
        parameters = newParams;
        randomGenerator.seed(parameters.seed);
        gridCellSize = parameters.minDistanceBetweenTrees * 2.0f;
    }
    
    const SpawnParameters& getSpawnParameters() const { return parameters; }
    
    void generateTrees() {
        if (treeTypes.empty()) {
            return;
        }
        
        clearTrees();
        
        std::vector<glm::vec2> positions = generatePoissonDiskSamples();
        
        for (const auto& pos : positions) {
            if (spawnedTrees.size() >= parameters.maxTrees) break;
            
            int treeTypeIndex = getRandomTreeType();
            auto tree = spawnTree(pos, treeTypeIndex);
            if (tree) {
                spawnedTrees.push_back(tree);
            }
        }
    }
    
    void clearTrees() {
        for (auto& tree : spawnedTrees) {
            scene->removeEntity(tree);
        }
        for (auto& trigger : treeTriggers) {
            scene->removeEntity(trigger);
        }
        spawnedTrees.clear();
        treeTriggers.clear();
        spatialGrid.clear();
    }
    
    void update(float deltaTime) {
        for (int i = static_cast<int>(spawnedTrees.size()) - 1; i >= 0; --i) {
            auto& tree = spawnedTrees[i];
            auto& trigger = treeTriggers[i];
            
            bool shouldRemove = false;
            
            if (tree && tree->hasComponent<TreeRemovalMarker>()) {
                shouldRemove = true;
            }
            if (trigger && trigger->hasComponent<TreeRemovalMarker>()) {
                shouldRemove = true;
            }
            
            if (shouldRemove) {
                if (tree) {
                    scene->removeEntity(tree);
                }
                if (trigger) {
                    scene->removeEntity(trigger);
                }
                
                spawnedTrees.erase(spawnedTrees.begin() + i);
                treeTriggers.erase(treeTriggers.begin() + i);
            }
        }
    }
    
    const std::vector<std::shared_ptr<Entity>>& getSpawnedTrees() const { return spawnedTrees; }
    const std::vector<std::shared_ptr<Entity>>& getTreeTriggers() const { return treeTriggers; }
    
    void regenerate(unsigned int newSeed) {
        parameters.seed = newSeed;
        randomGenerator.seed(parameters.seed);
        generateTrees();
    }
    
    void addExclusionZone(glm::vec2 center, float radius) {
        parameters.exclusionZones.push_back(center);
        if (radius > parameters.exclusionRadius) {
            parameters.exclusionRadius = radius;
        }
    }
    
private:
    bool isValidPosition(const glm::vec2& position) {
        if (position.x < parameters.spawnAreaMin.x || position.x > parameters.spawnAreaMax.x ||
            position.y < parameters.spawnAreaMin.y || position.y > parameters.spawnAreaMax.y) {
            return false;
        }
        
        for (const auto& zone : parameters.exclusionZones) {
            float distance = glm::length(position - zone);
            if (distance < parameters.exclusionRadius) {
                return false;
            }
        }
        
        return checkDistanceToNearbyTrees(position);
    }
    
    glm::vec2 getRandomPosition() {
        std::uniform_real_distribution<float> xDist(parameters.spawnAreaMin.x, parameters.spawnAreaMax.x);
        std::uniform_real_distribution<float> zDist(parameters.spawnAreaMin.y, parameters.spawnAreaMax.y);
        return glm::vec2(xDist(randomGenerator), zDist(randomGenerator));
    }
    
    int getRandomTreeType() {
        std::uniform_int_distribution<int> dist(0, static_cast<int>(treeTypes.size() - 1));
        return dist(randomGenerator);
    }
    
    float getRandomRotation() {
        std::uniform_real_distribution<float> dist(0.0f, 360.0f);
        return dist(randomGenerator);
    }
    
    glm::vec3 getRandomScale(const TreeType& treeType) {
        std::uniform_real_distribution<float> dist(1.0f - treeType.scaleVariation, 
                                                   1.0f + treeType.scaleVariation);
        float scaleFactor = dist(randomGenerator);
        return treeType.baseScale * scaleFactor;
    }
    
    std::shared_ptr<Entity> spawnTree(const glm::vec2& position, int treeTypeIndex) {
        if (treeTypeIndex >= treeTypes.size()) {
            return nullptr;
        }
        
        const TreeType& treeType = treeTypes[treeTypeIndex];
        
        std::string treeName = treeType.name + "_" + std::to_string(spawnedTrees.size());
        auto treeRoot = scene->createEntity(treeName + "_Root");
        
        glm::vec3 pos3D(position.x, parameters.groundHeight, position.y);
        treeRoot->addComponent<Transform>(pos3D, glm::vec3(0.0f), glm::vec3(1.0f));
        
        treeRoot->addComponent<RigidBody>(RigidBody::BodyType::STATIC);
        treeRoot->addComponent<BoxCollider>(treeType.colliderSize);
        
        auto treeModel = scene->createEntity(treeName + "_Model");
        treeModel->setParent(treeRoot);
        
        glm::vec3 rotation(0.0f, getRandomRotation(), 0.0f);
        glm::vec3 scale = getRandomScale(treeType);
        
        treeModel->addComponent<Transform>(glm::vec3(0.0f), rotation, scale);
        treeModel->addComponent<ModelRenderer>(treeType.model);
        
        std::string triggerName = treeName + "_Trigger";
        auto trigger = scene->createEntity(triggerName);
        trigger->setParent(treeRoot);
        
        glm::vec3 triggerPos = glm::vec3(0.0f, 0.5f, 0.0f);
        trigger->addComponent<Transform>(triggerPos, glm::vec3(0.0f), glm::vec3(1.0f));
        trigger->addComponent<RigidBody>(RigidBody::BodyType::STATIC);
        
        glm::vec3 triggerSize = treeType.triggerSize;
        if (triggerSize == glm::vec3(0.0f)) {
            triggerSize = treeType.colliderSize * 1.5f;
        }
        
        trigger->addComponent<BoxCollider>(triggerSize).setIsTrigger(true);
        trigger->addComponent<TreeCutter>();
        
        treeTriggers.push_back(trigger);
        addToGrid(position);
        
        return treeRoot;
    }
    
    int getGridKey(const glm::vec2& position) {
        int gridX = static_cast<int>(position.x / gridCellSize);
        int gridY = static_cast<int>(position.y / gridCellSize);
        return gridX * 10000 + gridY;
    }
    
    void addToGrid(const glm::vec2& position) {
        int key = getGridKey(position);
        spatialGrid[key].positions.push_back(position);
    }
    
    bool checkDistanceToNearbyTrees(const glm::vec2& position) {
        int centerKey = getGridKey(position);
        
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                glm::vec2 checkPos = position + glm::vec2(dx * gridCellSize, dy * gridCellSize);
                int key = getGridKey(checkPos);
                
                auto it = spatialGrid.find(key);
                if (it != spatialGrid.end()) {
                    for (const auto& treePos : it->second.positions) {
                        float distance = glm::length(position - treePos);
                        if (distance < parameters.minDistanceBetweenTrees) {
                            return false;
                        }
                    }
                }
            }
        }
        
        return true;
    }
    
    std::vector<glm::vec2> generatePoissonDiskSamples() {
        std::vector<glm::vec2> samples;
        std::vector<glm::vec2> activeList;
        
        int gridDivisions = 3;
        float areaWidth = parameters.spawnAreaMax.x - parameters.spawnAreaMin.x;
        float areaHeight = parameters.spawnAreaMax.y - parameters.spawnAreaMin.y;
        
        for (int x = 0; x < gridDivisions; x++) {
            for (int y = 0; y < gridDivisions; y++) {
                float gridX = parameters.spawnAreaMin.x + (x + 0.5f) * (areaWidth / gridDivisions);
                float gridY = parameters.spawnAreaMin.y + (y + 0.5f) * (areaHeight / gridDivisions);
                
                std::uniform_real_distribution<float> offsetDist(-areaWidth / (gridDivisions * 3), 
                                                                 areaWidth / (gridDivisions * 3));
                glm::vec2 initialPoint(gridX + offsetDist(randomGenerator), gridY + offsetDist(randomGenerator));
                
                if (isValidPosition(initialPoint)) {
                    samples.push_back(initialPoint);
                    activeList.push_back(initialPoint);
                    addToGrid(initialPoint);
                }
            }
        }
        
        const int k = 30;
        
        while (!activeList.empty() && samples.size() < static_cast<size_t>(parameters.maxTrees)) {
            std::uniform_int_distribution<size_t> indexDist(0, activeList.size() - 1);
            size_t activeIndex = indexDist(randomGenerator);
            glm::vec2 activePoint = activeList[activeIndex];
            
            bool foundValid = false;
            
            for (int i = 0; i < k; i++) {
                std::uniform_real_distribution<float> radiusDist(parameters.minDistanceBetweenTrees, 
                                                                 parameters.minDistanceBetweenTrees * 2.0f);
                std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * static_cast<float>(M_PI));
                
                float radius = radiusDist(randomGenerator);
                float angle = angleDist(randomGenerator);
                
                glm::vec2 candidate = activePoint + glm::vec2(
                    radius * cos(angle),
                    radius * sin(angle)
                );
                
                if (isValidPosition(candidate)) {
                    samples.push_back(candidate);
                    activeList.push_back(candidate);
                    addToGrid(candidate);
                    foundValid = true;
                    break;
                }
            }
            
            if (!foundValid) {
                activeList.erase(activeList.begin() + activeIndex);
            }
        }
        
        return samples;
    }
};

}