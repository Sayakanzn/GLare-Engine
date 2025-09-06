#pragma once

#include "GLare.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <random>
#include <vector>
#include <memory>

namespace GLR {

class PlayerLogStack : public Component {
public:
    PlayerLogStack(std::shared_ptr<Model> logModel, std::shared_ptr<Entity> playerEntity = nullptr) 
        : logModel(logModel)
        , playerEntity(playerEntity)
        , currentLogCount(0)
        , stackHeight(0.15f)
        , baseOffset(glm::vec3(0.0f, 0.8f, -0.3f))
        , logScale(glm::vec3(0.3f, 0.3f, 0.3f))
        , maxVisibleLogs(20)
        , rotationVariation(15.0f)
        , popAnimationDuration(0.4f)
        , popHeight(0.1f)
        , popScaleMultiplier(1.2f)
        , swayIntensity(0.08f)
        , swaySpeed(12.0f)
        , jiggleIntensity(0.05f)
        , jiggleSpeed(25.0f)
        , dampingFactor(0.92f)
        , stackPhaseOffset(0.3f)
        , heightMultiplier(1.5f)
        , isPlayerMoving(false)
        , isPlayerInteracting(false)
        , previousPlayerPosition(0.0f)
        , playerVelocity(0.0f)
        , movementTime(0.0f)
        , interactionTime(0.0f)
        , movementSmoothing(0.0f)
        , interactionIntensity(0.0f) {
        std::random_device rd;
        rng.seed(rd());
    }
    
    void update(float deltaTime) override {
        updatePlayerState(deltaTime);
        updateLogAnimations(deltaTime);
        updateMovementAnimations(deltaTime);
    }
    
    void updateLogCount(int newLogCount) {
        if (newLogCount == currentLogCount) {
            return;
        }
        
        if (newLogCount > currentLogCount) {
            addLogs(newLogCount - currentLogCount);
        } 
        else {
            removeLogs(currentLogCount - newLogCount);
        }
        
        currentLogCount = newLogCount;
    }
    
    int getCurrentLogCount() const { return currentLogCount; }
    
    void setPopAnimationDuration(float duration) { popAnimationDuration = duration; }
    void setPopHeight(float height) { popHeight = height; }
    void setPopScaleMultiplier(float multiplier) { popScaleMultiplier = multiplier; }
    void setSwayIntensity(float intensity) { swayIntensity = intensity; }
    void setSwaySpeed(float speed) { swaySpeed = speed; }
    void setJiggleIntensity(float intensity) { jiggleIntensity = intensity; }
    void setJiggleSpeed(float speed) { jiggleSpeed = speed; }
    void setDampingFactor(float damping) { dampingFactor = damping; }
    void setStackPhaseOffset(float offset) { stackPhaseOffset = offset; }
    void setHeightMultiplier(float multiplier) { heightMultiplier = multiplier; }
    void setStackHeight(float height) { stackHeight = height; }
    void setBaseOffset(const glm::vec3& offset) { baseOffset = offset; }
    void setLogScale(const glm::vec3& scale) { logScale = scale; }
    void setMaxVisibleLogs(int maxLogs) { maxVisibleLogs = maxLogs; }
    void setRotationVariation(float variation) { rotationVariation = variation; }
    void setPlayerEntity(std::shared_ptr<Entity> player) { playerEntity = player; }
    
    void clearAllLogs() {
        animatingLogs.clear();
        
        for (auto& logEntity : logEntities) {
            if (logEntity && logEntity->getScene()) {
                logEntity->getScene()->removeEntity(logEntity);
            }
        }
        logEntities.clear();
        logBaseRotations.clear();
        currentLogCount = 0;
    }

    void setPlayerInteracting(bool interacting) {
        isPlayerInteracting = interacting;
        if (interacting) {
            interactionTime = 0.0f;
        }
    }

private:
    struct AnimatingLog {
        std::shared_ptr<Entity> entity;
        float animationTime;
        float animationDuration;
        glm::vec3 targetPosition;
        glm::vec3 targetScale;
        glm::vec3 basePosition;
        glm::vec3 swayOffset;
        glm::vec3 jiggleOffset;
        glm::vec3 baseRotation;
        float individualPhase;
        
