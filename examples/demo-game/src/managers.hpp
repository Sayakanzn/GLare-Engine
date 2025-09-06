#pragma once

#include "GLare.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <vector>
#include <memory>
#include "player.hpp"

namespace GLR {

class CampfireManager : public CollisionResponder {
public:
    struct CampfireSettings {
        float maxFuel = 100.0f;
        float fuelBurnRate = 5.0f;
        float logsToFuelRatio = 10.0f;
        float minFireScale = 0.1f;
        float maxFireScale = 1.0f;
        float minLightRadius = 1.0f;
        float maxLightRadius = 8.0f;
        float scaleChangeSpeed = 2.0f;
        float lightChangeSpeed = 3.0f;
        float snowChangeSpeed = 1.5f;
    };

    CampfireManager() {}
    CampfireManager(std::shared_ptr<Entity> fireEntity, std::shared_ptr<Entity> lightEntity, std::shared_ptr<Material> snowMaterial, CampfireSettings settings)
        : fireEntity(fireEntity)
        , lightEntity(lightEntity)
        , snowMaterial(snowMaterial)
        , settings(settings)
        , currentFuel(50.0f)
        , targetFireScale(0.5f)
        , currentFireScale(0.5f)
        , targetLightRadius(4.0f)
        , currentLightRadius(4.0f)
        , currentSnowRadius(0.315f)
        , isPlayerNearby(false)
        , nearbyPlayer(nullptr)
        , lastFeedTime(0.0f) {
        
        updateTargetValues();
        currentSnowRadius = 0.2685f + (currentLightRadius * 0.0115f);
    }

    void init() override {
        CollisionResponder::init();
    }

    void update(float deltaTime) override {
        burnFuel(deltaTime);
        updateTargetValues();
        updateFireScale(deltaTime);
        updateLightRadius(deltaTime);
        updateSnowRadius(deltaTime);
        handlePlayerInput();
        
        if (lastFeedTime > 0.0f) {
            lastFeedTime -= deltaTime;
        }
    }

    bool feedLogs(int logCount) {
        if (logCount <= 0) return false;
        
        float fuelToAdd = logCount * settings.logsToFuelRatio;
        float oldFuel = currentFuel;
        currentFuel = std::min(currentFuel + fuelToAdd, settings.maxFuel);
        
        float actualFuelAdded = currentFuel - oldFuel;
        int actualLogsUsed = static_cast<int>(actualFuelAdded / settings.logsToFuelRatio);
        
        if (actualLogsUsed > 0) {
            lastFeedTime = 0.5f;
            return true;
        }
        
        return false;
    }

    void setCurrentFuel(float f) { currentFuel = f; }
    float getCurrentFuel() const { return currentFuel; }
    float getMaxFuel() const { return settings.maxFuel; }
    float getFuelPercentage() const { return currentFuel / settings.maxFuel; }
    bool getPlayerNearbyStatus() const { return isPlayerNearby; }
    
    void setFireEntity(std::shared_ptr<Entity> fireEntity) { this->fireEntity = fireEntity; }
    void setLightEntity(std::shared_ptr<Entity> lightEntity) { this->lightEntity = lightEntity; }
    void setSnowMaterial(std::shared_ptr<Material> snowMaterial) { this->snowMaterial = snowMaterial; }
    
    void updateSettings(const CampfireSettings& newSettings) {
        settings = newSettings;
        updateTargetValues();
    }    

protected:
    void handleTriggerEnter(Entity* otherEntity, const CollisionEvent& event) override {
        if (isPlayerEntity(otherEntity)) {
            isPlayerNearby = true;
            nearbyPlayer = otherEntity;
        }
    }
    
    void handleTriggerExit(Entity* otherEntity, const CollisionEvent& event) override {
        if (isPlayerEntity(otherEntity)) {
            isPlayerNearby = false;
            nearbyPlayer = nullptr;
        }
    }

private:
    void burnFuel(float deltaTime) {
        if (currentFuel > 0.0f) {
            currentFuel -= settings.fuelBurnRate * deltaTime;
            currentFuel = std::max(0.0f, currentFuel);
        }
    }
    
