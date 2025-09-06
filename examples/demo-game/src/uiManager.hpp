#pragma once

#include "GLare.hpp"
#include "player.hpp"
#include "managers.hpp"

#include <memory>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

enum class GameState {
    START_SCREEN,
    PLAYING,
    GAME_OVER,
    GAME_WON
};

struct GameSettings {
    int treeSeed = 42;
    int vsync = 1;
    int maxTrees = 100;
    float minTreeDistance = 3.5f;
    float dayDuration = 60.0f;
    float nightDuration = 30.0f;
};

struct DayNightCycle {
    float currentTime = 0.0f;
    float dayDuration = 60.0f;
    float nightDuration = 30.0f;
    float cycleDuration = 90.0f;
    bool isNight = false;
    float nightIntensity = 0.0f;
    float transitionDuration = 5.0f;
    
    glm::vec3 dayLightColor = glm::vec3(1.0f, 0.95f, 0.8f);
    glm::vec3 nightLightColor = glm::vec3(0.05f, 0.08f, 0.15f);
    float dayLightIntensity = 1.0f;
    float nightLightIntensity = 0.02f;
    
    float baseCampfireRadius = 8.0f;
    float nightCampfireRadius = 6.0f;
    float baseCampfireIntensity = 1.0f;
    float nightCampfireIntensity = 2.0f;
};

struct ColdSystem {
    float currentColdness = 0.0f;
    float maxColdness = 100.0f;
    float coldIncreaseRate = 8.0f;
    float warmUpRate = 15.0f;
    float nightColdMultiplier = 1.5f;
    
    float coldnessSlowdownThreshold = 0.3f;
    float maxMovementPenalty = 0.8f;
    float vignetteStartThreshold = 0.2f;
    float saturationStartThreshold = 0.2f;
    float maxVignetteIntensity = 1.0f;
    float minSaturation = -0.5f;
    float warmthCheckRadius = 1.0f;
    
    bool isFrozen = false;
    
    float getNormalizedColdness() const {
        return std::clamp(currentColdness / maxColdness, 0.0f, 1.0f);
    }
    
    float getMovementSpeedMultiplier() const {
        float normalizedColdness = getNormalizedColdness();
        if (normalizedColdness < coldnessSlowdownThreshold) {
            return 1.0f;
        }
        
        float penaltyRange = normalizedColdness - coldnessSlowdownThreshold;
        float maxRange = 1.0f - coldnessSlowdownThreshold;
        float penaltyFactor = penaltyRange / maxRange;
        
        return 1.0f - (penaltyFactor * maxMovementPenalty);
    }
    
    float getVignetteIntensity() const {
        float normalizedColdness = getNormalizedColdness();
        if (normalizedColdness < vignetteStartThreshold) {
            return 0.0f;
        }
        
        float vignetteRange = normalizedColdness - vignetteStartThreshold;
        float maxRange = 1.0f - vignetteStartThreshold;
        float vignetteFactor = vignetteRange / maxRange;
        
        return vignetteFactor * maxVignetteIntensity;
    }
    
    float getSaturation() const {
        float normalizedColdness = getNormalizedColdness();
        if (normalizedColdness < saturationStartThreshold) {
            return 1.0f;
        }
        
        float saturationRange = normalizedColdness - saturationStartThreshold;
        float maxRange = 1.0f - saturationStartThreshold;
        float saturationFactor = saturationRange / maxRange;
        
        return 1.0f - (saturationFactor * (1.0f - minSaturation));
    }
};

class UiManager {
public:
    UiManager() = default;
    ~UiManager() = default;

    bool initialize(GLFWwindow* window) {
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

        return true;
    }