        AnimatingLog(std::shared_ptr<Entity> e, float duration, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rotation)
            : entity(e), animationTime(0.0f), animationDuration(duration), targetPosition(pos), targetScale(scale)
            , basePosition(pos), swayOffset(0.0f), jiggleOffset(0.0f), baseRotation(rotation), individualPhase(0.0f) {}
    };

    void updatePlayerState(float deltaTime) {
        if (!playerEntity) return;
        
        auto& transform = playerEntity->getComponent<Transform>();
        glm::vec3 currentPosition = transform.getPosition();
        
        playerVelocity = (currentPosition - previousPlayerPosition) / deltaTime;
        previousPlayerPosition = currentPosition;
        
        glm::vec2 horizontalVelocity(playerVelocity.x, playerVelocity.z);
        float speed = glm::length(horizontalVelocity);
        isPlayerMoving = speed > 0.1f;
        
        float targetMovement = isPlayerMoving ? glm::min(speed / 3.0f, 1.0f) : 0.0f;
        movementSmoothing = glm::mix(movementSmoothing, targetMovement, deltaTime * 5.0f);
        
        movementTime += deltaTime;
        interactionTime += deltaTime;
        
        float targetInteraction = isPlayerInteracting ? 1.0f : 0.0f;
        interactionIntensity = glm::mix(interactionIntensity, targetInteraction, deltaTime * 8.0f);
    }
    
    void updateMovementAnimations(float deltaTime) {
        for (size_t i = 0; i < logEntities.size(); ++i) {
            auto& logEntity = logEntities[i];
            if (!logEntity || !logEntity->hasComponent<Transform>()) continue;
            
            auto& transform = logEntity->getComponent<Transform>();
            
            glm::vec3 basePos = baseOffset;
            basePos.y += i * stackHeight;
            
            float heightFactor = 1.0f + (float(i) / std::max(1.0f, float(logEntities.size()))) * heightMultiplier;
            float logPhase = float(i) * stackPhaseOffset;
            
            glm::vec3 swayOffset(0.0f);
            if (movementSmoothing > 0.01f) {
                glm::vec2 horizontalVel(playerVelocity.x, playerVelocity.z);
                float velMagnitude = glm::length(horizontalVel);
                
                if (velMagnitude > 0.01f) {
                    glm::vec2 normalizedVel = horizontalVel / velMagnitude;
                    
                    float swayPhase = movementTime * swaySpeed + logPhase;
                    float swayAmount = swayIntensity * movementSmoothing * heightFactor;
                    
                    swayOffset.x = sin(swayPhase) * swayAmount * normalizedVel.y;
                    swayOffset.z = sin(swayPhase) * swayAmount * -normalizedVel.x;
                    swayOffset.y = sin(swayPhase * 0.5f) * swayAmount * 0.3f;
                    
                    float momentumPhase = swayPhase - 0.5f;
                    swayOffset.x += cos(momentumPhase) * swayAmount * 0.4f * normalizedVel.x;
                    swayOffset.z += cos(momentumPhase) * swayAmount * 0.4f * normalizedVel.y;
                }
            }
            
            glm::vec3 jiggleOffset(0.0f);
            if (interactionIntensity > 0.01f) {
                float jigglePhase = interactionTime * jiggleSpeed + logPhase;
                float jiggleAmount = jiggleIntensity * interactionIntensity * heightFactor;
                
                jiggleOffset.x = sin(jigglePhase * 1.3f) * jiggleAmount;
                jiggleOffset.y = sin(jigglePhase * 1.7f) * jiggleAmount * 0.5f;
                jiggleOffset.z = sin(jigglePhase * 0.9f) * jiggleAmount;
                
                float randomPhase = sin(jigglePhase * 2.3f + logPhase * 3.0f);
                jiggleOffset += glm::vec3(
                    sin(jigglePhase * 3.1f) * randomPhase * jiggleAmount * 0.3f,
                    cos(jigglePhase * 2.7f) * randomPhase * jiggleAmount * 0.2f,
                    sin(jigglePhase * 3.7f) * randomPhase * jiggleAmount * 0.3f
                );
            }
            
            glm::vec3 totalOffset = swayOffset + jiggleOffset;
            
            if (glm::length(totalOffset) > 0.001f) {
                totalOffset *= dampingFactor;
            }
            
            glm::vec3 finalPosition = basePos + totalOffset;
            transform.setPosition(finalPosition);
            
            glm::vec3 baseRotation;
            if (i < logBaseRotations.size()) {
                baseRotation = logBaseRotations[i];
            } 
            else {
                baseRotation = transform.getRotation();
            }
            
            glm::vec3 finalRotation = baseRotation;
            
            if (movementSmoothing > 0.01f || interactionIntensity > 0.01f) {
                float rotationIntensity = (movementSmoothing + interactionIntensity) * 0.5f;
                
                float rotPhase = (movementTime + interactionTime) * 8.0f + logPhase;
                float rotAmount = rotationIntensity * heightFactor * 3.0f;
                
                finalRotation.x = baseRotation.x + sin(rotPhase * 1.2f) * rotAmount * 0.5f;
                finalRotation.z = baseRotation.z + sin(rotPhase * 0.8f) * rotAmount;
            }
            
            transform.setRotation(finalRotation);
        }
        
        for (auto& animLog : animatingLogs) {
            if (!animLog.entity || !animLog.entity->hasComponent<Transform>()) continue;
            
            auto& transform = animLog.entity->getComponent<Transform>();
            glm::vec3 currentPos = transform.getPosition();
            
            float reducedMovement = movementSmoothing * 0.3f;
            float reducedInteraction = interactionIntensity * 0.2f;
            
            if (reducedMovement > 0.01f || reducedInteraction > 0.01f) {
                glm::vec3 minorOffset(sin(movementTime * 15.0f) * (reducedMovement + reducedInteraction) * 0.02f, 0.0f, cos(movementTime * 12.0f) * (reducedMovement + reducedInteraction) * 0.02f);
                transform.setPosition(currentPos + minorOffset);
            }
        }
    }
    