    void updateTargetValues() {
        float fuelPercentage = getFuelPercentage();
        
        targetFireScale = settings.minFireScale + (settings.maxFireScale - settings.minFireScale) * fuelPercentage;
        targetLightRadius = settings.minLightRadius + (settings.maxLightRadius - settings.minLightRadius) * fuelPercentage;
    }
    
    void updateFireScale(float deltaTime) {
        if (!fireEntity) return;
        
        currentFireScale = glm::mix(currentFireScale, targetFireScale, deltaTime * settings.scaleChangeSpeed);
        
        if (fireEntity->hasComponent<Transform>()) {
            auto& transform = fireEntity->getComponent<Transform>();
            glm::vec3 newScale = glm::vec3(currentFireScale);
            transform.setScale(newScale);
        }
    }
    
    void updateLightRadius(float deltaTime) {
        if (!lightEntity) return;
        
        currentLightRadius = glm::mix(currentLightRadius, targetLightRadius, deltaTime * settings.lightChangeSpeed);
        
        if (lightEntity->hasComponent<PointLight>()) {
            auto& pointLight = lightEntity->getComponent<PointLight>();
            pointLight.setRadius(currentLightRadius);
        }
    }
    
    void updateSnowRadius(float deltaTime) {
        if (!snowMaterial) return;
        
        float targetSnowWidth = 0.2685f + (currentLightRadius * 0.0115f);
        currentSnowRadius = glm::mix(currentSnowRadius, targetSnowWidth, deltaTime * settings.snowChangeSpeed);
        
        snowMaterial->setFloat("width", currentSnowRadius);
    }
    
    void handlePlayerInput() {
        if (!isPlayerNearby || !nearbyPlayer || lastFeedTime > 0.0f) return;
        
        GLFWwindow* window = glfwGetCurrentContext();
        if (window && glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            if (nearbyPlayer->hasComponent<PlayerInventory>()) {
                auto& inventory = nearbyPlayer->getComponent<PlayerInventory>();
                int availableLogs = inventory.getLogs();
                
                if (availableLogs > 0) {
                    float fuelSpace = settings.maxFuel - currentFuel;
                    int maxLogsToUse = static_cast<int>(fuelSpace / settings.logsToFuelRatio);
                    int logsToUse = std::min(availableLogs, std::max(1, maxLogsToUse));
                    
                    if (feedLogs(logsToUse)) {
                        inventory.removeLogs(logsToUse);
                    }
                } 
                else {
                    lastFeedTime = 1.0f;
                }
            }
        }
    }
    
    bool isPlayerEntity(Entity* entity) {
        return entity && (entity->getName() == "PlayerCollider" || entity->getName().find("Player") != std::string::npos);
    }

private:
    std::shared_ptr<Entity> fireEntity;
    std::shared_ptr<Entity> lightEntity;
    std::shared_ptr<Material> snowMaterial;
    CampfireSettings settings;
    float currentFuel;
    float targetFireScale;
    float currentFireScale;
    float targetLightRadius;
    float currentLightRadius;
    float currentSnowRadius;
    bool isPlayerNearby;
    Entity* nearbyPlayer;
    float lastFeedTime;
};

class LogCabinManager : public CollisionResponder {
public:
    struct CabinSettings {
        int maxLogs = 20;
        float logsToProgressRatio = 0.05f;
        float progressChangeSpeed = 2.0f;
        bool playCompletionSound = true;
        float completionCelebrationTime = 3.0f;
    };

    LogCabinManager() {}
    LogCabinManager(float* blueprintProgress, CabinSettings settings)
        : blueprintProgress(blueprintProgress)
        , settings(settings)
        , currentLogs(0)
        , targetProgress(0.0f)
        , isPlayerNearby(false)
        , nearbyPlayer(nullptr)
        , lastBuildTime(0.0f)
        , isCompleted(false)
        , completionTime(0.0f) {
        
        if (blueprintProgress) {
            targetProgress = *blueprintProgress;
            currentLogs = static_cast<int>(*blueprintProgress / settings.logsToProgressRatio);
        }
    }

    void init() override {
        CollisionResponder::init();
    }