    void shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void beginFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void endFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void drawStartScreen(int windowWidth, int windowHeight, GameSettings& settings, GLFWwindow* window) {
        // Make the ImGui window fill the entire GLFW window
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoResize | 
                                        ImGuiWindowFlags_NoMove | 
                                        ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoScrollWithMouse |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus |
                                        ImGuiWindowFlags_NoNavFocus;
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.14f, 0.18f, 1.0f));
        
        ImGui::Begin("##StartScreen", nullptr, windowFlags);
        
        float contentWidth = 350.0f;
        float contentHeight = 450.0f;
        float startX = (windowWidth - contentWidth) * 0.5f;
        float startY = (windowHeight - contentHeight) * 0.5f;
        
        ImGui::SetCursorPos(ImVec2(startX, startY));
        
        ImGui::BeginChild("ContentArea", ImVec2(contentWidth, contentHeight), true, ImGuiWindowFlags_NoScrollbar);
        
        ImGui::SetCursorPosX((contentWidth - ImGui::CalcTextSize("SURVIVAL GAME").x) * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "SURVIVAL GAME");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Game Settings
        ImGui::Text("Game Settings");
        ImGui::Spacing();
        
        ImGui::Text("Tree Generation Seed:");
        ImGui::SliderInt("##Seed", &settings.treeSeed, 1, 1000);
        
        ImGui::Spacing();
        
        ImGui::Text("Max Trees:");
        ImGui::SliderInt("##MaxTrees", &settings.maxTrees, 50, 200);
        
        ImGui::Spacing();
        
        ImGui::Text("Tree Spacing:");
        ImGui::SliderFloat("##TreeSpacing", &settings.minTreeDistance, 2.0f, 8.0f, "%.1f");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Day/Night Settings
        ImGui::Text("Day/Night Cycle");
        ImGui::Spacing();
        
        ImGui::Text("Day Duration (seconds):");
        ImGui::SliderFloat("##DayDuration", &settings.dayDuration, 30.0f, 180.0f, "%.0f");
        
        ImGui::Spacing();
        
        ImGui::Text("Night Duration (seconds):");
        ImGui::SliderFloat("##NightDuration", &settings.nightDuration, 15.0f, 90.0f, "%.0f");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Graphics Settings
        ImGui::Text("Graphics Settings");
        ImGui::Spacing();
        
        bool vsyncEnabled = settings.vsync == 1;
        if (ImGui::Checkbox("Enable VSync", &vsyncEnabled)) {
            settings.vsync = vsyncEnabled ? 1 : 0;
            glfwSwapInterval(settings.vsync);
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Play Button
        float buttonWidth = 200.0f;
        ImGui::SetCursorPosX((contentWidth - buttonWidth) * 0.5f);
        
        startGamePressed = false;
        if (ImGui::Button("START GAME", ImVec2(buttonWidth, 40))) {
            startGamePressed = true;
        }
        
        ImGui::Spacing();
        
        ImGui::SetCursorPosX((contentWidth - buttonWidth) * 0.5f);
        if (ImGui::Button("QUIT", ImVec2(buttonWidth, 30))) {
            glfwSetWindowShouldClose(window, true);
        }
        
        // Instructions
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Text("Controls:");
        ImGui::BulletText("WASD - Move");
        ImGui::BulletText("Shift - Sprint");
        ImGui::BulletText("E - Attack/Chop trees");
        ImGui::BulletText("F - Interact");
        ImGui::BulletText("ESC - Quit");
        
        ImGui::EndChild();
        ImGui::End();
        
        ImGui::PopStyleColor();
    }

    void drawGameHUD(int windowWidth, int windowHeight, 
                     std::shared_ptr<GLR::Scene> scene,
                     GLR::PlayerController* playerController,
                     GLR::CampfireManager* campfireManager,
                     GLR::LogCabinManager* logCabinManager,
                     const DayNightCycle& dayNightCycle,
                     const ColdSystem& coldSystem,
                     bool& gameWon,
                     GameState& currentGameState) {
        
        ImGui::SetNextWindowPos(ImVec2(10, windowHeight - 170), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(250, 160), ImGuiCond_Always);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | 
                                        ImGuiWindowFlags_NoResize | 
                                        ImGuiWindowFlags_NoMove | 
                                        ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoScrollbar;
        
        ImGui::Begin("GameStats", nullptr, windowFlags);
        
        // Get player inventory
        int woodCount = 0;
        auto playerEntity = scene->findEntityByName("PlayerCollider");
        if (playerEntity && playerEntity->hasComponent<GLR::PlayerInventory>()) {
            auto& inventory = playerEntity->getComponent<GLR::PlayerInventory>();
            woodCount = inventory.getLogs();
        }
        
        ImGui::Text("Wood: %d", woodCount);
        
        // Get campfire fuel
        float fuelPercentage = 0.0f;
        if (campfireManager) {
            fuelPercentage = (campfireManager->getCurrentFuel() / campfireManager->getMaxFuel()) * 100.0f;
        }
        
        if (fuelPercentage > 50.0f) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Campfire: %.1f%%", fuelPercentage);
        } 
        else if (fuelPercentage > 25.0f) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Campfire: %.1f%%", fuelPercentage);
        } 
        else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Campfire: %.1f%%", fuelPercentage);
        }
        
        // Sprint system display
        if (playerController) {
            ImGui::Text("Sprint:");
            
            if (playerController->isPlayerSprinting()) {
                float sprintTimeLeft = playerController->getSprintTimeRemaining();
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "  SPRINTING! (%.1fs left)", sprintTimeLeft);
            } else if (playerController->canSprint()) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "  Ready!");
            } else {
                float cooldownLeft = playerController->getSprintCooldownRemaining();
                if (cooldownLeft > 0.0f) {
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "  Cooldown: %.1fs", cooldownLeft);
                } else {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  Not moving");
                }
            }
        }
        
        // Cold system display
        float coldPercentage = coldSystem.getNormalizedColdness() * 100.0f;
        bool nearWarmth = isPlayerNearWarmth(scene, coldSystem);
        
        ImGui::Text("Temperature:");
        
        if (coldPercentage < 25.0f) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "  Warmth: %.1f%%", 100.0f - coldPercentage);
            if (nearWarmth) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), " [Near Fire]");
            }
        } 
        else if (coldPercentage < 50.0f) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "  Cool: %.1f%%", coldPercentage);
        } 
        else if (coldPercentage < 75.0f) {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "  Cold: %.1f%%", coldPercentage);
        }
        else if (coldPercentage < 90.0f) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "  Very Cold: %.1f%%", coldPercentage);
        }
        else {
            ImGui::TextColored(ImVec4(0.8f, 0.0f, 1.0f, 1.0f), "  FREEZING: %.1f%%", coldPercentage);
        }
        
        // Cabin progress display
        if (logCabinManager) {
            float cabinProgress = logCabinManager->getCurrentProgress();
            int logsNeeded = logCabinManager->getLogsNeeded();
            
            if (logCabinManager->getCompletionStatus()) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Cabin: COMPLETED!");
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "YOU WON!");
                if (!gameWon) {
                    gameWon = true;
                    currentGameState = GameState::GAME_WON;
                }
            } 
            else {
                if (cabinProgress >= 75.0f) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Cabin: %.1f%% (%d logs left)", cabinProgress, logsNeeded);
                } 
                else if (cabinProgress >= 50.0f) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Cabin: %.1f%% (%d logs left)", cabinProgress, logsNeeded);
                } 
                else {
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "Cabin: %.1f%% (%d logs left)", cabinProgress, logsNeeded);
                }
            }
        }
        
        // Day-Night cycle info
        ImGui::Separator();
        const char* timeOfDay = dayNightCycle.isNight ? "Night" : "Day";
        float timeRemaining = dayNightCycle.isNight ? (dayNightCycle.nightDuration - (dayNightCycle.currentTime - dayNightCycle.dayDuration)) : (dayNightCycle.dayDuration - dayNightCycle.currentTime);
        
        ImGui::Text("Time: %s (%.1fs)", timeOfDay, timeRemaining);
        
        ImGui::End();
    }
    
    void drawGameOverScreen(int windowWidth, int windowHeight, const ColdSystem& coldSystem, GLFWwindow* window, GameState& currentGameState) {
        
        ImGui::SetNextWindowPos(ImVec2(windowWidth * 0.5f, windowHeight * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_Always);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | 
                                        ImGuiWindowFlags_NoMove | 
                                        ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoScrollbar;
        
        ImGui::Begin("Game Over", nullptr, windowFlags);
        
        if (coldSystem.isFrozen) {
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("You froze to death!").x) * 0.5f);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "You froze to death!");
            
            ImGui::Spacing();
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Stay near the campfire to keep warm.").x) * 0.5f);
            ImGui::Text("Stay near the campfire to keep warm.");
        } 
        else {
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("The campfire went out!").x) * 0.5f);
            ImGui::Text("The campfire went out!");
            
            ImGui::Spacing();
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("You survived as long as you could.").x) * 0.5f);
            ImGui::Text("You survived as long as you could.");
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        float buttonWidth = 100.0f;
        float spacing = 10.0f;
        float totalWidth = buttonWidth * 2 + spacing;
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - totalWidth) * 0.5f);
        
        restartPressed = false;
        if (ImGui::Button("Restart", ImVec2(buttonWidth, 30))) {
            restartPressed = true;
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Quit", ImVec2(buttonWidth, 30))) {
            glfwSetWindowShouldClose(window, true);
        }
        
        ImGui::End();
    }
    
    void drawWinScreen(int windowWidth, int windowHeight, GLFWwindow* window, GameState& currentGameState) {
        
        ImGui::SetNextWindowPos(ImVec2(windowWidth * 0.5f, windowHeight * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(350, 220), ImGuiCond_Always);
        
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | 
                                        ImGuiWindowFlags_NoMove | 
                                        ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoScrollbar;
        
        ImGui::Begin("Victory!", nullptr, windowFlags);
        
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("CONGRATULATIONS!").x) * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "CONGRATULATIONS!");
        
        ImGui::Spacing();
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("You built the cabin!").x) * 0.5f);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "You built the cabin!");
        
        ImGui::Spacing();
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("You survived the harsh winter").x) * 0.5f);
        ImGui::Text("You survived the harsh winter");
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("and completed your shelter.").x) * 0.5f);
        ImGui::Text("and completed your shelter.");
        
        ImGui::Spacing();
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("You are now safe from the cold!").x) * 0.5f);
        ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f), "You are now safe from the cold!");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        float buttonWidth = 100.0f;
        float spacing = 10.0f;
        float totalWidth = buttonWidth * 2 + spacing;
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - totalWidth) * 0.5f);
        
        playAgainPressed = false;
        if (ImGui::Button("Play Again", ImVec2(buttonWidth, 30))) {
            playAgainPressed = true;
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Quit", ImVec2(buttonWidth, 30))) {
            glfwSetWindowShouldClose(window, true);
        }
        
        ImGui::End();
    }

    bool isPlayerNearWarmth(std::shared_ptr<GLR::Scene> scene, const ColdSystem& coldSystem) {
        auto playerEntity = scene->findEntityByName("PlayerCollider");
        if (!playerEntity) return false;
        
        auto& playerTransform = playerEntity->getComponent<GLR::Transform>();
        glm::vec3 playerPos = playerTransform.getWorldPosition();
        
        auto pitFireLight = scene->findEntityByName("pitFireLight");
        if (!pitFireLight || !pitFireLight->hasComponent<GLR::PointLight>()) return false;
        
        auto& pitFireLightTransform = pitFireLight->getComponent<GLR::Transform>();
        auto& pointLight = pitFireLight->getComponent<GLR::PointLight>();
        
        glm::vec3 firePos = pitFireLightTransform.getWorldPosition();
        float fireRadius = pointLight.getRadius() * coldSystem.warmthCheckRadius;
        
        float distance = glm::length(playerPos - firePos);
        return distance <= fireRadius;
    }

    // Public flags to check button presses
    bool startGamePressed = false;
    bool restartPressed = false;
    bool playAgainPressed = false;
};