    void addLogs(int count) {
        if (!playerEntity || !logModel) {
            return;
        }
        
        for (int i = 0; i < count; ++i) {
            if (logEntities.size() >= maxVisibleLogs) {
                break;
            }
            
            std::string logName = "PlayerLog_" + std::to_string(logEntities.size());
            auto logEntity = playerEntity->getScene()->createEntity(logName);
            logEntity->setParent(playerEntity);
            
            glm::vec3 finalPosition = baseOffset;
            finalPosition.y += logEntities.size() * stackHeight;
            
            std::uniform_real_distribution<float> posDist(-0.02f, 0.02f);
            std::uniform_real_distribution<float> rotDist(-rotationVariation, rotationVariation);
            std::uniform_real_distribution<float> phaseDist(0.0f, 6.28f);
            
            finalPosition.x += posDist(rng);
            finalPosition.z += posDist(rng);
            
            glm::vec3 logRotation(rotDist(rng), rotDist(rng) + 90.0f, rotDist(rng));
            
            logBaseRotations.push_back(logRotation);
            
            glm::vec3 startPosition = finalPosition;
            startPosition.y -= popHeight;
            
            glm::vec3 startScale = glm::vec3(0.1f, 0.1f, 0.1f);
            
            logEntity->addComponent<Transform>(startPosition, logRotation, startScale);
            logEntity->addComponent<ModelRenderer>(logModel);
            
            AnimatingLog animLog(logEntity, popAnimationDuration, finalPosition, logScale, logRotation);
            animLog.individualPhase = phaseDist(rng);
            animatingLogs.push_back(animLog);
            
            logEntities.push_back(logEntity);
        }
    }
    
    void removeLogs(int count) {
        while (count > 0 && !animatingLogs.empty()) {
            auto& animatingLog = animatingLogs.back();
            
            if (animatingLog.entity && animatingLog.entity->getScene()) {
                animatingLog.entity->getScene()->removeEntity(animatingLog.entity);
            }
            
            animatingLogs.pop_back();
            if (!logEntities.empty()) {
                logEntities.pop_back();
            }
            if (!logBaseRotations.empty()) {
                logBaseRotations.pop_back();
            }
            
            count--;
        }
        
        while (count > 0 && !logEntities.empty()) {
            auto logEntity = logEntities.back();
            logEntities.pop_back();
            
            if (!logBaseRotations.empty()) {
                logBaseRotations.pop_back();
            }
            
            if (logEntity && logEntity->getScene()) {
                logEntity->getScene()->removeEntity(logEntity);
            }
            
            count--;
        }
    }
    