    void update(float deltaTime) override {
        updateBlueprintProgress(deltaTime);
        
        if (isCompleted && completionTime > 0.0f) {
            completionTime -= deltaTime;
        }
        
        handlePlayerInput();
        
        if (lastBuildTime > 0.0f) {
            lastBuildTime -= deltaTime;
        }
    }

    bool addLogs(int logCount) {
        if (logCount <= 0 || isCompleted) return false;
        
        int oldLogs = currentLogs;
        currentLogs = std::min(currentLogs + logCount, settings.maxLogs);
        
        int actualLogsUsed = currentLogs - oldLogs;
        
        if (actualLogsUsed > 0) {
            targetProgress = currentLogs * settings.logsToProgressRatio;
            targetProgress = std::min(targetProgress, 1.0f);
            
            if (currentLogs >= settings.maxLogs && !isCompleted) {
                completeConstruction();
            }
            
            lastBuildTime = 0.5f;
            return true;
        }
        
        return false;
    }

    int getCurrentLogs() const { return currentLogs; }
    int getMaxLogs() const { return settings.maxLogs; }
    float getCurrentProgress() const { return (currentLogs * 100.0f) / settings.maxLogs; }
    bool getCompletionStatus() const { return isCompleted; }
    bool getPlayerNearbyStatus() const { return isPlayerNearby; }
    int getLogsNeeded() const { return std::max(0, settings.maxLogs - currentLogs); }
    
    void setBlueprintProgress(float* blueprintProgress) { this->blueprintProgress = blueprintProgress; }
    
    void updateSettings(const CabinSettings& newSettings) {
        settings = newSettings;
        targetProgress = currentLogs * settings.logsToProgressRatio;
        targetProgress = std::min(targetProgress, 1.0f);
    }

    void resetCabin() {
        currentLogs = 0;
        targetProgress = 0.0f;
        isCompleted = false;
        completionTime = 0.0f;
        isPlayerNearby = false;
        nearbyPlayer = nullptr;
        lastBuildTime = 0.0f;
        
        if (blueprintProgress) {
            *blueprintProgress = 0.0f;
        }
    }

protected:
    void handleTriggerEnter(Entity* otherEntity, const CollisionEvent& event) override {
        if (isPlayerEntity(otherEntity)) {
            isPlayerNearby = true;
            nearbyPlayer = otherEntity;
        }
    }
    
    void handleTriggerExit(Entity* otherEntity, const CollisionEvent& event) override {
        if (isPlayerEntity(otherEntity)) {
            isPlayerNearby = false;
            nearbyPlayer = nullptr;
        }
    }

private:
    void updateBlueprintProgress(float deltaTime) {
        if (!blueprintProgress) return;
        
        float currentProgress = *blueprintProgress;
        float newProgress = glm::mix(currentProgress, targetProgress, deltaTime * settings.progressChangeSpeed);
        *blueprintProgress = newProgress;
    }
    
    void completeConstruction() {
        isCompleted = true;
        completionTime = settings.completionCelebrationTime;
        targetProgress = 1.0f;
    }
    
    void handlePlayerInput() {
        if (!isPlayerNearby || !nearbyPlayer || lastBuildTime > 0.0f) return;
        if (isCompleted) return;
        
        GLFWwindow* window = glfwGetCurrentContext();
        if (window && glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            if (nearbyPlayer->hasComponent<PlayerInventory>()) {
                auto& inventory = nearbyPlayer->getComponent<PlayerInventory>();
                int availableLogs = inventory.getLogs();
                
                if (availableLogs > 0) {
                    int logsNeeded = getLogsNeeded();
                    int logsToUse = std::min(availableLogs, logsNeeded);
                    
                    if (logsToUse > 0 && addLogs(logsToUse)) {
                        inventory.removeLogs(logsToUse);
                    }
                } 
                else {
                    lastBuildTime = 1.0f;
                }
            }
        }
    }
    
    bool isPlayerEntity(Entity* entity) {
        return entity && (entity->getName() == "PlayerCollider" || entity->getName().find("Player") != std::string::npos);
    }

private:
    float* blueprintProgress;
    CabinSettings settings;
    int currentLogs;
    float targetProgress;
    bool isCompleted;
    float completionTime;
    bool isPlayerNearby;
    Entity* nearbyPlayer;
    float lastBuildTime;
};

}