    void updateLogAnimations(float deltaTime) {
        auto it = animatingLogs.begin();
        while (it != animatingLogs.end()) {
            AnimatingLog& animLog = *it;
            animLog.animationTime += deltaTime;
            
            if (animLog.animationTime >= animLog.animationDuration) {
                if (animLog.entity && animLog.entity->hasComponent<Transform>()) {
                    auto& transform = animLog.entity->getComponent<Transform>();
                    transform.setPosition(animLog.targetPosition);
                    transform.setScale(animLog.targetScale);
                }
                
                it = animatingLogs.erase(it);
            } 
            else {
                animateLog(animLog);
                ++it;
            }
        }
    }
    
    void animateLog(AnimatingLog& animLog) {
        if (!animLog.entity || !animLog.entity->hasComponent<Transform>()) {
            return;
        }
        
        auto& transform = animLog.entity->getComponent<Transform>();
        float progress = animLog.animationTime / animLog.animationDuration;
        
        float easedProgress = easeOutBounce(progress);
        float scaleProgress = easeOutBack(progress);
        
        glm::vec3 startPos = animLog.targetPosition;
        startPos.y -= popHeight;
        
        glm::vec3 currentPos = glm::mix(startPos, animLog.targetPosition, easedProgress);
        
        glm::vec3 startScale(0.1f, 0.1f, 0.1f);
        glm::vec3 overshootScale = animLog.targetScale * popScaleMultiplier;
        
        glm::vec3 currentScale;
        if (scaleProgress < 0.7f) {
            float overshootProgress = scaleProgress / 0.7f;
            currentScale = glm::mix(startScale, overshootScale, overshootProgress);
        } 
        else {
            float settleProgress = (scaleProgress - 0.7f) / 0.3f;
            currentScale = glm::mix(overshootScale, animLog.targetScale, settleProgress);
        }
        
        transform.setPosition(currentPos);
        transform.setScale(currentScale);
    }
    
    float easeOutBounce(float t) {
        if (t < 1.0f / 2.75f) {
            return 7.5625f * t * t;
        } 
        else if (t < 2.0f / 2.75f) {
            t -= 1.5f / 2.75f;
            return 7.5625f * t * t + 0.75f;
        } 
        else if (t < 2.5f / 2.75f) {
            t -= 2.25f / 2.75f;
            return 7.5625f * t * t + 0.9375f;
        } 
        else {
            t -= 2.625f / 2.75f;
            return 7.5625f * t * t + 0.984375f;
        }
    }
    
    float easeOutBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
    }

private:
    std::shared_ptr<Model> logModel;
    std::shared_ptr<Entity> playerEntity;
    std::vector<std::shared_ptr<Entity>> logEntities;
    std::vector<AnimatingLog> animatingLogs;
    
    int currentLogCount;
    float stackHeight;
    glm::vec3 baseOffset;
    glm::vec3 logScale;
    int maxVisibleLogs;
    float rotationVariation;
    
    float popAnimationDuration;
    float popHeight;
    float popScaleMultiplier;
    
    float swayIntensity;
    float swaySpeed;
    float jiggleIntensity;
    float jiggleSpeed;
    float dampingFactor;
    float stackPhaseOffset;
    float heightMultiplier;
    
    bool isPlayerMoving;
    bool isPlayerInteracting;
    glm::vec3 previousPlayerPosition;
    glm::vec3 playerVelocity;
    float movementTime;
    float interactionTime;
    float movementSmoothing;
    float interactionIntensity;
    
    std::vector<glm::vec3> logBaseRotations;
    
    std::mt19937 rng;
};

class PlayerInventory : public Component {
public:
    PlayerInventory() : logs(0), playerLogStack(nullptr) {}
    
    void addLogs(int amount) { 
        logs += amount; 
        
        if (playerLogStack) {
            playerLogStack->updateLogCount(logs);
        }
    }
    
    void removeLogs(int amount) { 
        logs = std::max(0, logs - amount);
        
        if (playerLogStack) {
            playerLogStack->updateLogCount(logs);
        }
    }
    
    int getLogs() const { return logs; }
    
    void setLogs(int logCount) { 
        logs = logCount; 
        
        if (playerLogStack) {
            playerLogStack->updateLogCount(logs);
        }
    }
    
    void setPlayerLogStack(PlayerLogStack* manager) {
        playerLogStack = manager;
        if (playerLogStack) {
            playerLogStack->updateLogCount(logs);
        }
    }
    
    PlayerLogStack* getPlayerLogStack() const { return playerLogStack; }
    
private:
    int logs;
    PlayerLogStack* playerLogStack;
};

class PlayerController {
public:
    PlayerController(Entity* playerEntity, Entity* modelEntity = nullptr) 
        : playerEntity(playerEntity)
        , modelEntity(modelEntity)
        , moveSpeed(2.0f)
        , rotationSpeed(10.0f)
        , maxVelocity(6.0f)
        , dampingFactor(0.9f)
        , forceMagnitude(50.0f)
        , wobbleAmount(0.5f)
        , wobbleSpeed(20.0f)
        , wobbleTime(0.0f)
        , isMoving(false)
        , currentRotation(0.0f)
        , targetRotation(0.0f)
        , moveInput(0.0f)
        , lastMoveDirection(0.0f, 0.0f, -1.0f)
        , rigidBody(nullptr)
        , modelBaseY(0.0f)
        , modelBaseYInitialized(false)
        , currentAnimationState(AnimationState::IDLE)
        , playerModel(nullptr)
        , isAttacking(false)
        , isInteracting(false)
        , interactAnimationTimer(0.0f)
        , interactAnimationDuration(1.0f)
        , coldSpeedMultiplier(1.0f)
        , isSprinting(false)
        , sprintTimer(0.0f)
        , sprintCooldownTimer(0.0f)
        , sprintDuration(1.0f)
        , sprintCooldown(10.0f)
        , sprintSpeedMultiplier(1.5f)
        , sprintAcceleration(8.0f)
        , currentSprintMultiplier(1.0f) {
        
        initialize();
    }

    void setColdSpeedMultiplier(float multiplier) {
        coldSpeedMultiplier = std::clamp(multiplier, 0.1f, 1.0f);
    }
    
    float getColdSpeedMultiplier() const {
        return coldSpeedMultiplier;
    }

    void setSprintDuration(float duration) { sprintDuration = duration; }
    float getSprintDuration() const { return sprintDuration; }
    
    void setSprintCooldown(float cooldown) { sprintCooldown = cooldown; }
    float getSprintCooldown() const { return sprintCooldown; }
    
    void setSprintSpeedMultiplier(float multiplier) { sprintSpeedMultiplier = multiplier; }
    float getSprintSpeedMultiplier() const { return sprintSpeedMultiplier; }
    
    void setSprintAcceleration(float acceleration) { sprintAcceleration = acceleration; }
    float getSprintAcceleration() const { return sprintAcceleration; }
    
    bool isPlayerSprinting() const { return isSprinting; }
    bool canSprint() const { return sprintCooldownTimer <= 0.0f && !isAttacking && !isInteracting; }
    
    float getSprintCooldownRemaining() const { return std::max(0.0f, sprintCooldownTimer); }
    float getSprintTimeRemaining() const { return isSprinting ? std::max(0.0f, sprintDuration - sprintTimer) : 0.0f; }

    void initialize() {
        if (!playerEntity) return;
        
        if (playerEntity->hasComponent<RigidBody>()) {
            rigidBody = &playerEntity->getComponent<RigidBody>();
            
            rigidBody->getInternalBody()->setAngularLockAxisFactor(reactphysics3d::Vector3(0, 0, 0));
            
            auto& transform = playerEntity->getComponent<Transform>();
            currentRotation = transform.getRotation().y;
            targetRotation = currentRotation;
        }
        
        if (modelEntity) {
            auto& modelTransform = modelEntity->getComponent<Transform>();
            modelBaseY = modelTransform.getPosition().y;
            modelBaseYInitialized = true;
            
            if (modelEntity->hasComponent<ModelRenderer>()) {
                auto& modelRenderer = modelEntity->getComponent<ModelRenderer>();
                playerModel = modelRenderer.getModel().get();
                
                if (playerModel && playerModel->getAnimationManager()) {
                    playerModel->getAnimationManager()->playAnimation("idle", true);
                    currentAnimationState = AnimationState::IDLE;
                }
            }
        }
    }

    void update(float deltaTime) {
        if (!rigidBody || !playerEntity) return;

        updateSprint(deltaTime);
        updateInteractAnimation(deltaTime);
        applyMovement();
        updateRotation(deltaTime);
        updateAnimations();
        updateWobbleAnimation(deltaTime);
        
        if (glm::length(moveInput) < 0.01f) {
            applyDamping();
        }
    }

    void setMoveInput(const glm::vec3& input) {
        if (isAttacking || isInteracting) {
            moveInput = glm::vec3(0.0f);
            isMoving = false;
            return;
        }
        
        moveInput = input;
        moveInput.y = 0.0f;
        
        if (glm::length(moveInput) > 1.0f) {
            moveInput = glm::normalize(moveInput);
        }
        
        isMoving = glm::length(moveInput) > 0.01f;
        
        if (isMoving) {
            lastMoveDirection = moveInput;
            
            targetRotation = atan2(moveInput.x, moveInput.z);
            targetRotation = glm::degrees(targetRotation);
        }
    }

    void setAttackInput(bool attacking) {
        if (attacking && !isAttacking) {
            isAttacking = true;
            moveInput = glm::vec3(0.0f);
            isMoving = false;
            
            if (isSprinting) {
                isSprinting = false;
                sprintTimer = 0.0f;
                sprintCooldownTimer = sprintCooldown;
            }
        } 
        else if (!attacking && isAttacking) {
            isAttacking = false;
        }
    }

    void setInteractInput(bool interacting) {
        static bool lastInteractState = false;
        
        if (interacting && !lastInteractState && !isInteracting && !isAttacking) {
            startInteract();
        }
        
        lastInteractState = interacting;
    }

    void setSprintInput(bool sprinting) {
        static bool lastSprintState = false;
        
        if (sprinting && !lastSprintState && canSprint() && isMoving) {
            startSprint();
        }
        
        lastSprintState = sprinting;
    }

    void setModelEntity(Entity* entity) {
        modelEntity = entity;
        
        if (modelEntity) {
            auto& modelTransform = modelEntity->getComponent<Transform>();
            modelBaseY = modelTransform.getPosition().y;
            modelBaseYInitialized = true;
            
            if (modelEntity->hasComponent<ModelRenderer>()) {
                auto& modelRenderer = modelEntity->getComponent<ModelRenderer>();
                playerModel = modelRenderer.getModel().get();
            }
        }
    }

    void setMoveSpeed(float speed) { moveSpeed = speed; }
    float getMoveSpeed() const { return moveSpeed; }
    
    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    float getRotationSpeed() const { return rotationSpeed; }
    
    void setMaxVelocity(float velocity) { maxVelocity = velocity; }
    float getMaxVelocity() const { return maxVelocity; }
    
    void setDampingFactor(float damping) { dampingFactor = damping; }
    float getDampingFactor() const { return dampingFactor; }
    
    void setWobbleAmount(float amount) { wobbleAmount = amount; }
    float getWobbleAmount() const { return wobbleAmount; }
    
    void setWobbleSpeed(float speed) { wobbleSpeed = speed; }
    float getWobbleSpeed() const { return wobbleSpeed; }

    void setLinearLockAxis(bool lockX, bool lockY, bool lockZ) {
        if (rigidBody) {
            rigidBody->getInternalBody()->setLinearLockAxisFactor(reactphysics3d::Vector3(lockX ? 0 : 1, lockY ? 0 : 1, lockZ ? 0 : 1));
        }
    }

    void setAngularLockAxis(bool lockX, bool lockY, bool lockZ) {
        if (rigidBody) {
            rigidBody->getInternalBody()->setAngularLockAxisFactor(reactphysics3d::Vector3(lockX ? 0 : 1, lockY ? 0 : 1, lockZ ? 0 : 1));
        }
    }

private:
    enum class AnimationState {
        IDLE,
        WALKING,
        SPRINTING,
        ATTACKING,
        INTERACTING
    };

    void startSprint() {
        if (!canSprint()) return;
        
        isSprinting = true;
        sprintTimer = 0.0f;
    }

    void updateSprint(float deltaTime) {
        if (sprintCooldownTimer > 0.0f) {
            sprintCooldownTimer -= deltaTime;
        }
        
        if (isSprinting) {
            sprintTimer += deltaTime;
            
            if (sprintTimer >= sprintDuration || !isMoving) {
                isSprinting = false;
                sprintTimer = 0.0f;
                sprintCooldownTimer = sprintCooldown;
            }
        }
        
        float targetMultiplier = isSprinting ? sprintSpeedMultiplier : 1.0f;
        
        if (currentSprintMultiplier != targetMultiplier) {
            float accelerationRate = sprintAcceleration * deltaTime;
            
            if (isSprinting) {
                currentSprintMultiplier = std::min(currentSprintMultiplier + accelerationRate, targetMultiplier);
            } 
            else {
                currentSprintMultiplier = std::max(currentSprintMultiplier - accelerationRate * 1.5f, targetMultiplier);
            }
        }
    }

    void startInteract() {
        isInteracting = true;
        interactAnimationTimer = 0.0f;
        moveInput = glm::vec3(0.0f);
        isMoving = false;
        
        if (isSprinting) {
            isSprinting = false;
            sprintTimer = 0.0f;
            sprintCooldownTimer = sprintCooldown;
        }

        if (playerModel && playerModel->getAnimationManager()) {
            interactAnimationDuration = 1.0f;
        }
    }

    void updateInteractAnimation(float deltaTime) {
        if (!isInteracting) return;
        
        interactAnimationTimer += deltaTime;
        
        if (interactAnimationTimer >= interactAnimationDuration) {
            isInteracting = false;
            interactAnimationTimer = 0.0f;
        }
    }

    void updateAnimations() {
        if (!playerModel || !playerModel->getAnimationManager()) return;
     
        AnimationState targetState;
     
        if (isInteracting) {
            targetState = AnimationState::INTERACTING;
        } 
        else if (isAttacking) {
            targetState = AnimationState::ATTACKING;
        } 
        else if (currentSprintMultiplier > 1.2f && isMoving) {
            targetState = AnimationState::SPRINTING;
        } 
        else if (isMoving) {
            targetState = AnimationState::WALKING;
        } 
        else {
            targetState = AnimationState::IDLE;
        }
     
        if (targetState != currentAnimationState) {            
            switch (targetState) {
                case AnimationState::IDLE:
                    playerModel->getAnimationManager()->playAnimation("idle", true);
                    playerModel->getAnimationManager()->setSpeed(0.5f);
                    break;
                case AnimationState::WALKING:
                    playerModel->getAnimationManager()->playAnimation("walk", true);
                    playerModel->getAnimationManager()->setSpeed(1.0f);
                    break;
                case AnimationState::SPRINTING:
                    playerModel->getAnimationManager()->playAnimation("sprint", true);
                    playerModel->getAnimationManager()->setSpeed(1.2f);
                    break;
                case AnimationState::ATTACKING:
                    playerModel->getAnimationManager()->playAnimation("attack-melee-right", true);
                    playerModel->getAnimationManager()->setSpeed(0.9f);
                    break;
                case AnimationState::INTERACTING:
                    playerModel->getAnimationManager()->playAnimation("interact-right", false);
                    playerModel->getAnimationManager()->setSpeed(1.0f);
                    break;
            }
         
            currentAnimationState = targetState;
        }
        
        if (targetState == AnimationState::WALKING || targetState == AnimationState::SPRINTING) {
            float animationSpeed = 0.8f + (currentSprintMultiplier - 1.0f) * 0.8f;
            playerModel->getAnimationManager()->setSpeed(animationSpeed);
        }
    }

    void applyMovement() {
        if (isAttacking || isInteracting) {
            glm::vec3 velocity = rigidBody->getLinearVelocity();
            velocity.x *= 0.7f;
            velocity.z *= 0.7f;
            rigidBody->setLinearVelocity(velocity);
            return;
        }
        
        glm::vec3 velocity = rigidBody->getLinearVelocity();
        float currentY = velocity.y;
        
        if (glm::length(moveInput) > 0.01f) {
            float effectiveMoveSpeed = moveSpeed * coldSpeedMultiplier * currentSprintMultiplier;
            
            glm::vec3 force = moveInput * effectiveMoveSpeed * forceMagnitude;
            force.y = 0.0f;
            
            rigidBody->applyForce(force);
            
            if (std::abs(moveInput.x) < 0.01f) {
                velocity.x *= dampingFactor;
            }
            if (std::abs(moveInput.z) < 0.01f) {
                velocity.z *= dampingFactor;
            }
            
            velocity.y = 0.0f;
            
            float effectiveMaxVelocity = maxVelocity * coldSpeedMultiplier * currentSprintMultiplier;
            
            if (glm::length(velocity) > effectiveMaxVelocity) {
                velocity = glm::normalize(velocity) * effectiveMaxVelocity;
            }
            
            velocity.y = currentY;
            rigidBody->setLinearVelocity(velocity);
        }
    }

    void updateRotation(float deltaTime) {
        if (!isMoving) return;
        
        float effectiveRotationSpeed = rotationSpeed * coldSpeedMultiplier;
        if (isSprinting) {
            effectiveRotationSpeed *= 1.5f;
        }
        
        float rotationDiff = targetRotation - currentRotation;
        
        while (rotationDiff > 180.0f) rotationDiff -= 360.0f;
        while (rotationDiff < -180.0f) rotationDiff += 360.0f;
        
        currentRotation += rotationDiff * effectiveRotationSpeed * deltaTime;
        
        while (currentRotation > 360.0f) currentRotation -= 360.0f;
        while (currentRotation < 0.0f) currentRotation += 360.0f;
        
        auto& transform = playerEntity->getComponent<Transform>();
        
        glm::quat newOrientation = glm::angleAxis(glm::radians(currentRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        
        transform.setRotation(newOrientation);
    }

    void updateWobbleAnimation(float deltaTime) {
        if (!modelEntity) return;
        
        auto& modelTransform = modelEntity->getComponent<Transform>();
        
        if (!modelBaseYInitialized) {
            modelBaseY = modelTransform.getPosition().y;
            modelBaseYInitialized = true;
        }
        
        if (isMoving) {
            float effectiveWobbleSpeed = wobbleSpeed * coldSpeedMultiplier;
            if (isSprinting) {
                effectiveWobbleSpeed *= 1.8f;
            }
            
            wobbleTime += deltaTime * effectiveWobbleSpeed;
            
            float wobbleIntensity = isSprinting ? 1.5f : 1.0f;
            float wobbleZ = sin(wobbleTime) * wobbleAmount * 5.0f * wobbleIntensity;
            float wobbleX = sin(wobbleTime * 0.5f) * wobbleAmount * 2.5f * wobbleIntensity;
            
            glm::vec3 modelRotation = modelTransform.getRotation();
            modelRotation.x = wobbleX;
            modelRotation.z = wobbleZ;
            modelTransform.setRotation(modelRotation);
            
            glm::vec3 modelPos = modelTransform.getPosition();
            modelPos.y = modelBaseY + sin(wobbleTime * 2.0f) * wobbleAmount * 0.05f * wobbleIntensity;
            modelTransform.setPosition(modelPos);
        } 
        else {
            glm::vec3 modelRotation = modelTransform.getRotation();
            modelRotation.x = glm::mix(modelRotation.x, 0.0f, deltaTime * 5.0f);
            modelRotation.z = glm::mix(modelRotation.z, 0.0f, deltaTime * 5.0f);
            modelTransform.setRotation(modelRotation);
            
            glm::vec3 modelPos = modelTransform.getPosition();
            modelPos.y = glm::mix(modelPos.y, modelBaseY, deltaTime * 5.0f);
            modelTransform.setPosition(modelPos);
        }
    }

    void applyDamping() {
        glm::vec3 velocity = rigidBody->getLinearVelocity();
        velocity.x *= dampingFactor;
        velocity.z *= dampingFactor;
        rigidBody->setLinearVelocity(velocity);
    }

private:
    Entity* playerEntity;
    Entity* modelEntity;
    
    float moveSpeed;
    float rotationSpeed;
    float maxVelocity;
    float dampingFactor;
    float forceMagnitude;
    
    float coldSpeedMultiplier;
    
    bool isSprinting;
    float sprintTimer;
    float sprintCooldownTimer;
    float sprintDuration;
    float sprintCooldown;
    float sprintSpeedMultiplier;
    float sprintAcceleration;
    float currentSprintMultiplier;
    
    float wobbleAmount;
    float wobbleSpeed;
    float wobbleTime;
    bool isMoving;
    
    float currentRotation;
    float targetRotation;
    
    float modelBaseY;
    bool modelBaseYInitialized;
    
    glm::vec3 moveInput;
    glm::vec3 lastMoveDirection;
    RigidBody* rigidBody;
    
    AnimationState currentAnimationState;
    Model* playerModel;
    bool isAttacking;
    bool isInteracting;
    float interactAnimationTimer;
    float interactAnimationDuration;
};

} // namespace GLR