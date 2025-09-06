#pragma once

// std
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <chrono>
#include <set>
#include <algorithm>
#include <utility>
#include <filesystem>
#include <unordered_map>

// openGL
#include <glad/glad.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/ext.hpp>
// #include <glm/gtx/string_cast.hpp>
// #include <glm/gtc/quaternion.hpp>
// #include <glm/gtx/euler_angles.hpp>
// #include <glm/ext.hpp>

// Physics Library
#include <reactphysics3d/reactphysics3d.h>

// Image loading
#include <stb_image.h>

// Model loading
#include <tinygltf.h>

namespace GLR {

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_CYAN    "\033[36m"

inline GLenum checkOpenGLError(const char* file, int line) {
	// Only print errors if user has defined DEBUG
	#ifdef DEBUG
	GLenum err;
	bool hasError = false;
	
	while ((err = glGetError()) != GL_NO_ERROR) {
		hasError = true;
		std::string error;
		switch (err) {
			case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
			case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
			case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
			case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			default: error = "UNKNOWN";
		}
		
		std::cerr << "OpenGL Error at " << file << ":" << line << ": " << error << " (0x" << std::hex << err << ")" << std::endl;
	}
	
	if (hasError) {
		GLint currentProgram, currentVAO, currentVBO, currentEBO;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentVBO);
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentEBO);
		
		std::cerr << "Current OpenGL State:" << std::endl;
		std::cerr << "  Program: " << currentProgram << std::endl;
		std::cerr << "  VAO: " << currentVAO << std::endl;
		std::cerr << "  VBO: " << currentVBO << std::endl;
		std::cerr << "  EBO: " << currentEBO << std::endl;
		
		if (currentProgram == 0) {
			std::cerr << "  ERROR: No shader program is bound!" << std::endl;
		}
		
		if (currentVAO == 0) {
			std::cerr << "  ERROR: No VAO is bound!" << std::endl;
		}
	}
	return err;
	#endif
	return GL_NO_ERROR;
}

inline void printError(const char* file, int line, const std::string& message) {
	std::cerr << COLOR_RED << "ERROR: " << message << " (" << file << ":" << line << ")" << COLOR_RESET << std::endl;
}

inline void printInfo(const char* file, int line, const std::string& message) {
	std::cerr << COLOR_CYAN << "Info " << message << " (" << file << ":" << line << ")" << COLOR_RESET << std::endl;
}

#define GL_CHECK() checkOpenGLError(__FILE__, __LINE__)
#define ERROR(x)   printError(__FILE__, __LINE__, (x))
#define INFO(x)    printInfo(__FILE__, __LINE__, (x))

class Color {
public:
	float r, g, b, a;

	Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}

	Color(float red, float green, float blue, float alpha = 1.0f) : r(red), g(green), b(blue), a(alpha) {}

	static Color White()        { return Color(1.0f, 1.0f, 1.0f); }
	static Color Black()        { return Color(0.0f, 0.0f, 0.0f); }
	static Color Grey()         { return Color(0.5f, 0.5f, 0.5f); }
	static Color Red()          { return Color(1.0f, 0.0f, 0.0f); }
	static Color Green()        { return Color(0.0f, 1.0f, 0.0f); }
	static Color Blue()         { return Color(0.0f, 0.0f, 1.0f); }
	static Color Yellow()       { return Color(1.0f, 1.0f, 0.0f); }
	static Color Cyan()         { return Color(0.0f, 1.0f, 1.0f); }
	static Color Magenta()      { return Color(1.0f, 0.0f, 1.0f); }
	static Color Transparent()  { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
	static Color LightBlue()    { return Color(0.678f, 0.847f, 0.902f); }
	static Color DarkBlue()     { return Color(0.0f, 0.0f, 0.545f); }    
	static Color Coral()        { return Color(1.0f, 0.498f, 0.314f); }  
	static Color Orange()       { return Color(1.0f, 0.647f, 0.0f); }    
	static Color Pink()         { return Color(1.0f, 0.753f, 0.796f); }  
	static Color Brown()        { return Color(0.545f, 0.271f, 0.075f); }
	static Color Purple()       { return Color(0.502f, 0.0f, 0.502f); }  
	static Color Teal()         { return Color(0.0f, 0.502f, 0.502f); }  
	static Color Olive()        { return Color(0.502f, 0.502f, 0.0f); }  
	static Color Navy()         { return Color(0.0f, 0.0f, 0.502f); }    
	static Color Mint()         { return Color(0.596f, 1.0f, 0.596f); }  
	static Color Gold()         { return Color(1.0f, 0.843f, 0.0f); }    
	static Color Beige()        { return Color(0.961f, 0.961f, 0.863f); }
	static Color Maroon()       { return Color(0.502f, 0.0f, 0.0f); }    
	static Color Indigo()       { return Color(0.294f, 0.0f, 0.510f); }  

	static glm::vec3 toVec3(Color c) { return glm::vec3(c.r, c.g, c.b); }
};

class TimeStep {
public:
	TimeStep() : lastFrame(0.0f), currentFrame(0.0f), deltaTime(0.0f) {}

	void updateTimeStep(float lastFrameTime, float currentFrameTime) {
		lastFrame = lastFrameTime;
		currentFrame = currentFrameTime;
		deltaTime = currentFrame - lastFrame;
	}

	float getDeltaTime() const { return deltaTime; }
	float getMilliseconds() const { return deltaTime * 1000.0f; }

private:
	float lastFrame, currentFrame;
	float deltaTime;
};

class Shader {
public:
	Shader() : ID(0) {}
	
	Shader(const std::string& vertexPath, const std::string& fragmentPath) : ID(0), vertexFilePath(vertexPath), fragmentFilePath(fragmentPath) {
		compile(returnFileContents(vertexPath), returnFileContents(fragmentPath));
	}
	
	Shader(const std::string& vertexSource, const std::string& fragmentSource, bool isPlainText) : ID(0) {
		if (isPlainText) {
			compile(vertexSource, fragmentSource);
		}
	}
	
	~Shader() { destroy(); }
	
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	
	Shader(Shader&& other) noexcept : ID(other.ID) {
		other.ID = 0;
	}
	
	Shader& operator=(Shader&& other) noexcept {
		if (this != &other) {
			destroy();
			ID = other.ID;
			other.ID = 0;
		}
		return *this;
	}

	void setBool(const std::string& name, bool value) {
		glUniform1i(getUniformLocation(name), (int)value);
		GL_CHECK();
	}
	
	void setInt(const std::string& name, int value) {
		glUniform1i(getUniformLocation(name), value);
		GL_CHECK();
	}
	
	void setFloat(const std::string& name, float value) {
		glUniform1f(getUniformLocation(name), value);
		GL_CHECK();
	}
	
	void setVector2Float(const std::string& name, const glm::vec2& vec2) {
		glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(vec2));
		GL_CHECK();
	}
	
	void setVector3Float(const std::string& name, const glm::vec3& vec3) {
		glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(vec3));
		GL_CHECK();
	}
	
	void setVector4Float(const std::string& name, const glm::vec4& vec4) {
		glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(vec4));
		GL_CHECK();
	}
	
	void setMatrix4Float(const std::string& name, const glm::mat4& mat4) {
		glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat4));
		GL_CHECK();
	}

	void bind() { 
		glUseProgram(ID); 
		GL_CHECK();
	}
	
	void unbind() { 
		glUseProgram(0); 
		GL_CHECK();
	}
	
	void destroy() { 
		if (ID) {
			glDeleteProgram(ID); 
			ID = 0;
			GL_CHECK();
		}
	}

	GLuint getID() const { return ID; }
	bool isValid() const { return ID != 0; }
	std::string getVertexFilePath() { return vertexFilePath; }
	std::string getFragmentFilePath() { return fragmentFilePath; }

private:
	GLuint ID;
	std::string vertexFilePath; 
	std::string fragmentFilePath;

	std::string returnFileContents(const std::string& filePath) {
		std::string contents;
		std::ifstream file(filePath, std::ios::in);

		if (!file.is_open()) {
			ERROR("Failed reading (" + filePath + " ) Maybe wrong file name?");
			return contents;
		}

		std::string line = "";
		while (!file.eof()) {
			std::getline(file, line);
			contents.append(line + "\n");
		}

		file.close();
		return contents;
	}
	
	void compile(const std::string& vertexContents, const std::string& fragmentContents) {
		const char* vertexSource = vertexContents.c_str();
		const char* fragmentSource = fragmentContents.c_str();

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		GL_CHECK();

		// compiling vertex shader
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);
		GL_CHECK();

		// compiling fragment shader
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);
		GL_CHECK();

		// checking both shaders for errors
		compileErrorChecking(vertexShader, "Vertex Shader");
		compileErrorChecking(fragmentShader, "Fragment Shader");

		// linking shaders
		ID = glCreateProgram();
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		glLinkProgram(ID);
		GL_CHECK();
		
		// Check for linking errors
		GLint success;
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			ERROR("ERROR::SHADER::PROGRAM::LINKING_FAILED\n" + std::string(infoLog));
			glDeleteProgram(ID);
			ID = 0;
		}

		// deleting shaders
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		GL_CHECK();
	}
	
	void compileErrorChecking(const GLuint& shaderID, const std::string& shaderType) {
		GLint compileStatus;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);

		// If there is an error
		if (compileStatus != GL_TRUE) {
			GLint infoLogLength;
			glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
			
			std::vector<GLchar> errorLog(infoLogLength);
			glGetShaderInfoLog(shaderID, infoLogLength, &infoLogLength, &errorLog[0]);
			
			ERROR("ERROR::SHADER::" + shaderType + "::COMPILATION_FAILED\n" + std::string(errorLog.begin(), errorLog.end()));

			// Exiting engine if there is a shader error
			exit(1);
		}
	}
	
	GLint getUniformLocation(const std::string& name) {
		GLint location = glGetUniformLocation(ID, name.c_str());
		if (location == -1) {
			ERROR("Uniform '" + name + "' not found in shader program with ID: " + std::to_string(ID));
		}
		return location;
	}
};

class ShaderLibrary {
public:
	struct ShaderSource {
		std::string vertex;
		std::string fragment;
	};

	static void addShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource) {
		shaders[name] = {vertexSource, fragmentSource};
	}

	static const ShaderSource& getShader(const std::string& name) {
		ensureInitialized();
		auto it = shaders.find(name);
		if (it != shaders.end()) {
			return it->second;
		}
		
		ERROR("Shader '" + name + "' not found in shader library");
		static ShaderSource empty = {"", ""};
		return empty;
	}
	
	static bool hasShader(const std::string& name) {
		ensureInitialized();
		return shaders.find(name) != shaders.end();
	}
	
	static std::vector<std::string> getShaderNames() {
		ensureInitialized();
		std::vector<std::string> names;
		for (const auto& pair : shaders) {
			names.push_back(pair.first);
		}
		return names;
	}

private:
	static void ensureInitialized() {
		if (!initialized) {
			initialize();
			initialized = true;
		}
	}

	static void initialize() {
		addShader("debug", 
			R"(#version 410 core
			layout (location = 0) in vec3 a_Position;
			layout (location = 1) in vec3 a_Color;
			uniform mat4 viewProjection;
			out vec3 v_Color;
			void main() {
				v_Color = a_Color;
				gl_Position = viewProjection * vec4(a_Position, 1.0);
			})",
			R"(#version 410 core
			in vec3 v_Color;
			out vec4 FragColor;
			void main() {
				FragColor = vec4(v_Color, 1.0);
			})"
		);

		addShader("debug_point",
			R"(#version 410 core
			layout (location = 0) in vec3 a_Position;
			layout (location = 1) in vec3 a_Color;
			uniform mat4 viewProjection;
			out vec3 v_Color;
			void main() {
				v_Color = a_Color;
				gl_Position = viewProjection * vec4(a_Position, 1.0);
			})",
			R"(#version 410 core
			in vec3 v_Color;
			out vec4 FragColor;
			void main() {
				vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
				if (dot(circCoord, circCoord) > 1.0) {
					discard;
				}
				
				FragColor = vec4(v_Color, 1.0);
			})"
		);

		addShader("point_shadow",
			R"(#version 410 core
			layout (location = 0) in vec3 aPos;
			layout (location = 1) in vec3 aNormal;
			layout (location = 2) in vec2 aTexCoord;
			layout (location = 3) in vec3 aTangent;
			layout (location = 4) in vec3 aBitangent;
			layout (location = 5) in vec4 aJointIndices;
			layout (location = 6) in vec4 aJointWeights;
			out vec4 FragPos;
			uniform mat4 lightSpaceMatrix;
			uniform mat4 model;
			uniform int isSkinned;
			#define MAX_JOINTS 100
			uniform mat4 jointMatrices[MAX_JOINTS];
			void main()
			{
				vec4 finalPosition;
				
				if (isSkinned == 1) {
					finalPosition = vec4(0.0);
					
					for (int i = 0; i < 4; i++) {
						float weight = aJointWeights[i];
						if (weight > 0.0) {
							int jointIndex = int(aJointIndices[i]);
							mat4 jointMatrix = jointMatrices[jointIndex];
							
							finalPosition += weight * (jointMatrix * vec4(aPos, 1.0));
						}
					}
				}
				else {
					finalPosition = vec4(aPos, 1.0);
				}
				
				FragPos = model * finalPosition;
				
				gl_Position = lightSpaceMatrix * FragPos;
			})",
			R"(#version 410 core
			in vec4 FragPos;
			uniform vec3 lightPos;
			uniform float farPlane;
			out vec4 FragColor;
			void main()
			{
				float lightDistance = length(FragPos.xyz - lightPos);
				
				lightDistance = lightDistance / farPlane;
				
				gl_FragDepth = lightDistance;
				FragColor = vec4(lightDistance, lightDistance, lightDistance, 1.0);
			})"
		);

		addShader("skybox",
			R"(#version 410 core
			layout (location = 0) in vec2 aPos;
			out vec3 WorldPos;
			uniform mat4 invViewProjection;
			void main()
			{
				gl_Position = vec4(aPos, 1.0, 1.0);
				
				vec4 worldPos = invViewProjection * vec4(aPos, 1.0, 1.0);
				WorldPos = worldPos.xyz;
			})",
			R"(#version 410 core
			out vec4 FragColor;
			in vec3 WorldPos;
			uniform samplerCube skybox;
			void main()
			{
				FragColor = texture(skybox, normalize(WorldPos));
			})"
		);

		addShader("postprocess",
		    R"(#version 410 core
		    out vec2 TexCoord;
		    void main() {
		        float x = (gl_VertexID & 1) * 2.0 - 1.0;
		        float y = (gl_VertexID & 2) - 1.0;
		        
		        gl_Position = vec4(x, y, 0.0, 1.0);
		        TexCoord = vec2((x + 1.0) * 0.5, (y + 1.0) * 0.5);
		    })",
		    R"(#version 410 core
		    in vec2 TexCoord;
		    out vec4 FragColor;
		    uniform sampler2D screenTexture;
		    uniform sampler2D bloomTexture;
		    uniform vec2 resolution;
		    uniform float gamma = 2.2;
		    uniform float exposure = 1.0;
		    uniform int enableBloom = 0;
		    uniform float bloomIntensity = 1.0;
		    uniform float saturation = 1.0;
		    uniform float contrast = 1.0;
		    uniform float brightness = 0.0;
		    uniform float vibrancy = 0.0;
		    uniform float colorBoost = 1.0;
		    
		    // Vignette uniforms
		    uniform float vignetteIntensity = 0.0;
		    uniform vec3 vignetteColor = vec3(0.0, 0.0, 0.0);
		    
		    vec3 rgb2hsv(vec3 c) {
		        vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
		        vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
		        vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
		        
		        float d = q.x - min(q.w, q.y);
		        float e = 1.0e-10;
		        
		        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
		    }
		    
		    vec3 hsv2rgb(vec3 c) {
		        vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
		        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
		        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
		    }
		    
		    void main() {
		        vec3 color = texture(screenTexture, TexCoord).rgb;
		        
		        if (enableBloom == 1) {
		            vec3 bloom = texture(bloomTexture, TexCoord).rgb;
		            color += bloom * bloomIntensity;
		        }
		        
		        color *= exposure;
		        
		        color *= colorBoost;
		        
		        if (vibrancy != 0.0) {
		            vec3 hsv = rgb2hsv(color);
		            float satBoost = 1.0 + vibrancy * (1.0 - hsv.y);
		            hsv.y = clamp(hsv.y * satBoost, 0.0, 1.0);
		            color = hsv2rgb(hsv);
		        }
		        
		        vec3 gray = vec3(dot(color, vec3(0.299, 0.587, 0.114)));
		        color = mix(gray, color, saturation);
		        
		        color = (color - 0.5) * contrast + 0.5 + brightness;
		        
		        color = color / (1.0 + color * 0.3);
		        
		        color = pow(color, vec3(1.0 / gamma));
		        
		        // Apply vignette effect
		        if (vignetteIntensity > 0.0) {
		            vec2 uv = TexCoord - 0.5;
		            float dist = length(uv);
		            float vignette = smoothstep(0.0, 1.0, dist * vignetteIntensity);
		            color = mix(color, vignetteColor, vignette);
		        }
		        
		        color = clamp(color, 0.0, 1.0);
		        
		        FragColor = vec4(color, 1.0);
		    })"
		);

		addShader("shadow",
			R"(#version 410 core
			layout (location = 0) in vec3 aPos;
			layout (location = 1) in vec3 aNormal;
			layout (location = 2) in vec2 aTexCoord;
			layout (location = 3) in vec3 aTangent;
			layout (location = 4) in vec3 aBitangent;
			layout (location = 5) in vec4 aJointIndices;
			layout (location = 6) in vec4 aJointWeights;
			uniform mat4 lightSpaceMatrix;
			uniform mat4 model;
			uniform int isSkinned;
			#define MAX_JOINTS 100
			uniform mat4 jointMatrices[MAX_JOINTS];
			void main() {
				vec4 finalPosition;
				
				if (isSkinned == 1) {
					finalPosition = vec4(0.0);
					
					for (int i = 0; i < 4; i++) {
						float weight = aJointWeights[i];
						if (weight > 0.0) {
							int jointIndex = int(aJointIndices[i]);
							mat4 jointMatrix = jointMatrices[jointIndex];
							
							finalPosition += weight * (jointMatrix * vec4(aPos, 1.0));
						}
					}
				}
				else {
					finalPosition = vec4(aPos, 1.0);
				}
				vec4 worldPosition = model * finalPosition;
				gl_Position = lightSpaceMatrix * worldPosition;
			})",
			R"(#version 410 core
			void main() { })"
		);

		addShader("main",
			R"(#version 410 core
			layout (location = 0) in vec3 aPos;
			layout (location = 1) in vec3 aNormal;
			layout (location = 2) in vec2 aTexCoord;
			layout (location = 3) in vec3 aTangent;
			layout (location = 4) in vec3 aBitangent;
			layout (location = 5) in vec4 aJointIndices;
			layout (location = 6) in vec4 aJointWeights;
			uniform mat4 model;
			uniform mat4 view;
			uniform mat4 projection;
			uniform int isSkinned;
			uniform mat4 jointMatrices[100];
			out vec2 TexCoord;
			out vec3 Normal;
			out vec3 FragPos;
			out mat3 TBN;
			void main() {
			    vec4 finalPosition;
			    vec3 finalNormal = aNormal;
			    vec3 finalTangent = aTangent;
			    vec3 finalBitangent = aBitangent;
			    
			    if (isSkinned == 1) {
			        finalPosition = vec4(0.0);
			        finalNormal = vec3(0.0);
			        finalTangent = vec3(0.0);
			        finalBitangent = vec3(0.0);
			        
			        for (int i = 0; i < 4; i++) {
			            float weight = aJointWeights[i];
			            if (weight > 0.0) {
			                int jointIndex = int(aJointIndices[i]);
			                mat4 jointMatrix = jointMatrices[jointIndex];
			                
			                finalPosition += weight * (jointMatrix * vec4(aPos, 1.0));
			                
			                mat3 jointRotation = mat3(jointMatrix);
			                finalNormal += weight * (jointRotation * aNormal);
			                finalTangent += weight * (jointRotation * aTangent);
			                finalBitangent += weight * (jointRotation * aBitangent);
			            }
			        }
			    }
			    else {
			        finalPosition = vec4(aPos, 1.0);
			    }
			    
			    vec4 worldPosition = model * finalPosition;
			    
			    FragPos = worldPosition.xyz;
			    
			    mat3 normalMatrix = transpose(inverse(mat3(model)));
			    vec3 N = normalize(normalMatrix * finalNormal);
			    vec3 T = normalize(normalMatrix * finalTangent);
			    vec3 B = normalize(normalMatrix * finalBitangent);
			    
			    T = normalize(T - dot(T, N) * N);
			    B = cross(N, T);
			    
			    TBN = mat3(T, B, N);
			    
			    Normal = N;
			    
			    TexCoord = aTexCoord;
			    
			    gl_Position = projection * view * worldPosition;
			})",
			R"(#version 410 core
			layout (location = 0) out vec4 FragColor;
			layout (location = 1) out vec4 BloomColor;
			in vec2 TexCoord;
			in vec3 Normal;
			in vec3 FragPos;
			in mat3 TBN;
			uniform mat4 model;
			uniform mat4 view;
			uniform mat4 projection;
			uniform vec3 viewPosition;
			uniform sampler2D baseColorTexture;
			uniform sampler2D normalTexture;
			uniform sampler2D metallicRoughnessTexture;
			uniform sampler2D emissiveTexture;
			uniform bool baseColorTextureBool = false;
			uniform bool normalTextureBool = false;
			uniform bool metallicRoughnessTextureBool = false;
			uniform bool emissiveTextureBool = false;
			uniform vec4 baseColorFactor = vec4(1.0, 1.0, 1.0, 1.0);
			uniform float metallicFactor = 1.0;
			uniform float roughnessFactor = 1.0;
			uniform float normalScale = 1.0;
			uniform vec3 emissiveFactor = vec3(0.0, 0.0, 0.0);
			uniform float baseMetallic = 0.0;
			uniform float baseRoughness = 0.5;
			uniform float bloomThreshold = 1.0;
			uniform float emissiveBloomBoost = 1.0;
			struct DirectionalLight {
			    vec3 direction;
			    vec3 color;
			    float intensity;
			    bool castShadows;
			    mat4 lightSpaceMatrix;
			    float shadowBias;
			};
			struct PointLight {
			    vec3 position;
			    vec3 color;
			    float farPlane;
			    float intensity;
			    float radius;
			    float constant;
			    float linear;
			    float quadratic;
			    bool castShadows;
			    int shadowMapIndex;
			    float shadowBias;
			};
			struct SpotLight {
			    vec3 position;
			    vec3 direction;
			    vec3 color;
			    float intensity;
			    float radius;
			    float innerCutoff;
			    float outerCutoff;
			    float constant;
			    float linear;
			    float quadratic;
			};
			uniform int directionalLightCount;
			uniform int pointLightCount;
			uniform int spotLightCount;
			uniform DirectionalLight directionalLights[2];
			uniform PointLight pointLights[8];
			uniform SpotLight spotLights[8];
			uniform sampler2D directionalLightShadowMaps[2];
			uniform samplerCube pointLightShadowCubemaps[4];
			uniform float ambientStrength = 0.1;
			uniform int debugMode = 0;
			const float PI = 3.14159265359;
			vec3 fresnelSchlick(float cosTheta, vec3 F0) {
			    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
			}
			float DistributionGGX(vec3 N, vec3 H, float roughness) {
			    float a = roughness * roughness;
			    float a2 = a * a;
			    float NdotH = max(dot(N, H), 0.0);
			    float NdotH2 = NdotH * NdotH;
			    
			    float num = a2;
			    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
			    denom = PI * denom * denom;
			    
			    return num / denom;
			}
			float GeometrySchlickGGX(float NdotV, float roughness) {
			    float r = (roughness + 1.0);
			    float k = (r * r) / 8.0;
			    
			    float num = NdotV;
			    float denom = NdotV * (1.0 - k) + k;
			    
			    return num / denom;
			}
			float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
			    float NdotV = max(dot(N, V), 0.0);
			    float NdotL = max(dot(N, L), 0.0);
			    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
			    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
			    
			    return ggx1 * ggx2;
			}
			vec3 calculatePBRLighting(vec3 N, vec3 V, vec3 L, vec3 lightColor, float lightIntensity, 
			                          vec3 albedo, float metallic, float roughness) {
			    vec3 H = normalize(V + L);
			    vec3 radiance = lightColor * lightIntensity;
			    
			    vec3 F0 = vec3(0.04);
			    F0 = mix(F0, albedo, metallic);
			    
			    float NDF = DistributionGGX(N, H, roughness);
			    float G = GeometrySmith(N, V, L, roughness);
			    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
			    
			    vec3 kS = F;
			    vec3 kD = vec3(1.0) - kS;
			    kD *= 1.0 - metallic;
			    
			    vec3 numerator = NDF * G * F;
			    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
			    vec3 specular = numerator / denominator;
			    
			    float NdotL = max(dot(N, L), 0.0);
			    return (kD * albedo / PI + specular) * radiance * NdotL;
			}
			float calculateShadow(vec3 fragPos, DirectionalLight light, sampler2D shadowMap) {
			    vec4 fragPosLightSpace = light.lightSpaceMatrix * vec4(fragPos, 1.0);
			    
			    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
			    projCoords = projCoords * 0.5 + 0.5;
			    
			    if(projCoords.x < 0.0 || projCoords.x > 1.0 || 
			       projCoords.y < 0.0 || projCoords.y > 1.0 || 
			       projCoords.z < 0.0 || projCoords.z > 1.0) {
			        return 0.0;
			    }
			    
			    float closestDepth = texture(shadowMap, projCoords.xy).r;
			    float currentDepth = projCoords.z;
			    float bias = light.shadowBias;
			    
			    float shadow = 0.0;
			    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
			    const int pcfSize = 2;
			    
			    for(int x = -pcfSize; x <= pcfSize; ++x) {
			        for(int y = -pcfSize; y <= pcfSize; ++y) {
			            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
			        }
			    }
			    
			    shadow /= ((pcfSize * 2 + 1) * (pcfSize * 2 + 1));
			    
			    return shadow;
			}
			float calculatePointShadow(vec3 fragPos, PointLight light, samplerCube shadowCubemap) {
			    vec3 fragToLight = fragPos - light.position;
			    float currentDepth = length(fragToLight);
			    
			    if(currentDepth > light.farPlane) {
			        return 0.0;
			    }
			    
			    vec3 sampleDirection = normalize(fragToLight);
			    float closestDepth = texture(shadowCubemap, sampleDirection).r;
			    closestDepth *= light.farPlane;
			    
			    float bias = light.shadowBias;
			    float shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
			    
			    return shadow;
			}
			void main() {
			    vec4 texColor;
			    if (baseColorTextureBool) {
			        texColor = texture(baseColorTexture, TexCoord) * baseColorFactor;
			    } else {
			        texColor = baseColorFactor;
			    }
			    
			    if (texColor.a < 0.01) {
			        discard;
			    }
			    float metallic = baseMetallic;
			    float roughness = baseRoughness;
			    
			    if (metallicRoughnessTextureBool) {
			        vec3 metallicRoughnessValue = texture(metallicRoughnessTexture, TexCoord).rgb;
			        metallic = metallicRoughnessValue.b * metallicFactor;
			        roughness = metallicRoughnessValue.g * roughnessFactor;
			    } else {
			        metallic = baseMetallic * metallicFactor;
			        roughness = baseRoughness * roughnessFactor;
			    }
			    
			    vec3 norm;
			    if (normalTextureBool) {
			        vec3 normalMapValue = texture(normalTexture, TexCoord).rgb;
			        normalMapValue = normalMapValue * 2.0 - 1.0;
			        normalMapValue.xy *= normalScale;
			        norm = normalize(TBN * normalMapValue);
			    } else {
			        norm = normalize(Normal);
			    }
			    
			    vec3 emissive;
			    if (emissiveTextureBool) {
			        emissive = texture(emissiveTexture, TexCoord).rgb * emissiveFactor;
			    } else {
			        emissive = emissiveFactor;
			    }
			    
			    vec3 viewDir = normalize(viewPosition - FragPos);
			    vec3 baseColor = texColor.rgb;
			    
			    vec3 ambient = ambientStrength * baseColor;
			    vec3 result = ambient;
			    
			    for(int i = 0; i < directionalLightCount; i++) {
			        DirectionalLight light = directionalLights[i];
			        vec3 lightDir = normalize(-light.direction);
			        
			        float shadow = 0.0;
			        if(light.castShadows) {
			            shadow = calculateShadow(FragPos, light, directionalLightShadowMaps[i]);
			        }
			        
			        vec3 radiance = calculatePBRLighting(norm, viewDir, lightDir, light.color, 
			                                             light.intensity, baseColor, metallic, roughness);
			        result += radiance * (1.0 - shadow);
			    }
			    for(int i = 0; i < pointLightCount; i++) {
			        PointLight light = pointLights[i];
			        vec3 lightDir = normalize(light.position - FragPos);
			        
			        float distance = length(light.position - FragPos);
			        if(distance > light.radius) continue;
			        
			        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
			        
			        float shadow = 0.0;
			        if(light.castShadows && light.shadowMapIndex >= 0) {
			            shadow = calculatePointShadow(FragPos, light, pointLightShadowCubemaps[light.shadowMapIndex]);
			        }
			        
			        vec3 radiance = calculatePBRLighting(norm, viewDir, lightDir, light.color, 
			                                             light.intensity * attenuation, baseColor, metallic, roughness);
			        result += radiance * (1.0 - shadow);
			    }
			    
			    for(int i = 0; i < spotLightCount; i++) {
			        SpotLight light = spotLights[i];
			        vec3 lightDir = normalize(light.position - FragPos);
			        
			        float distance = length(light.position - FragPos);
			        if(distance > light.radius) continue;
			        
			        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
			        
			        float theta = dot(lightDir, normalize(-light.direction));
			        float epsilon = light.innerCutoff - light.outerCutoff;
			        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
			        
			        vec3 radiance = calculatePBRLighting(norm, viewDir, lightDir, light.color, 
			                                             light.intensity * attenuation * intensity, baseColor, metallic, roughness);
			        result += radiance;
			    }
			    
			    result += emissive;
			    
			    if (debugMode == 1) {
			        FragColor = vec4(baseColor, texColor.a);
			        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
			    } 
			    else if (debugMode == 2) {
			        FragColor = vec4(norm * 0.5 + 0.5, texColor.a);
			        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
			    } 
			    else if (debugMode == 3) {
			        FragColor = vec4(vec3(roughness), texColor.a);
			        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
			    } 
			    else if (debugMode == 4) {
			        FragColor = vec4(vec3(metallic), texColor.a);
			        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
			    } 
			    else if (debugMode == 5) {
			        vec3 lightOnlyResult = ambientStrength * vec3(1.0);
			        
			        for(int i = 0; i < directionalLightCount; i++) {
			            DirectionalLight light = directionalLights[i];
			            vec3 lightDir = normalize(-light.direction);
			            
			            float shadow = 0.0;
			            if(light.castShadows) {
			                shadow = calculateShadow(FragPos, light, directionalLightShadowMaps[i]);
			            }
			            
			            vec3 radiance = calculatePBRLighting(norm, viewDir, lightDir, light.color, 
			                                                 light.intensity, vec3(1.0), 0.0, 0.5);
			            lightOnlyResult += radiance * (1.0 - shadow);
			        }
			        for(int i = 0; i < pointLightCount; i++) {
			            PointLight light = pointLights[i];
			            vec3 lightDir = normalize(light.position - FragPos);
			            
			            float distance = length(light.position - FragPos);
			            if(distance > light.radius) continue;
			            
			            float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
			            
			            float shadow = 0.0;
			            if(light.castShadows && light.shadowMapIndex >= 0) {
			                shadow = calculatePointShadow(FragPos, light, pointLightShadowCubemaps[light.shadowMapIndex]);
			            }
			            
			            vec3 radiance = calculatePBRLighting(norm, viewDir, lightDir, light.color, 
			                                                 light.intensity * attenuation, vec3(1.0), 0.0, 0.5);
			            lightOnlyResult += radiance * (1.0 - shadow);
			        }
			        
			        for(int i = 0; i < spotLightCount; i++) {
			            SpotLight light = spotLights[i];
			            vec3 lightDir = normalize(light.position - FragPos);
			            
			            float distance = length(light.position - FragPos);
			            if(distance > light.radius) continue;
			            
			            float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
			            
			            float theta = dot(lightDir, normalize(-light.direction));
			            float epsilon = light.innerCutoff - light.outerCutoff;
			            float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
			            
			            vec3 radiance = calculatePBRLighting(norm, viewDir, lightDir, light.color, 
			                                                 light.intensity * attenuation * intensity, vec3(1.0), 0.0, 0.5);
			            lightOnlyResult += radiance;
			        }
			        
			        FragColor = vec4(lightOnlyResult, texColor.a);
			        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
			    }
			    else if (debugMode == 6) {
			        float totalShadow = 0.0;
			        int shadowCastingLights = 0;
			        
			        for(int i = 0; i < directionalLightCount; i++) {
			            if(directionalLights[i].castShadows) {
			                totalShadow += calculateShadow(FragPos, directionalLights[i], directionalLightShadowMaps[i]);
			                shadowCastingLights++;
			            }
			        }
			        
			        for(int i = 0; i < pointLightCount; i++) {
			            if(pointLights[i].castShadows && pointLights[i].shadowMapIndex >= 0) {
			                totalShadow += calculatePointShadow(FragPos, pointLights[i], pointLightShadowCubemaps[pointLights[i].shadowMapIndex]);
			                shadowCastingLights++;
			            }
			        }
			        
			        if (shadowCastingLights > 0) {
			            float avgShadow = totalShadow / float(shadowCastingLights);
			            FragColor = vec4(vec3(1.0 - avgShadow), texColor.a);
			        } else {
			            FragColor = vec4(1.0, 1.0, 1.0, texColor.a);
			        }
			        BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
			    }
			    else {
			        FragColor = vec4(result, texColor.a);
			        
			        vec3 bloomContribution = vec3(0.0);
			        
			        bloomContribution += emissive * emissiveBloomBoost;
			        
			        float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
			        if (brightness > bloomThreshold) {
			            bloomContribution += result * (brightness - bloomThreshold);
			        }
			        
			        BloomColor = vec4(bloomContribution, texColor.a);
			    }
			})"
		);
	}

	static std::unordered_map<std::string, ShaderSource> shaders;
	static bool initialized;
};
inline std::unordered_map<std::string, ShaderLibrary::ShaderSource> ShaderLibrary::shaders;
inline bool ShaderLibrary::initialized = false;

class Texture {
public:
	Texture() : id(0), data(nullptr), width(0), height(0), nrChannels(0), internalFormat(0), imageFormat(0), pixelType(0) {}
	
	Texture(const std::string& filename) : id(0), data(nullptr), filename(filename), pixelType(GL_UNSIGNED_BYTE) {
		loadTexture();
		setFormat();
		createOpenGLTexture();

		// freeing memory
		if (data) {
			stbi_image_free(data);
			data = nullptr;
		}
	}

	Texture(int width, int height, GLenum internalFormat, GLenum imageFormat, GLenum pixelType) 
		: id(0), data(nullptr), width(width), height(height), nrChannels(0),
		  internalFormat(internalFormat), imageFormat(imageFormat), pixelType(pixelType) {
		createOpenGLTexture();
	}

	Texture(int width, int height, GLenum format, unsigned char *m_data) 
		: id(0), data(m_data), width(width), height(height), nrChannels(0),
		  internalFormat(format), imageFormat(format), pixelType(GL_UNSIGNED_BYTE) {
		createOpenGLTexture();
	}
	
	~Texture() {
		destroy();
	}
	
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	
	Texture(Texture&& other) noexcept 
		: id(other.id), data(other.data), width(other.width), height(other.height),
		  nrChannels(other.nrChannels), filename(std::move(other.filename)),
		  internalFormat(other.internalFormat), imageFormat(other.imageFormat),
		  pixelType(other.pixelType) {
		other.id = 0;
		other.data = nullptr;
	}
	
	Texture& operator=(Texture&& other) noexcept {
		if (this != &other) {
			destroy();
			
			id = other.id;
			data = other.data;
			width = other.width;
			height = other.height;
			nrChannels = other.nrChannels;
			filename = std::move(other.filename);
			internalFormat = other.internalFormat;
			imageFormat = other.imageFormat;
			pixelType = other.pixelType;
			
			other.id = 0;
			other.data = nullptr;
		}
		return *this;
	}

	void bind(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, id);
		GL_CHECK();
	}
	
	void unbind() {
		glBindTexture(GL_TEXTURE_2D, 0);
		GL_CHECK();
	}
	
	void destroy() {
		if (id) {
			glDeleteTextures(1, &id);
			id = 0;
			GL_CHECK();
		}
		if (data) {
			stbi_image_free(data);
			data = nullptr;
		}
	}

	GLuint getID() const { return id; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	std::string getFilePath() { return filename; }

private:
	std::string filename;
	GLuint id;
	unsigned char *data;
	int width, height, nrChannels;
	GLenum internalFormat;
	GLenum imageFormat;
	GLenum pixelType;

	void loadTexture() {
		stbi_set_flip_vertically_on_load(true); // flip the texture
		data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
		if (!data) {
			ERROR("Failed to load image: " + filename);
			width = height = nrChannels = 0;
		}
	}
	
	void setFormat() {
		if (!data) return;
		
		switch (nrChannels) {
			case 1: internalFormat = imageFormat = GL_RED; break;
			case 2: internalFormat = imageFormat = GL_RG; break;
			case 3: internalFormat = imageFormat = GL_RGB; break;
			case 4: internalFormat = imageFormat = GL_RGBA; break;
			default: 
				ERROR("Unsupported format for image: " + filename);
				return;
		}
	}
	
	void createOpenGLTexture() {
		if ((width <= 0 || height <= 0) && !data) {
			ERROR("Invalid texture dimensions and no texture data");
			return;
		}
		
		glGenTextures(1, &id);
		GL_CHECK();
		
		glBindTexture(GL_TEXTURE_2D, id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, imageFormat, pixelType, data);
		GL_CHECK();

		if (data) {  // Only generate mipmaps for loaded textures, not render targets
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

class CubemapTexture {
public:
	CubemapTexture() : id(0) {}
	
	CubemapTexture(const std::vector<std::string>& faces) : id(0), cubeFaces(faces) {
		loadCubemap(faces);
	}
	
	CubemapTexture(int resolution, GLenum format = GL_DEPTH_COMPONENT, GLenum type = GL_FLOAT) : id(0) {
		createDepthCubemap(resolution, format, type);
	}
	
	~CubemapTexture() {
		destroy();
	}
	
	CubemapTexture(const CubemapTexture&) = delete;
	CubemapTexture& operator=(const CubemapTexture&) = delete;
	CubemapTexture(CubemapTexture&& other) noexcept : id(other.id), cubeFaces(std::move(other.cubeFaces)) {
		other.id = 0;
	}
	
	CubemapTexture& operator=(CubemapTexture&& other) noexcept {
		if (this != &other) {
			destroy();
			id = other.id;
			cubeFaces = std::move(other.cubeFaces);
			other.id = 0;
		}
		return *this;
	}
	
	void bind(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		GL_CHECK();
	}
	
	void unbind() {
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		GL_CHECK();
	}
	
	void destroy() {
		if (id) {
			glDeleteTextures(1, &id);
			id = 0;
			GL_CHECK();
		}
	}
		
	GLuint getID() const { return id; }
	bool isValid() const { return id != 0; }
	std::vector<std::string> getCubeFaces() { return cubeFaces; }

private:
	std::vector<std::string> cubeFaces;
	GLuint id;

	void loadCubemap(const std::vector<std::string>& faces) {
		if (faces.size() != 6) {
			ERROR("Cubemap requires exactly 6 faces");
			return;
		}

		stbi_set_flip_vertically_on_load(false);
		
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		GL_CHECK();
		
		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++) {
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				GLenum format = GL_RGB;
				if (nrChannels == 1) format = GL_RED;
				else if (nrChannels == 3) format = GL_RGB;
				else if (nrChannels == 4) format = GL_RGBA;
				
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				GL_CHECK();
				stbi_image_free(data);
			} 
			else {
				ERROR("Cubemap texture failed to load at path: " + faces[i]);
				stbi_image_free(data);
			}
		}
		
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		if (GL_EXT_texture_filter_anisotropic) {
			float maxAnisotropy = 0.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
			glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
		}

		stbi_set_flip_vertically_on_load(true);
		
		GL_CHECK();
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void createDepthCubemap(int resolution, GLenum format = GL_DEPTH_COMPONENT, GLenum type = GL_FLOAT) {
		if (id) {
			destroy();
		}
		
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		GL_CHECK();
		
		for (unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, resolution, resolution, 0, format, type, NULL);
		}
		
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		GL_CHECK();
	}
};

template <typename T>
class VBO {
public:
	VBO() : ID(0) {}
	
	VBO(const std::vector<T>& vertices) : ID(0) {
		create(vertices);
	}
	
	~VBO() { destroy(); }
	
	// Delete copy constructor and assignment operator
	VBO(const VBO&) = delete;
	VBO& operator=(const VBO&) = delete;
	
	// Move constructor and assignment operator
	VBO(VBO&& other) noexcept : ID(other.ID) {
		other.ID = 0;
	}
	
	VBO& operator=(VBO&& other) noexcept {
		if (this != &other) {
			destroy();
			ID = other.ID;
			other.ID = 0;
		}
		return *this;
	}
	
	void create(const std::vector<T>& vertices) {
		if (ID) destroy();
		
		glGenBuffers(1, &ID);
		GL_CHECK();
		
		glBindBuffer(GL_ARRAY_BUFFER, ID);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		GL_CHECK();
	}

	void bind() { 
		glBindBuffer(GL_ARRAY_BUFFER, ID); 
		GL_CHECK();
	}
	
	void unbind() { 
		glBindBuffer(GL_ARRAY_BUFFER, 0); 
		GL_CHECK();
	}
	
	void destroy() { 
		if (ID) {
			glDeleteBuffers(1, &ID); 
			ID = 0;
			GL_CHECK();
		}
	}
	
	GLuint getID() const { return ID; }
	bool isValid() const { return ID != 0; }

private:
	GLuint ID;
};

class VAO {
public:
	VAO() : ID(0) { 
		glGenVertexArrays(1, &ID); 
		GL_CHECK();
	}
	
	~VAO() { destroy(); }
	
	// Delete copy constructor and assignment operator
	VAO(const VAO&) = delete;
	VAO& operator=(const VAO&) = delete;
	
	// Move constructor and assignment operator
	VAO(VAO&& other) noexcept : ID(other.ID) {
		other.ID = 0;
	}
	
	VAO& operator=(VAO&& other) noexcept {
		if (this != &other) {
			destroy();
			ID = other.ID;
			other.ID = 0;
		}
		return *this;
	}

	void linkAttribute(GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, size_t offset, bool normalized = false) {
		glVertexAttribPointer(layout, numComponents, type, normalized ? GL_TRUE : GL_FALSE, stride, (void*)offset);
		glEnableVertexAttribArray(layout);
		GL_CHECK();
	}

	void bind() { 
		glBindVertexArray(ID); 
		GL_CHECK();
	}
	
	void unbind() { 
		glBindVertexArray(0); 
		GL_CHECK();
	}
	
	void destroy() { 
		if (ID) {
			glDeleteVertexArrays(1, &ID); 
			ID = 0;
			GL_CHECK();
		}
	}
	
	GLuint getID() const { return ID; }

private:
	GLuint ID;
};

class EBO {
public:
	EBO() : ID(0) {}
	
	EBO(const std::vector<GLuint>& indices) : ID(0) {
		create(indices);
	}
	
	~EBO() { destroy(); }
	
	// Delete copy constructor and assignment operator
	EBO(const EBO&) = delete;
	EBO& operator=(const EBO&) = delete;
	
	// Move constructor and assignment operator
	EBO(EBO&& other) noexcept : ID(other.ID) {
		other.ID = 0;
	}
	
	EBO& operator=(EBO&& other) noexcept {
		if (this != &other) {
			destroy();
			ID = other.ID;
			other.ID = 0;
		}
		return *this;
	}
	
	void create(const std::vector<GLuint>& indices) {
		if (ID) destroy();
		
		glGenBuffers(1, &ID);
		GL_CHECK();
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
		GL_CHECK();
	}

	void bind() { 
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); 
		GL_CHECK();
	}
	
	void unbind() { 
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
		GL_CHECK();
	}
	
	void destroy() { 
		if (ID) {
			glDeleteBuffers(1, &ID); 
			ID = 0;
			GL_CHECK();
		}
	}
	
	GLuint getID() const { return ID; }

private:
	GLuint ID;
};

class FBO {
public:
	FBO() : ID(0) { 
		glGenFramebuffers(1, &ID); 
		GL_CHECK();
	}
	
	~FBO() { destroy(); }
	
	// Delete copy constructor and assignment operator
	FBO(const FBO&) = delete;
	FBO& operator=(const FBO&) = delete;
	
	// Move constructor and assignment operator
	FBO(FBO&& other) noexcept : ID(other.ID) {
		other.ID = 0;
	}
	
	FBO& operator=(FBO&& other) noexcept {
		if (this != &other) {
			destroy();
			ID = other.ID;
			other.ID = 0;
		}
		return *this;
	}
	
	void bind() { 
		glBindFramebuffer(GL_FRAMEBUFFER, ID); 
		GL_CHECK();
	}
	
	void unbind() { 
		glBindFramebuffer(GL_FRAMEBUFFER, 0); 
		GL_CHECK();
	}
	
	void destroy() { 
		if (ID) {
			glDeleteFramebuffers(1, &ID); 
			ID = 0;
			GL_CHECK();
		}
	}
	
	void attachTexture(std::shared_ptr<Texture> texture, GLenum attachment) {
		bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->getID(), 0);
		GL_CHECK();
		// Don't set draw/read buffers here - do it after all attachments are done
		unbind();
	}
	
	// Call this after attaching all textures
	void finalize(int numDrawBuffers = 1) {
		bind();
		
		if (numDrawBuffers == 2) {
			// Set up dual render targets
			GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, drawBuffers);
		} 
		else {
			// Default single render target
			GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, drawBuffers);
		}
		GL_CHECK();
		
		if (!checkFramebufferStatus()) {
			ERROR("Framebuffer is not complete!");
		}
		
		unbind();
	}
	
	GLuint getID() const { return ID; }
	
private:
	GLuint ID;
	
	bool checkFramebufferStatus() const {
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		GL_CHECK();
		
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			ERROR("Framebuffer is not complete. Status: " + std::to_string(status));
			return false;
		}
		return true;
	}
};

class UBO {
public:
	UBO() : ID(0) { 
		glGenBuffers(1, &ID); 
		GL_CHECK();
	}
	
	~UBO() { destroy(); }
	
	// Delete copy constructor and assignment operator
	UBO(const UBO&) = delete;
	UBO& operator=(const UBO&) = delete;
	
	// Move constructor and assignment operator
	UBO(UBO&& other) noexcept : ID(other.ID) {
		other.ID = 0;
	}
	
	UBO& operator=(UBO&& other) noexcept {
		if (this != &other) {
			destroy();
			ID = other.ID;
			other.ID = 0;
		}
		return *this;
	}

	void bind() { 
		glBindBuffer(GL_UNIFORM_BUFFER, ID); 
		GL_CHECK();
	}
	
	void unbind() { 
		glBindBuffer(GL_UNIFORM_BUFFER, 0); 
		GL_CHECK();
	}
	
	void destroy() { 
		if (ID) {
			glDeleteBuffers(1, &ID); 
			ID = 0;
			GL_CHECK();
		}
	}

	void allocate(GLsizeiptr size, GLenum usage = GL_STATIC_DRAW) {
		bind();
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, usage);
		unbind();
		GL_CHECK();
	}

	void update(GLintptr offset, GLsizeiptr size, const void* data) {
		bind();
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
		unbind();
		GL_CHECK();
	}

	void bindToIndex(GLuint index) {
		glBindBufferBase(GL_UNIFORM_BUFFER, index, ID);
		GL_CHECK();
	}
	
	GLuint getID() const { return ID; }

private:
	GLuint ID;
};

// Predefinitions
class Node;
class Mesh;
class Material;
class RigidBody;
class Collider;
class Entity;
class Scene;
class BoxCollider;
class SphereCollider;
class CapsuleCollider;

///////////////////////
// Rendering Classes //
///////////////////////

enum class InterpolationType {
	LINEAR,
	STEP,
	CUBICSPLINE
};

enum class AnimationPathType {
	TRANSLATION,
	ROTATION,
	SCALE,
	WEIGHTS
};

template <typename T>
struct Keyframe {
	float time;
	T value;
};

struct AnimationChannel {
	std::string targetNodeName;
	AnimationPathType pathType; // Property to animate
	InterpolationType interpolation;
	
	// Keyframes for different types
	std::vector<Keyframe<glm::vec3>> translationKeys;
	std::vector<Keyframe<glm::quat>> rotationKeys;
	std::vector<Keyframe<glm::vec3>> scaleKeys;
	std::vector<Keyframe<std::vector<float>>> weightKeys;
	
	// Target node reference
	std::weak_ptr<Node> targetNode;
};

struct Animation {
	std::string name;                       // Animation name
	std::vector<AnimationChannel> channels; // Animation channels
	float duration;                         // Duration in seconds
	
	void calculateDuration() {
		duration = 0.0f;
		for (const auto& channel : channels) {
			// Check each keyframe type and find the maximum time value
			for (const auto& key : channel.translationKeys) {
				duration = std::max(duration, key.time);
			}
			for (const auto& key : channel.rotationKeys) {
				duration = std::max(duration, key.time);
			}
			for (const auto& key : channel.scaleKeys) {
				duration = std::max(duration, key.time);
			}
			for (const auto& key : channel.weightKeys) {
				duration = std::max(duration, key.time);
			}
		}
	}
};

struct Skin {
	std::string name;                            // Skin name
	std::vector<std::string> jointNodeNames;     // Names of nodes used as joints
	std::vector<std::weak_ptr<Node>> joints;     // References to joint nodes
	std::vector<glm::mat4> inverseBindMatrices;  // Inverse bind matrices
	std::shared_ptr<Node> skeletonRoot;          // Optional root node of skeleton
};

class Node : public std::enable_shared_from_this<Node> {
public:
	Node() : position(0.0f), eulerRotation(0.0f), quaternion(1.0f, 0.0f, 0.0f, 0.0f), scale(1.0f), matrix(1.0f) {}
	
	Node(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl) : position(pos), eulerRotation(rot), scale(scl), matrix(1.0f) {
		quaternion = glm::quat(eulerRotation);
		updateMatrix();
	}

	void updateMatrix() {
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotationMatrix = glm::mat4_cast(quaternion);
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
		
		glm::mat4 localMatrix = translationMatrix * rotationMatrix * scaleMatrix;
		
		if (auto parentNode = parent.lock()) {
			matrix = parentNode->matrix * localMatrix;
		} 
		else {
			matrix = localMatrix;
		}
		
		for (auto& child : children) {
			child->updateMatrix();
		}
	}

	glm::vec3 getPosition() const { return position; }
	void setPosition(const glm::vec3& pos) {
		position = pos;
		updateMatrix();
	}

	glm::vec3 getEulerRotation() const { return eulerRotation; }
	void setEulerRotation(const glm::vec3& rot) {
		eulerRotation = rot;
		quaternion = glm::quat(eulerRotation);
		updateMatrix();
	}

	glm::quat getQuaternion() const { return quaternion; }
	void setQuaternion(const glm::quat& quat) {
		quaternion = quat;
		eulerRotation = glm::eulerAngles(quaternion);
		updateMatrix();
	}

	glm::vec3 getScale() const { return scale; }
	void setScale(const glm::vec3& scl) {
		scale = scl;
		updateMatrix();
	}

	glm::mat4 getMatrix() const { return matrix; } // World transform matrix
	void setMatrix(const glm::mat4& mat) {
		matrix = mat;

		position = glm::vec3(matrix[3]);
		scale = glm::vec3(
			glm::length(glm::vec3(matrix[0])),
			glm::length(glm::vec3(matrix[1])),
			glm::length(glm::vec3(matrix[2]))
		);
		
		glm::mat3 rotMat(
			glm::vec3(matrix[0]) / scale.x,
			glm::vec3(matrix[1]) / scale.y,
			glm::vec3(matrix[2]) / scale.z
		);
		quaternion = glm::quat(rotMat);
		eulerRotation = glm::eulerAngles(quaternion);
	}

	std::string getName() const { return name; }
	void setName(const std::string& nme) { name = nme; }

	size_t getIndex() const { return index; }
	void setIndex(const size_t& idx) { index = idx; }

	std::shared_ptr<Mesh> getMesh() const { return mesh; }
	void setMesh(const std::shared_ptr<Mesh>& msh) { mesh = msh; }

	std::shared_ptr<Material> getMaterial() const { return material; }
	void setMaterial(const std::shared_ptr<Material>& mtrl) { material = mtrl; }

	std::weak_ptr<Node> getParent() const { return parent; }
	std::vector<std::shared_ptr<Node>> getChildren() const { return children; }
	void addChild(const std::shared_ptr<Node>& child) {
		children.push_back(child);
		child->parent = shared_from_this();
		child->updateMatrix();
	}

private:
	glm::vec3 position;
	glm::vec3 eulerRotation;
	glm::quat quaternion;
	glm::vec3 scale;
	glm::mat4 matrix;

	std::weak_ptr<Node> parent;
	std::vector<std::shared_ptr<Node>> children;

	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;

	size_t index = 0;
	std::string name;
};

struct VertexAttribute {
	GLuint index;   // Attribute location
	GLint size;     // Components per attribute
	GLenum type;    // GL_FLOAT, GL_INT, etc.
	GLsizei stride; // Byte size of a full vertex
	size_t offset;  // Offset into vertex data
};

class BoundingBox {
public:
	BoundingBox() {
		reset();
	}
	
	BoundingBox(const glm::vec3& min, const glm::vec3& max) {
		setMinMax(min, max);
	}
	
	void reset() {
		minExtents = glm::vec3(FLT_MAX);
		maxExtents = glm::vec3(-FLT_MAX);
		valid = false;
		updateDerivedValues();
	}
	
	void setMinMax(const glm::vec3& min, const glm::vec3& max) {
		minExtents = min;
		maxExtents = max;
		valid = true;
		updateDerivedValues();
	}
	
	void expandToInclude(const glm::vec3& point) {
		if (!valid) {
			minExtents = maxExtents = point;
			valid = true;
		} 
		else {
			minExtents = glm::min(minExtents, point);
			maxExtents = glm::max(maxExtents, point);
		}
		updateDerivedValues();
	}
	
	void calculateFromVertices(const std::vector<uint8_t>& vertices, const std::vector<VertexAttribute>& layout) {
		reset();
		
		const VertexAttribute* positionAttr = nullptr;
		for (const auto& attr : layout) {
			if (attr.index == 0) {
				positionAttr = &attr;
				break;
			}
		}
		
		if (!positionAttr || positionAttr->size < 3) {
			setMinMax(glm::vec3(-1.0f), glm::vec3(1.0f));
			return;
		}
		
		if (vertices.empty() || positionAttr->stride == 0) {
			setMinMax(glm::vec3(0.0f), glm::vec3(0.0f));
			return;
		}
		
		size_t vertexCount = vertices.size() / positionAttr->stride;
		
		for (size_t i = 0; i < vertexCount; i++) {
			const uint8_t* vertexData = vertices.data() + i * positionAttr->stride + positionAttr->offset;
			
			if (positionAttr->type == GL_FLOAT) {
				const float* floatPos = reinterpret_cast<const float*>(vertexData);
				glm::vec3 position(floatPos[0], floatPos[1], floatPos[2]);
				expandToInclude(position);
			}
		}
		
		if (!valid) {
			setMinMax(glm::vec3(0.0f), glm::vec3(0.0f));
		}
	}
	
	const glm::vec3& getMinExtents() const { return minExtents; }
	const glm::vec3& getMaxExtents() const { return maxExtents; }
	const glm::vec3& getCenter() const { return center; }
	const glm::vec3& getHalfExtents() const { return halfExtents; }
	glm::vec3 getSize() const { return maxExtents - minExtents; }
	bool isValid() const { return valid; }

private:
	glm::vec3 minExtents;
	glm::vec3 maxExtents;
	glm::vec3 center;
	glm::vec3 halfExtents;
	bool valid = false;
	
	void updateDerivedValues() {
		if (valid) {
			center = (minExtents + maxExtents) * 0.5f;
			halfExtents = (maxExtents - minExtents) * 0.5f;
		} 
		else {
			center = glm::vec3(0.0f);
			halfExtents = glm::vec3(0.0f);
		}
	}
};

class Mesh : public std::enable_shared_from_this<Mesh> {
public:
	Mesh() {}

	void create(const std::vector<uint8_t>& vertices, const std::vector<GLuint>& indices, const std::vector<VertexAttribute>& layout) {
		vao.bind();
		
		vbo.create(vertices);
		vbo.bind();
		
		ebo.create(indices);
		ebo.bind();
		
		for (const auto& attr : layout) {
			vao.linkAttribute(attr.index, attr.size, attr.type, attr.stride, attr.offset);
		}
		
		vao.unbind();
		vbo.unbind();
		ebo.unbind();
		
		indexCount = indices.size();
		boundingBox.calculateFromVertices(vertices, layout);
	}
	
	void draw() {
		vao.bind();
		GL_CHECK();
		
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		GL_CHECK();
		
		vao.unbind();
	}

	bool isValid() const {
		return indexCount > 0 && vao.getID() != 0 && vbo.getID() != 0 && ebo.getID() != 0;
	}

	BoundingBox& getBoundingBox() { return boundingBox; }
	glm::vec3 getMinExtents() const { return boundingBox.getMinExtents(); }
	glm::vec3 getMaxExtents() const { return boundingBox.getMaxExtents(); }
	glm::vec3 getHalfExtents() const { return boundingBox.getHalfExtents(); }
	glm::vec3 getCenter() const { return boundingBox.getCenter(); }
	glm::vec3 getSize() const { return boundingBox.getSize(); }
	bool hasBoundingBox() const { return boundingBox.isValid(); }
	
private:
	GLR::VAO vao;
	GLR::VBO<uint8_t> vbo;
	GLR::VBO<uint8_t> skinVbo;
	GLR::EBO ebo;
	GLsizei indexCount = 0;
	BoundingBox boundingBox;
};

class AnimationManager {
public:
	struct NodeState {
		glm::vec3 position = glm::vec3(0.0f);
		glm::quat quaternion = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 scale = glm::vec3(1.0f);
	};

	AnimationManager() : currentTime(0.0f), isPlaying(false), looping(true), speedFactor(1.0f), blendFactor(0.0f), blendDuration(0.25f), isBlending(false) {}
	
	void loadAnimations(const std::vector<Animation>& modelAnimations, const std::vector<std::shared_ptr<Node>>& nodes) {
		animations = modelAnimations;
		this->nodes = nodes;
		
		// Store initial pose for each node
		initialNodeStates.clear();
		for (const auto& node : nodes) {
			NodeState state;
			state.position = node->getPosition();
			state.quaternion = node->getQuaternion();
			state.scale = node->getScale();
			initialNodeStates[node->getName()] = state;
		}
		
		// Node references in animation channels
		for (auto& animation : animations) {
			for (auto& channel : animation.channels) {
				for (const auto& node : nodes) {
					if (node->getName() == channel.targetNodeName) {
						channel.targetNode = node;
						break;
					}
				}
			}
		}
	}

	void playAnimation(const std::string& animationName, bool shouldLoop = true) {
		size_t newAnimIndex = -1;
		for (size_t i = 0; i < animations.size(); i++) {
			if (animations[i].name == animationName) {
				newAnimIndex = i;
				break;
			}
		}

		if (newAnimIndex == -1) {
			ERROR("Animation: '" + animationName + "' not found:");
			return;
		}
		
		// If animation already playing
		if (isPlaying && currentAnimationIndex == newAnimIndex) {
			looping = shouldLoop;
			return;
		}
		
		// Store current pose as starting point for blending
		captureCurrentPose();
		
		// Set up blend parameters
		previousAnimationIndex = isPlaying ? currentAnimationIndex : -1;
		currentAnimationIndex = newAnimIndex;
		blendFactor = 0.0f;
		isBlending = true;
		isPlaying = true;
		looping = shouldLoop;

		currentTime = 0.0f;
	}
	
	void stopAnimation() {
		if (!isPlaying) return;
		
		// Store current pose as starting point for blending back to rest
		captureCurrentPose();
		
		previousAnimationIndex = currentAnimationIndex;
		currentAnimationIndex = -1; // No current animation
		blendFactor = 0.0f;
		isBlending = true;
		isPlaying = false;
	}
	
	void update(float deltaTime) {
		if (!isPlaying && !isBlending) {
			return;
		}

		float scaledDeltaTime = deltaTime * speedFactor;
		
		// Handle blending
		if (isBlending) {
			blendFactor += scaledDeltaTime / blendDuration;
			
			if (blendFactor >= 1.0f) {
				// Blending complete
				blendFactor = 1.0f;
				isBlending = false;
				
				// If blending to rest pose
				if (currentAnimationIndex == -1) {
					return;
				}
			}
		}
		
		// If there is an animation
		if (currentAnimationIndex >= 0 && currentAnimationIndex < animations.size()) {
			const Animation& currentAnimation = animations[currentAnimationIndex];
			
			currentTime += scaledDeltaTime;
			
			// Handle animation looping
			if (currentTime > currentAnimation.duration) {
				if (looping) {
					currentTime = std::fmod(currentTime, currentAnimation.duration);
				} 
				else {
					currentTime = currentAnimation.duration;
					isPlaying = false;
					
					stopAnimation();
				}
			}
		}
		
		applyAnimations();
	}

	std::vector<std::string> getAnimationNames() const {
		std::vector<std::string> names;
		for (const auto& animation : animations) {
			names.push_back(animation.name);
		}
		return names;
	}
	const std::vector<Animation>& getAnimations() const { return animations; }

	void setSpeed(float speed) { speedFactor = speed; }
	void setBlendDuration(float duration) { blendDuration = duration; }
	
	float getCurrentTime() const { return currentTime; }
	float getSpeed() const { return speedFactor; }
	bool getIsPlaying() const { return isPlaying; }
	bool getIsBlending() const { return isBlending; }

	std::string getCurrentAnimationName() const {
		if (currentAnimationIndex >= 0 && currentAnimationIndex < animations.size()) {
			return animations[currentAnimationIndex].name;
		}
		return "";
	}
	
private:
	std::vector<Animation> animations;
	std::vector<std::shared_ptr<Node>> nodes;
	std::map<std::string, NodeState> initialNodeStates;
	std::map<std::string, NodeState> currentPose;
	
	size_t currentAnimationIndex = 0;
	size_t previousAnimationIndex = -1;
	float currentTime;
	bool isPlaying;
	bool looping;
	float speedFactor;
	
	bool isBlending;
	float blendFactor;
	float blendDuration;

	// Store current pose of all nodes for blending
	void captureCurrentPose() {
		currentPose.clear();
		
		for (const auto& node : nodes) {
			if (!node) continue;
			
			NodeState state;
			state.position = node->getPosition();
			state.quaternion = node->getQuaternion();
			state.scale = node->getScale();
			currentPose[node->getName()] = state;
		}
	}
	
	void applyAnimations() {
		// If no animation and no blending reset all nodes to their initial state
		if (currentAnimationIndex == -1 && !isBlending) {
			resetNodesToInitialState();
			return;
		}
		
		// If blending to rest pose
		if (currentAnimationIndex == -1 && isBlending) {
			blendToInitialState();
			return;
		}
		
		// Apply the current animation
		if (currentAnimationIndex >= 0 && currentAnimationIndex < animations.size()) {
			const Animation& currentAnimation = animations[currentAnimationIndex];

			// Blending between two animations
			if (isBlending && previousAnimationIndex >= 0 && previousAnimationIndex < animations.size()) {
				const Animation& prevAnimation = animations[previousAnimationIndex];
				
				// Computing blended result for each node thats affected by either animation
				std::map<std::string, bool> processedNodes;
				
				// Process previous animation nodes
				for (const auto& channel : prevAnimation.channels) {
					auto targetNode = channel.targetNode.lock();
					if (!targetNode) continue;
					
					if (processedNodes[targetNode->getName()]) continue;
					processedNodes[targetNode->getName()] = true;
					
					const NodeState& fromState = currentPose[targetNode->getName()];
					
					// Compute target pose from current animation
					NodeState toState;
					computeNodeState(currentAnimation, targetNode->getName(), currentTime, toState);
					
					// Apply blended transform
					targetNode->setPosition(glm::mix(fromState.position, toState.position, blendFactor));
					targetNode->setQuaternion(glm::slerp(fromState.quaternion, toState.quaternion, blendFactor));
					targetNode->setScale(glm::mix(fromState.scale, toState.scale, blendFactor));
					targetNode->setEulerRotation(glm::eulerAngles(targetNode->getQuaternion()));
					
					targetNode->updateMatrix();
				}
				
				// Process any new nodes from the current animation
				for (const auto& channel : currentAnimation.channels) {
					auto targetNode = channel.targetNode.lock();
					if (!targetNode) continue;
					
					if (processedNodes[targetNode->getName()]) continue;
					
					const NodeState& fromState = currentPose[targetNode->getName()];
					
					// Compute target pose
					NodeState toState;
					computeNodeState(currentAnimation, targetNode->getName(), currentTime, toState);
					
					// Apply blended transform
					targetNode->setPosition(glm::mix(fromState.position, toState.position, blendFactor));
					targetNode->setQuaternion(glm::slerp(fromState.quaternion, toState.quaternion, blendFactor));
					targetNode->setScale(glm::mix(fromState.scale, toState.scale, blendFactor));
					targetNode->setEulerRotation(glm::eulerAngles(targetNode->getQuaternion()));
					
					targetNode->updateMatrix();
				}
			} 
			else {
				// Regular single animation playback (no blending)
				applyAnimation(currentAnimation, currentTime);
			}
		}
	}
	
	void resetNodesToInitialState() {
		for (const auto& node : nodes) {
			if (!node) continue;
			
			auto it = initialNodeStates.find(node->getName());
			if (it != initialNodeStates.end()) {
				node->setPosition(it->second.position);
				node->setQuaternion(it->second.quaternion);
				node->setScale(it->second.scale);
				node->setEulerRotation(glm::eulerAngles(node->getQuaternion()));

				node->updateMatrix();
			}
		}
	}
	
	void blendToInitialState() {
		for (const auto& node : nodes) {
			if (!node) continue;
			
			auto currentIt = currentPose.find(node->getName());
			auto initialIt = initialNodeStates.find(node->getName());
			
			if (currentIt != currentPose.end() && initialIt != initialNodeStates.end()) {
				const NodeState& fromState = currentIt->second;
				const NodeState& toState = initialIt->second;
				
				// Apply blended transform
				node->setPosition(glm::mix(fromState.position, toState.position, blendFactor));
				node->setQuaternion(glm::slerp(fromState.quaternion, toState.quaternion, blendFactor));
				node->setScale(glm::mix(fromState.scale, toState.scale, blendFactor));
				node->setEulerRotation(glm::eulerAngles(node->getQuaternion()));
				
				node->updateMatrix();
			}
		}
	}
	
	// Apply animation at a specific time
	void applyAnimation(const Animation& animation, float time) {
		for (const auto& channel : animation.channels) {
			auto targetNode = channel.targetNode.lock();
			if (!targetNode) continue;
			
			switch (channel.pathType) {
				case AnimationPathType::TRANSLATION:
					applyTranslation(channel, targetNode, time);
					break;
					
				case AnimationPathType::ROTATION:
					applyRotation(channel, targetNode, time);
					break;
					
				case AnimationPathType::SCALE:
					applyScale(channel, targetNode, time);
					break;
					
				case AnimationPathType::WEIGHTS:
					// Morph targets not implemented
					break;
			}
			
			targetNode->updateMatrix();
		}
	}
	
	void computeNodeState(const Animation& animation, const std::string& nodeName, float time, NodeState& outState) {
		auto initialIt = initialNodeStates.find(nodeName);
		if (initialIt != initialNodeStates.end()) {
			outState = initialIt->second;
		}
		
		for (const auto& channel : animation.channels) {
			if (channel.targetNodeName != nodeName) continue;
			
			auto targetNode = channel.targetNode.lock();
			if (!targetNode) continue;
			
			switch (channel.pathType) {
				case AnimationPathType::TRANSLATION:
					computeTranslation(channel, time, outState.position);
					break;
					
				case AnimationPathType::ROTATION:
					computeRotation(channel, time, outState.quaternion);
					break;
					
				case AnimationPathType::SCALE:
					computeScale(channel, time, outState.scale);
					break;
					
				case AnimationPathType::WEIGHTS:
					// Morph targets not implemented
					break;
			}
		}
	}
	
	void applyTranslation(const AnimationChannel& channel, std::shared_ptr<Node>& node, float time) {
		if (channel.translationKeys.empty()) return;
		
		// Handle special cases only one keyframe or time before first keyframe
		if (channel.translationKeys.size() == 1 || time <= channel.translationKeys.front().time) {
			node->setPosition(channel.translationKeys.front().value);
			return;
		}
		
		// Handle time after last keyframe
		if (time >= channel.translationKeys.back().time) {
			node->setPosition(channel.translationKeys.back().value);
			return;
		}
		
		// Find the two keyframes to interpolate between
		size_t nextKeyIndex = 0;
		while (nextKeyIndex < channel.translationKeys.size() && channel.translationKeys[nextKeyIndex].time < time) {
			nextKeyIndex++;
		}
		
		size_t prevKeyIndex = nextKeyIndex - 1;
		
		const auto& prevKey = channel.translationKeys[prevKeyIndex];
		const auto& nextKey = channel.translationKeys[nextKeyIndex];
		
		// Calculate interpolation factor
		float t = (time - prevKey.time) / (nextKey.time - prevKey.time);
		
		switch (channel.interpolation) {
			case InterpolationType::STEP:
				node->setPosition(prevKey.value);
				break;
				
			case InterpolationType::LINEAR:
				node->setPosition(glm::mix(prevKey.value, nextKey.value, t));
				break;
				
			case InterpolationType::CUBICSPLINE:
				// Cubic spline interpolation not implemented, just using linear interpolation
				node->setPosition(glm::mix(prevKey.value, nextKey.value, t));
				break;
		}
	}
	
	void computeTranslation(const AnimationChannel& channel, float time, glm::vec3& outPosition) {
		if (channel.translationKeys.empty()) return;
		
		if (channel.translationKeys.size() == 1 || time <= channel.translationKeys.front().time) {
			outPosition = channel.translationKeys.front().value;
			return;
		}
		
		if (time >= channel.translationKeys.back().time) {
			outPosition = channel.translationKeys.back().value;
			return;
		}
		
		size_t nextKeyIndex = 0;
		while (nextKeyIndex < channel.translationKeys.size() && channel.translationKeys[nextKeyIndex].time < time) {
			nextKeyIndex++;
		}
		
		size_t prevKeyIndex = nextKeyIndex - 1;
		
		const auto& prevKey = channel.translationKeys[prevKeyIndex];
		const auto& nextKey = channel.translationKeys[nextKeyIndex];
		
		float t = (time - prevKey.time) / (nextKey.time - prevKey.time);
		
		switch (channel.interpolation) {
			case InterpolationType::STEP:
				outPosition = prevKey.value;
				break;
				
			case InterpolationType::LINEAR:
			case InterpolationType::CUBICSPLINE: // Fallback to linear
				outPosition = glm::mix(prevKey.value, nextKey.value, t);
				break;
		}
	}
	
	void applyRotation(const AnimationChannel& channel, std::shared_ptr<Node>& node, float time) {
		if (channel.rotationKeys.empty()) return;
		
		// Handle special cases
		if (channel.rotationKeys.size() == 1 || time <= channel.rotationKeys.front().time) {
			node->setQuaternion(channel.rotationKeys.front().value);
			node->setEulerRotation(glm::eulerAngles(node->getQuaternion()));
			return;
		}
		
		if (time >= channel.rotationKeys.back().time) {
			node->setQuaternion(channel.rotationKeys.back().value);
			node->setEulerRotation(glm::eulerAngles(node->getQuaternion()));
			return;
		}
		
		size_t nextKeyIndex = 0;
		while (nextKeyIndex < channel.rotationKeys.size() && channel.rotationKeys[nextKeyIndex].time < time) {
			nextKeyIndex++;
		}
		
		size_t prevKeyIndex = nextKeyIndex - 1;
		
		const auto& prevKey = channel.rotationKeys[prevKeyIndex];
		const auto& nextKey = channel.rotationKeys[nextKeyIndex];
		
		float t = (time - prevKey.time) / (nextKey.time - prevKey.time);
		
		switch (channel.interpolation) {
			case InterpolationType::STEP:
				node->setQuaternion(prevKey.value);
				break;
				
			case InterpolationType::LINEAR:
			case InterpolationType::CUBICSPLINE:
				node->setQuaternion(glm::slerp(prevKey.value, nextKey.value, t));
				break;
		}
		
		node->setEulerRotation(glm::eulerAngles(node->getQuaternion()));
	}
	
	void computeRotation(const AnimationChannel& channel, float time, glm::quat& outQuaternion) {
		if (channel.rotationKeys.empty()) return;
		
		if (channel.rotationKeys.size() == 1 || time <= channel.rotationKeys.front().time) {
			outQuaternion = channel.rotationKeys.front().value;
			return;
		}
		
		if (time >= channel.rotationKeys.back().time) {
			outQuaternion = channel.rotationKeys.back().value;
			return;
		}
		
		size_t nextKeyIndex = 0;
		while (nextKeyIndex < channel.rotationKeys.size() && channel.rotationKeys[nextKeyIndex].time < time) {
			nextKeyIndex++;
		}
		
		size_t prevKeyIndex = nextKeyIndex - 1;
		
		const auto& prevKey = channel.rotationKeys[prevKeyIndex];
		const auto& nextKey = channel.rotationKeys[nextKeyIndex];
		
		float t = (time - prevKey.time) / (nextKey.time - prevKey.time);
		
		switch (channel.interpolation) {
			case InterpolationType::STEP:
				outQuaternion = prevKey.value;
				break;
				
			case InterpolationType::LINEAR:
			case InterpolationType::CUBICSPLINE:
				outQuaternion = glm::slerp(prevKey.value, nextKey.value, t);
				break;
		}
	}
	
	void applyScale(const AnimationChannel& channel, std::shared_ptr<Node>& node, float time) {
		if (channel.scaleKeys.empty()) return;
		
		if (channel.scaleKeys.size() == 1 || time <= channel.scaleKeys.front().time) {
			node->setScale(channel.scaleKeys.front().value);
			return;
		}
		
		if (time >= channel.scaleKeys.back().time) {
			node->setScale(channel.scaleKeys.back().value);
			return;
		}
		
		size_t nextKeyIndex = 0;
		while (nextKeyIndex < channel.scaleKeys.size() && channel.scaleKeys[nextKeyIndex].time < time) {
			nextKeyIndex++;
		}
		
		size_t prevKeyIndex = nextKeyIndex - 1;
		
		const auto& prevKey = channel.scaleKeys[prevKeyIndex];
		const auto& nextKey = channel.scaleKeys[nextKeyIndex];
		
		float t = (time - prevKey.time) / (nextKey.time - prevKey.time);
		
		switch (channel.interpolation) {
			case InterpolationType::STEP:
				node->setScale(prevKey.value);
				break;
				
			case InterpolationType::LINEAR:
			case InterpolationType::CUBICSPLINE:
				node->setScale(glm::mix(prevKey.value, nextKey.value, t));
				break;
		}
	}
	
	void computeScale(const AnimationChannel& channel, float time, glm::vec3& outScale) {
		if (channel.scaleKeys.empty()) return;
		
		if (channel.scaleKeys.size() == 1 || time <= channel.scaleKeys.front().time) {
			outScale = channel.scaleKeys.front().value;
			return;
		}
		
		if (time >= channel.scaleKeys.back().time) {
			outScale = channel.scaleKeys.back().value;
			return;
		}
		
		size_t nextKeyIndex = 0;
		while (nextKeyIndex < channel.scaleKeys.size() && channel.scaleKeys[nextKeyIndex].time < time) {
			nextKeyIndex++;
		}
		
		size_t prevKeyIndex = nextKeyIndex - 1;
		
		const auto& prevKey = channel.scaleKeys[prevKeyIndex];
		const auto& nextKey = channel.scaleKeys[nextKeyIndex];
		
		float t = (time - prevKey.time) / (nextKey.time - prevKey.time);
		
		switch (channel.interpolation) {
			case InterpolationType::STEP:
				outScale = prevKey.value;
				break;
				
			case InterpolationType::LINEAR:
			case InterpolationType::CUBICSPLINE:
				outScale = glm::mix(prevKey.value, nextKey.value, t);
				break;
		}
	}
};

class Material {
public:
	Material(std::shared_ptr<GLR::Shader> shader) : shader(shader) {}
	
	void setTexture(const std::string& name, const std::shared_ptr<GLR::Texture>& texture) {
		textures[name] = texture;
	}

	void setFloat(const std::string& name, const float& value) {
		floatValues[name] = value;
	}
	
	void setVector3(const std::string& name, const glm::vec3& value) {
		vec3Values[name] = value;
	}
	
	void setVector4(const std::string& name, const glm::vec4& value) {
		vec4Values[name] = value;
	}
	
	void bind() {
		if (!shader) {
			ERROR("Shader missing in Material!");
			return;
		}
		
		shader->bind();
		shader->setBool("baseColorTextureBool", false);
		shader->setBool("normalTextureBool", false);
		shader->setBool("metallicRoughnessTextureBool", false);
		shader->setBool("emissiveTextureBool", false);
				
		int textureUnit = 0;
		for (auto& [name, texture] : textures) {
			if (texture) {
				texture->bind(textureUnit);
				shader->setInt(name, textureUnit);
				shader->setBool(name + std::string("Bool"), true);
				textureUnit++;
			}
		}
		
		for (auto& [name, value] : vec3Values) {
			shader->setVector3Float(name, value);
		}
		
		for (auto& [name, value] : vec4Values) {
			shader->setVector4Float(name, value);
		}
		
		for (auto& [name, value] : floatValues) {
			shader->setFloat(name, value);
		}
	}
	
	void unbind() {
		if (!shader) {
			ERROR("Shader missing in Material!");
			return;
		}
		shader->unbind();
	}

	float* getFloatPtr(const std::string& name) {
		auto it = floatValues.find(name);
		if (it != floatValues.end()) {
			return &it->second;
		}
		else {
			ERROR(name + " Not found in Material!");
		}
		return nullptr;
	}
	  
	std::shared_ptr<GLR::Shader> getShader() const { return shader; }
	void setShader(const std::shared_ptr<GLR::Shader>& newShader) { shader = newShader; }
	
	std::unordered_map<std::string, std::shared_ptr<GLR::Texture>> getTextures() const { return textures; }
	
	void setAlphaMode(const std::string& mode) { alphaMode = mode; }
	bool isTransparent() const {
		// Check glTF alpha mode
		if (alphaMode == "MASK") {
			return true;
		}
		
		// Check base color alpha
		auto it = vec4Values.find("baseColorFactor");
		if (it != vec4Values.end()) {
			if (it->second.a < 1.0f) {
				return true;
			}
		}
		
		return false;
	}

	bool isDoubleSided() const { return doubleSided; }
	void setDoubleSided(bool newDoubleSided) { doubleSided = newDoubleSided; }
		
private:
	std::shared_ptr<GLR::Shader> shader;
	std::string alphaMode = "OPAQUE";
	bool doubleSided = true;

	std::unordered_map<std::string, std::shared_ptr<GLR::Texture>> textures;
	std::unordered_map<std::string, glm::vec3> vec3Values;
	std::unordered_map<std::string, glm::vec4> vec4Values;
	std::unordered_map<std::string, float> floatValues;
};

struct PrimitiveData {
	std::vector<uint8_t> vertices;
	std::vector<GLuint> indices;
	std::vector<VertexAttribute> attributes;
	std::shared_ptr<Material> material;
};

class Model : public std::enable_shared_from_this<Model> {
public:
	Model(bool isDoubleSide = false, bool isTransparent = false) : isDoubleSided(isDoubleSide), isTransparent(isTransparent) {}

	bool create(const std::string& filename, std::shared_ptr<GLR::Shader> defaultShader) {
		if (!loadGLTFFile(filename)) {
			ERROR("Model '" + filename + "' Failed to load.");
			return false;
		}
		
		this->defaultShader = defaultShader;
		this->filename = filename;
		
		if (!processModelData()) {
			ERROR("Model '" + filename + "' Failed to load.");
			return false;
		}
		
		// Debug output
		#ifdef DEBUG
		printNodeHierarchy();
		#endif
		return true;
	}

	// Node Management
	std::vector<std::shared_ptr<Node>> getNodes() { return nodes; }
	std::vector<std::shared_ptr<Node>> getRootNodes() const {
		std::vector<std::shared_ptr<Node>> rootNodes;
		for (const auto& node : nodes) {
			if (!node->getParent().lock()) {
				rootNodes.push_back(node);
			}
		}
		return rootNodes;
	}
	std::shared_ptr<Node> findNodeByName(const std::string& name) const {
		for (const auto& node : nodes) {
			if (node->getName() == name) {
				return node;
			}
		}
		return nullptr;
	}

	// Skinning Management
	std::vector<std::shared_ptr<Skin>> getSkins() { return skins; }
	const std::vector<std::weak_ptr<Node>>& getJoints() const {
		static std::vector<std::weak_ptr<Node>> emptyJoints;
		
		if (!skins.empty() && skins[0]) {
			return skins[0]->joints;
		}
		return emptyJoints;
	}

	// Utility
	std::string getFileName() { return filename; }
	std::vector<std::shared_ptr<Material>> getMaterials() { return materials; }
	const std::shared_ptr<AnimationManager>& getAnimationManager() const { return animationManager; }
	BoundingBox& getBoundingBox() { return boundingBox; }
	glm::vec3 getMinExtents() const { return boundingBox.getMinExtents(); }
	glm::vec3 getMaxExtents() const { return boundingBox.getMaxExtents(); }
	glm::vec3 getHalfExtents() const { return boundingBox.getHalfExtents(); }
	glm::vec3 getCenter() const { return boundingBox.getCenter(); }
	glm::vec3 getSize() const { return boundingBox.getSize(); }

private:
	std::string filename;
	std::vector<std::shared_ptr<Node>> nodes;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<std::vector<PrimitiveData>> primitives;
	std::shared_ptr<GLR::Shader> defaultShader;
	std::vector<std::shared_ptr<Skin>> skins;
	std::shared_ptr<AnimationManager> animationManager;
	bool isDoubleSided;
	bool isTransparent;
	BoundingBox boundingBox;
	
	// Store loaded GLTF model for processing
	tinygltf::Model gltfModel;

	// Core Loading Logic
	bool loadGLTFFile(const std::string& filename) {
		tinygltf::TinyGLTF loader;
		std::string err, warn;
		bool ret = false;
		
		if (filename.find(".glb") != std::string::npos) {
			ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filename);
		} 
		else {
			ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filename);
		}
		
		if (!warn.empty()) {
			ERROR("TinyGLTF warning: " + warn);
		}
		
		if (!err.empty()) {
			ERROR("TinyGLTF error: " + err);
			return false;
		}
		
		if (!ret) {
			ERROR("Failed to load glTF file: " + filename);
			return false;
		}
		
		return true;
	}

	bool processModelData() {
		try {
			processMaterials();
			processMeshes();
			processNodes();
			processSkins();
			processAnimations();
			calculateBoundingBox();
			return true;
		} 
		catch (const std::exception& e) {
			ERROR("Processing model data: " + std::string(e.what()));
			return false;
		}
	}

	// Material Processing
	void processMaterials() {
		materials.resize(gltfModel.materials.size());
		
		for (size_t i = 0; i < gltfModel.materials.size(); i++) {
			materials[i] = createMaterialFromGLTF(gltfModel.materials[i]);
		}
	}

	std::shared_ptr<Material> createMaterialFromGLTF(const tinygltf::Material& mat) {
		auto material = std::make_shared<Material>(defaultShader);
		
		setPBRProperties(material, mat);
		setMaterialFlags(material, mat);
		loadMaterialTextures(material, mat);
		
		return material;
	}

	void setPBRProperties(std::shared_ptr<Material> material, const tinygltf::Material& mat) {
		// Base color factor
		if (mat.pbrMetallicRoughness.baseColorFactor.size() >= 4) {
			material->setVector4("baseColorFactor", glm::vec4(
				mat.pbrMetallicRoughness.baseColorFactor[0],
				mat.pbrMetallicRoughness.baseColorFactor[1],
				mat.pbrMetallicRoughness.baseColorFactor[2],
				mat.pbrMetallicRoughness.baseColorFactor[3]
			));
		}
		
		// Metallic factor
		if (mat.pbrMetallicRoughness.metallicFactor >= 0.0) {
			material->setFloat("metallicFactor", mat.pbrMetallicRoughness.metallicFactor);
		}

		// Roughness factor
		if (mat.pbrMetallicRoughness.roughnessFactor >= 0.0) {
			material->setFloat("roughnessFactor", mat.pbrMetallicRoughness.roughnessFactor);
		}
		
		// Emissive factor
		if (mat.emissiveFactor.size() >= 3) {
			material->setVector3("emissiveFactor", glm::vec3(
				mat.emissiveFactor[0],
				mat.emissiveFactor[1],
				mat.emissiveFactor[2]
			));
		}
		
		// Normal scale
		if (mat.normalTexture.scale != 0.0) {
			material->setFloat("normalScale", mat.normalTexture.scale);
		}
	}

	void setMaterialFlags(std::shared_ptr<Material> material, const tinygltf::Material& mat) {
		// Alpha mode and cutoff
		if (isTransparent) {
			material->setAlphaMode("MASK");
		}
		else {
			material->setAlphaMode(mat.alphaMode);
		}
		
		bool doubleSided = false;
	
		if (isDoubleSided) {
			doubleSided = true;
		} 
		// DoubleSided flag usually not set and will always return true even if model isnt doubleSided
		// else if (mat.doubleSided) {
		//     doubleSided = true;
		// } 
		else {
			if (material->isTransparent()) {
				doubleSided = true;
			}
		}

		material->setDoubleSided(doubleSided);

		#ifdef DEBUG
		std::cout << "Material - Mode: " << mat.alphaMode 
				  << ", Alpha Cutoff: " << mat.alphaCutoff
				  << ", Double Sided: " << doubleSided 
				  << " (glTF flag: " << mat.doubleSided << ", override: " << isDoubleSided << ")" << std::endl;
		#endif
	}

	void loadMaterialTextures(std::shared_ptr<Material> material, const tinygltf::Material& mat) {
		loadTextureIfAvailable(material, "baseColorTexture", mat.pbrMetallicRoughness.baseColorTexture.index);
		loadTextureIfAvailable(material, "metallicRoughnessTexture", mat.pbrMetallicRoughness.metallicRoughnessTexture.index);
		loadTextureIfAvailable(material, "normalTexture", mat.normalTexture.index);
		loadTextureIfAvailable(material, "emissiveTexture", mat.emissiveTexture.index);
	}

	void loadTextureIfAvailable(std::shared_ptr<Material> material, const std::string& uniformName, int textureIndex) {
		if (textureIndex < 0 || textureIndex >= gltfModel.textures.size()) {
			return;
		}
		
		int srcIndex = gltfModel.textures[textureIndex].source;
		if (srcIndex < 0 || srcIndex >= gltfModel.images.size()) {
			return;
		}
		
		auto texture = loadTexture(gltfModel.images[srcIndex]);
		if (texture) {
			material->setTexture(uniformName, texture);
		}
	}
	
	std::shared_ptr<GLR::Texture> loadTexture(const tinygltf::Image& image) {
		if (image.image.empty()) {
			return nullptr;
		}
		
		GLenum format = GL_RGBA;
		switch (image.component) {
			case 1: format = GL_RED; break;
			case 2: format = GL_RG; break;
			case 3: format = GL_RGB; break;
			case 4: format = GL_RGBA; break;
		}

		auto dataCopy = std::make_unique<unsigned char[]>(image.image.size());
		std::memcpy(dataCopy.get(), image.image.data(), image.image.size());
		
		return std::make_shared<GLR::Texture>(image.width, image.height, format, dataCopy.release());
	}

	// Mesh Processing
	void processMeshes() {
		primitives.resize(gltfModel.meshes.size());
		
		for (size_t i = 0; i < gltfModel.meshes.size(); i++) {
			const tinygltf::Mesh& mesh = gltfModel.meshes[i];
			primitives[i].resize(mesh.primitives.size());
			
			for (size_t j = 0; j < mesh.primitives.size(); j++) {
				primitives[i][j] = processPrimitive(mesh.primitives[j]);
			}
		}
	}

	PrimitiveData processPrimitive(const tinygltf::Primitive& primitive) {
		PrimitiveData primData;
		
		// Get vertex count from position attribute
		size_t vertexCount = getVertexCount(primitive);
		if (vertexCount == 0) {
			return primData; // Empty primitive
		}
		
		createInterleavedVertexData(primitive, vertexCount, primData);
		processIndices(primitive, primData);
		setMaterial(primitive, primData);
		
		return primData;
	}

	size_t getVertexCount(const tinygltf::Primitive& primitive) {
		auto posIt = primitive.attributes.find("POSITION");
		if (posIt == primitive.attributes.end()) {
			return 0;
		}
		
		const tinygltf::Accessor& accessor = gltfModel.accessors[posIt->second];
		return accessor.count;
	}

	void createInterleavedVertexData(const tinygltf::Primitive& primitive, size_t vertexCount, PrimitiveData& primData) {
		auto layout = calculateVertexLayout();
		size_t stride = sizeof(float) * 22; // Pre calculated total size
		
		// Allocate vertex buffer
		primData.vertices.resize(vertexCount * stride);
		primData.attributes = layout;
		
		// Fill vertex data for each attribute
		fillVertexAttribute(primitive, "POSITION", 0, 3, vertexCount, stride, primData.vertices.data());
		fillVertexAttribute(primitive, "NORMAL", sizeof(float) * 3, 3, vertexCount, stride, primData.vertices.data());
		fillVertexAttribute(primitive, "TEXCOORD_0", sizeof(float) * 6, 2, vertexCount, stride, primData.vertices.data());
		fillVertexAttribute(primitive, "TANGENT", sizeof(float) * 8, 3, vertexCount, stride, primData.vertices.data());
		fillVertexAttribute(primitive, "JOINTS_0", sizeof(float) * 14, 4, vertexCount, stride, primData.vertices.data());
		fillVertexAttribute(primitive, "WEIGHTS_0", sizeof(float) * 18, 4, vertexCount, stride, primData.vertices.data());
		
		// Calculate bitangents
		calculateBitangents(primitive, vertexCount, stride, primData.vertices.data());
	}

	std::vector<VertexAttribute> calculateVertexLayout() {
		return {
			{0, 3, GL_FLOAT, sizeof(float) * 22, 0},                    // Position
			{1, 3, GL_FLOAT, sizeof(float) * 22, sizeof(float) * 3},    // Normal
			{2, 2, GL_FLOAT, sizeof(float) * 22, sizeof(float) * 6},    // TexCoord
			{3, 3, GL_FLOAT, sizeof(float) * 22, sizeof(float) * 8},    // Tangent
			{4, 3, GL_FLOAT, sizeof(float) * 22, sizeof(float) * 11},   // Bitangent
			{5, 4, GL_FLOAT, sizeof(float) * 22, sizeof(float) * 14},   // Joints
			{6, 4, GL_FLOAT, sizeof(float) * 22, sizeof(float) * 18}    // Weights
		};
	}

	void fillVertexAttribute(const tinygltf::Primitive& primitive, const std::string& attributeName, size_t offset, int componentCount, size_t vertexCount, size_t stride, uint8_t* vertexData) {
		auto it = primitive.attributes.find(attributeName);
		if (it == primitive.attributes.end()) {
			// Fill with default values
			fillDefaultAttribute(attributeName, offset, componentCount, vertexCount, stride, vertexData);
			return;
		}
		
		// Extract data from GLTF buffer
		const tinygltf::Accessor& accessor = gltfModel.accessors[it->second];
		const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
		
		const uint8_t* srcData = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
		
		// Copy data with proper handling for different types
		copyAttributeData(srcData, accessor, attributeName, offset, componentCount, vertexCount, stride, vertexData);
	}

	void fillDefaultAttribute(const std::string& attributeName, size_t offset, int componentCount, size_t vertexCount, size_t stride, uint8_t* vertexData) {
		std::vector<float> defaultValues;
		if (attributeName == "NORMAL") {
			defaultValues = {0.0f, 1.0f, 0.0f};
		} 
		else if (attributeName == "TANGENT") {
			defaultValues = {0.0f, 0.0f, 1.0f};
		} 
		else if (attributeName == "WEIGHTS_0") {
			defaultValues = {1.0f, 0.0f, 0.0f, 0.0f};
		} 
		else {
			defaultValues.resize(componentCount, 0.0f);
		}
		
		for (size_t v = 0; v < vertexCount; v++) {
			float* dst = reinterpret_cast<float*>(vertexData + v * stride + offset);
			for (int c = 0; c < componentCount; c++) {
				dst[c] = (c < defaultValues.size()) ? defaultValues[c] : 0.0f;
			}
		}
	}

	void copyAttributeData(const uint8_t* srcData, const tinygltf::Accessor& accessor, const std::string& attributeName, size_t offset, int componentCount, size_t vertexCount, size_t stride, uint8_t* vertexData) {
		for (size_t v = 0; v < vertexCount; v++) {
			float* dst = reinterpret_cast<float*>(vertexData + v * stride + offset);
			
			if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				const float* src = reinterpret_cast<const float*>(srcData + v * componentCount * sizeof(float));
				for (int c = 0; c < componentCount; c++) {
					dst[c] = src[c];
				}
				
				// Special handling for texture coordinates
				if (attributeName == "TEXCOORD_0" && componentCount >= 2) {
					// dst[1] = 1.0f - dst[1]; // Uncomment if textures appear flipped
				}
			} 
			else if (attributeName == "JOINTS_0") {
				copyJointIndices(srcData, accessor, v, dst, componentCount);
			}
		}
	}

	void copyJointIndices(const uint8_t* srcData, const tinygltf::Accessor& accessor, size_t vertexIndex, float* dst, int componentCount) {
		if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
			const uint8_t* src = srcData + vertexIndex * componentCount;
			for (int c = 0; c < componentCount; c++) {
				dst[c] = static_cast<float>(src[c]);
			}
		} 
		else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
			const uint16_t* src = reinterpret_cast<const uint16_t*>(srcData) + vertexIndex * componentCount;
			for (int c = 0; c < componentCount; c++) {
				dst[c] = static_cast<float>(src[c]);
			}
		}
	}

	void calculateBitangents(const tinygltf::Primitive& primitive, size_t vertexCount, size_t stride, uint8_t* vertexData) {
		// Only calculate if both normal and tangent are present
		if (primitive.attributes.find("NORMAL") == primitive.attributes.end() ||
			primitive.attributes.find("TANGENT") == primitive.attributes.end()) {
			// Fill with default bitangent values
			for (size_t v = 0; v < vertexCount; v++) {
				float* dst = reinterpret_cast<float*>(vertexData + v * stride + sizeof(float) * 11);
				dst[0] = 0.0f; dst[1] = 1.0f; dst[2] = 0.0f;
			}
			return;
		}
		
		// Calculate bitangents from normals and tangents
		for (size_t v = 0; v < vertexCount; v++) {
			float* normal = reinterpret_cast<float*>(vertexData + v * stride + sizeof(float) * 3);
			float* tangent = reinterpret_cast<float*>(vertexData + v * stride + sizeof(float) * 8);
			float* bitangent = reinterpret_cast<float*>(vertexData + v * stride + sizeof(float) * 11);
			
			// Cross product
			bitangent[0] = normal[1] * tangent[2] - normal[2] * tangent[1];
			bitangent[1] = normal[2] * tangent[0] - normal[0] * tangent[2];
			bitangent[2] = normal[0] * tangent[1] - normal[1] * tangent[0];
		}
	}

	void processIndices(const tinygltf::Primitive& primitive, PrimitiveData& primData) {
		if (primitive.indices < 0) {
			return; // No indices
		}
		
		const tinygltf::Accessor& accessor = gltfModel.accessors[primitive.indices];
		const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
		
		const uint8_t* bufferData = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
		primData.indices.resize(accessor.count);
		
		// Convert indices based on component type
		switch (accessor.componentType) {
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
				for (size_t i = 0; i < accessor.count; i++) {
					primData.indices[i] = static_cast<GLuint>(bufferData[i]);
				}
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				for (size_t i = 0; i < accessor.count; i++) {
					primData.indices[i] = static_cast<GLuint>(reinterpret_cast<const uint16_t*>(bufferData)[i]);
				}
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
				std::memcpy(primData.indices.data(), bufferData, accessor.count * sizeof(GLuint));
				break;
		}
	}

	void setMaterial(const tinygltf::Primitive& primitive, PrimitiveData& primData) {
		if (primitive.material >= 0 && primitive.material < materials.size()) {
			primData.material = materials[primitive.material];
		} 
		else {
			ERROR("Set material failed.");
		}
	}

	// Node Processing  
	void processNodes() {
		createNodeObjects();
		setupNodeHierarchy();
		attachMeshesToNodes();
		applyCoordinateSystemConversion();
	}

	void createNodeObjects() {
		nodes.resize(gltfModel.nodes.size());
		
		for (size_t i = 0; i < gltfModel.nodes.size(); i++) {
			nodes[i] = std::make_shared<Node>();
			nodes[i]->setIndex(i);
			
			const std::string& name = gltfModel.nodes[i].name;
			nodes[i]->setName(name.empty() ? "node_" + std::to_string(i) : name);
		}
	}

	void setupNodeHierarchy() {
		for (size_t i = 0; i < gltfModel.nodes.size(); i++) {
			const tinygltf::Node& gltfNode = gltfModel.nodes[i];
			std::shared_ptr<Node>& currentNode = nodes[i];
			
			setNodeTransformation(currentNode, gltfNode);
			
			// Set up children
			for (int childIdx : gltfNode.children) {
				if (childIdx >= 0 && childIdx < nodes.size()) {
					currentNode->addChild(nodes[childIdx]);
				}
			}
		}
	}

	void setNodeTransformation(std::shared_ptr<Node> node, const tinygltf::Node& gltfNode) {
		if (gltfNode.matrix.size() == 16) {
			// Use matrix directly
			glm::mat4 mat(1.0f);
			for (int row = 0; row < 4; row++) {
				for (int col = 0; col < 4; col++) {
					mat[col][row] = gltfNode.matrix[row * 4 + col];
				}
			}
			node->setMatrix(mat);
		} 
		else {
			setNodeTRS(node, gltfNode);
		}
	}

	void setNodeTRS(std::shared_ptr<Node> node, const tinygltf::Node& gltfNode) {
		// Translation
		glm::vec3 translation(0.0f);
		if (gltfNode.translation.size() == 3) {
			translation = glm::vec3(gltfNode.translation[0], gltfNode.translation[1], gltfNode.translation[2]);
		}
		
		// Rotation
		glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
		if (gltfNode.rotation.size() == 4) {
			rotation = glm::quat(gltfNode.rotation[3], gltfNode.rotation[0], gltfNode.rotation[1], gltfNode.rotation[2]);
		}
		
		// Scale
		glm::vec3 scale(1.0f);
		if (gltfNode.scale.size() == 3) {
			scale = glm::vec3(gltfNode.scale[0], gltfNode.scale[1], gltfNode.scale[2]);
		}
		
		node->setPosition(translation);
		node->setQuaternion(rotation);
		node->setScale(scale);
	}

	void attachMeshesToNodes() {
		std::vector<std::shared_ptr<Node>> additionalNodes;
		
		for (size_t i = 0; i < gltfModel.nodes.size(); i++) {
			const tinygltf::Node& gltfNode = gltfModel.nodes[i];
			
			if (gltfNode.mesh >= 0 && gltfNode.mesh < primitives.size()) {
				attachMeshToNode(nodes[i], gltfNode.mesh, additionalNodes);
			}
		}
		
		// Add additional nodes created for multiple primitives
		nodes.insert(nodes.end(), additionalNodes.begin(), additionalNodes.end());
	}

	void attachMeshToNode(std::shared_ptr<Node> node, int meshIndex, std::vector<std::shared_ptr<Node>>& additionalNodes) {
		const auto& primitivesData = primitives[meshIndex];
		
		if (primitivesData.empty()) {
			return;
		}
		
		if (primitivesData.size() == 1) {
			attachSinglePrimitive(node, primitivesData[0]);
		} 
		else {
			attachMultiplePrimitives(node, primitivesData, additionalNodes);
		}
	}

	void attachSinglePrimitive(std::shared_ptr<Node> node, const PrimitiveData& primData) {
		auto mesh = std::make_shared<Mesh>();
		mesh->create(primData.vertices, primData.indices, primData.attributes);
		node->setMesh(mesh);
		node->setMaterial(primData.material);
	}

	void attachMultiplePrimitives(std::shared_ptr<Node> node, const std::vector<PrimitiveData>& primitivesData, std::vector<std::shared_ptr<Node>>& additionalNodes) {
		for (size_t primIdx = 0; primIdx < primitivesData.size(); primIdx++) {
			auto primitiveNode = std::make_shared<Node>();
			primitiveNode->setName(node->getName() + "_primitive_" + std::to_string(primIdx));
			primitiveNode->setIndex(nodes.size() + additionalNodes.size());
			
			// Identity transform relative to parent
			primitiveNode->setPosition(glm::vec3(0.0f));
			primitiveNode->setQuaternion(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
			primitiveNode->setScale(glm::vec3(1.0f));
			
			// Attach primitive
			attachSinglePrimitive(primitiveNode, primitivesData[primIdx]);
			
			// Add as child
			node->addChild(primitiveNode);
			additionalNodes.push_back(primitiveNode);
		}
	}

	void applyCoordinateSystemConversion() {
		// Apply coordinate system conversion to root nodes only
		glm::mat4 conversionMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		
		for (auto& node : nodes) {
			if (!node->getParent().lock()) {
				applyConversionToNode(node, conversionMatrix);
			}
		}
	}

	void applyConversionToNode(std::shared_ptr<Node> node, const glm::mat4& conversionMatrix) {
		glm::vec3 pos = node->getPosition();
		glm::quat rot = node->getQuaternion();
		glm::vec3 scl = node->getScale();
		
		// Apply rotation to position and quaternion
		node->setPosition(glm::vec3(conversionMatrix * glm::vec4(pos, 1.0f)));
		
		glm::quat conversionQuat = glm::quat_cast(glm::mat3(conversionMatrix));
		node->setQuaternion(conversionQuat * rot);
		node->setScale(scl); // Scale remains unchanged
		
		// Update euler angles and matrix
		node->setEulerRotation(glm::eulerAngles(node->getQuaternion()));
		node->updateMatrix();
	}

	// Skin Processing
	void processSkins() {
		if (gltfModel.skins.empty()) {
			return;
		}
		
		skins.reserve(gltfModel.skins.size());
		
		for (size_t i = 0; i < gltfModel.skins.size(); i++) {
			skins.push_back(createSkinFromGLTF(gltfModel.skins[i], i));
		}
	}

	std::shared_ptr<Skin> createSkinFromGLTF(const tinygltf::Skin& gltfSkin, size_t index) {
		auto skin = std::make_shared<Skin>();
		
		skin->name = gltfSkin.name.empty() ? "Skin_" + std::to_string(index) : gltfSkin.name;
		
		processJoints(gltfSkin, skin);
		processInverseBindMatrices(gltfSkin, skin);
		setSkeletonRoot(gltfSkin, skin);
		
		return skin;
	}

	void processJoints(const tinygltf::Skin& gltfSkin, std::shared_ptr<Skin> skin) {
		for (int jointNodeIdx : gltfSkin.joints) {
			if (jointNodeIdx >= 0 && jointNodeIdx < gltfModel.nodes.size()) {
				const auto& node = gltfModel.nodes[jointNodeIdx];
				std::string jointName = node.name.empty() ? "Node_" + std::to_string(jointNodeIdx) : node.name;
				
				skin->jointNodeNames.push_back(jointName);
				
				// Resolve joint reference
				if (jointNodeIdx < nodes.size()) {
					skin->joints.push_back(nodes[jointNodeIdx]);
				} 
				else {
					ERROR("Joint node index out of range: " + std::to_string(jointNodeIdx));
					skin->joints.push_back(std::weak_ptr<Node>());
				}
			}
		}
	}

	void processInverseBindMatrices(const tinygltf::Skin& gltfSkin, std::shared_ptr<Skin> skin) {
		if (gltfSkin.inverseBindMatrices < 0) {
			return;
		}
		
		const auto& accessor = gltfModel.accessors[gltfSkin.inverseBindMatrices];
		const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
		const auto& buffer = gltfModel.buffers[bufferView.buffer];
		
		const float* matrices = reinterpret_cast<const float*>(
			&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
		
		for (size_t i = 0; i < accessor.count; i++) {
			glm::mat4 inverseBindMatrix;
			memcpy(&inverseBindMatrix, matrices + i * 16, sizeof(glm::mat4));
			skin->inverseBindMatrices.push_back(inverseBindMatrix);
		}
	}

	void setSkeletonRoot(const tinygltf::Skin& gltfSkin, std::shared_ptr<Skin> skin) {
		if (gltfSkin.skeleton >= 0 && gltfSkin.skeleton < nodes.size()) {
			skin->skeletonRoot = nodes[gltfSkin.skeleton];
		}
	}

	// Animation Processing
	void processAnimations() {
		if (gltfModel.animations.empty()) {
			return;
		}

		std::vector<Animation> animations;
		animations.reserve(gltfModel.animations.size());
		
		for (size_t i = 0; i < gltfModel.animations.size(); i++) {
			animations.push_back(createAnimationFromGLTF(gltfModel.animations[i], i));
		}
		
		// Initialize animation manager
		if (!animations.empty()) {
			animationManager = std::make_shared<AnimationManager>();
			animationManager->loadAnimations(animations, nodes);
		}
	}

	Animation createAnimationFromGLTF(const tinygltf::Animation& gltfAnimation, size_t index) {
		Animation animation;
		animation.name = gltfAnimation.name.empty() ? "Animation_" + std::to_string(index) : gltfAnimation.name;
		
		// Process channels
		for (const auto& gltfChannel : gltfAnimation.channels) {
			auto channel = createAnimationChannel(gltfChannel, gltfAnimation);
			if (channel.has_value()) {
				animation.channels.push_back(channel.value());
			}
		}
		
		animation.calculateDuration();
		return animation;
	}

	std::optional<AnimationChannel> createAnimationChannel(const tinygltf::AnimationChannel& gltfChannel, const tinygltf::Animation& gltfAnimation) {
		// Validate channel
		if (gltfChannel.target_node < 0 || gltfChannel.sampler < 0 ||
			gltfChannel.sampler >= gltfAnimation.samplers.size() ||
			gltfChannel.target_node >= gltfModel.nodes.size()) {
			return std::nullopt;
		}
		
		AnimationChannel channel;
		
		const auto& targetNode = gltfModel.nodes[gltfChannel.target_node];
		channel.targetNodeName = targetNode.name.empty() ? "Node_" + std::to_string(gltfChannel.target_node) : targetNode.name;
		
		if (!setAnimationPathType(channel, gltfChannel.target_path)) {
			return std::nullopt;
		}
		
		const auto& gltfSampler = gltfAnimation.samplers[gltfChannel.sampler];
		setInterpolationType(channel, gltfSampler.interpolation);
		
		if (!processKeyframeData(channel, gltfSampler)) {
			return std::nullopt;
		}
		
		return channel;
	}

	bool setAnimationPathType(AnimationChannel& channel, const std::string& targetPath) {
		if (targetPath == "translation") {
			channel.pathType = AnimationPathType::TRANSLATION;
		} 
		else if (targetPath == "rotation") {
			channel.pathType = AnimationPathType::ROTATION;
		} 
		else if (targetPath == "scale") {
			channel.pathType = AnimationPathType::SCALE;
		} 
		else if (targetPath == "weights") {
			channel.pathType = AnimationPathType::WEIGHTS;
		} 
		else {
			ERROR("Model '" + filename + "' has unknown path type: " + targetPath);
			return false;
		}
		return true;
	}

	void setInterpolationType(AnimationChannel& channel, const std::string& interpolation) {
		if (interpolation == "LINEAR") {
			channel.interpolation = InterpolationType::LINEAR;
		} 
		else if (interpolation == "STEP") {
			channel.interpolation = InterpolationType::STEP;
		} 
		else if (interpolation == "CUBICSPLINE") {
			channel.interpolation = InterpolationType::CUBICSPLINE;
		} 
		else {
			channel.interpolation = InterpolationType::LINEAR; // Default
		}
	}

	bool processKeyframeData(AnimationChannel& channel, const tinygltf::AnimationSampler& gltfSampler) {
		// Get input times
		std::vector<float> times;
		if (!extractKeyframeTimes(gltfSampler.input, times)) {
			return false;
		}
		
		// Get output values
		return extractKeyframeValues(gltfSampler.output, times, channel);
	}

	bool extractKeyframeTimes(int inputAccessorIndex, std::vector<float>& times) {
		if (inputAccessorIndex < 0 || inputAccessorIndex >= gltfModel.accessors.size()) {
			return false;
		}
		
		const auto& accessor = gltfModel.accessors[inputAccessorIndex];
		const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
		const auto& buffer = gltfModel.buffers[bufferView.buffer];
		
		const float* timeData = reinterpret_cast<const float*>(
			&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
		
		times.resize(accessor.count);
		for (size_t i = 0; i < accessor.count; i++) {
			times[i] = timeData[i];
		}
		
		return true;
	}

	bool extractKeyframeValues(int outputAccessorIndex, const std::vector<float>& times, AnimationChannel& channel) {
		if (outputAccessorIndex < 0 || outputAccessorIndex >= gltfModel.accessors.size()) {
			return false;
		}
		
		const auto& accessor = gltfModel.accessors[outputAccessorIndex];
		const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
		const auto& buffer = gltfModel.buffers[bufferView.buffer];
		
		const void* outputData = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
		
		switch (channel.pathType) {
			case AnimationPathType::TRANSLATION:
			case AnimationPathType::SCALE:
				return extractVec3Keys(outputData, times, channel);
			case AnimationPathType::ROTATION:
				return extractQuatKeys(outputData, times, channel);
			case AnimationPathType::WEIGHTS:
				return extractWeightKeys(outputData, times, channel, accessor);
		}
		
		return false;
	}

	bool extractVec3Keys(const void* outputData, const std::vector<float>& times, AnimationChannel& channel) {
		const glm::vec3* vectors = reinterpret_cast<const glm::vec3*>(outputData);
		
		for (size_t i = 0; i < times.size(); i++) {
			if (channel.pathType == AnimationPathType::TRANSLATION) {
				channel.translationKeys.push_back({times[i], vectors[i]});
			} 
			else {
				channel.scaleKeys.push_back({times[i], vectors[i]});
			}
		}
		
		return true;
	}

	bool extractQuatKeys(const void* outputData, const std::vector<float>& times, AnimationChannel& channel) {
		const glm::vec4* quats = reinterpret_cast<const glm::vec4*>(outputData);
		
		for (size_t i = 0; i < times.size(); i++) {
			// glTF uses [x,y,z,w] quaternion layout
			glm::quat q(quats[i].w, quats[i].x, quats[i].y, quats[i].z);
			channel.rotationKeys.push_back({times[i], q});
		}
		
		return true;
	}

	bool extractWeightKeys(const void* outputData, const std::vector<float>& times, AnimationChannel& channel, const tinygltf::Accessor& accessor) {
		if (accessor.type != TINYGLTF_TYPE_SCALAR) {
			return false;
		}
		
		const float* weights = reinterpret_cast<const float*>(outputData);
		
		for (size_t i = 0; i < times.size(); i++) {
			std::vector<float> weightVec = {weights[i]};
			channel.weightKeys.push_back({times[i], weightVec});
		}
		
		return true;
	}

	// bounding box generating
	void calculateBoundingBox() {
		boundingBox.reset();
		
		for (const auto& meshPrimitives : primitives) {
			for (const auto& primitive : meshPrimitives) {
				BoundingBox primitiveBounds;
				auto layout = calculateVertexLayout();
				primitiveBounds.calculateFromVertices(primitive.vertices, layout);
				
				if (primitiveBounds.isValid()) {
					if (!boundingBox.isValid()) {
						boundingBox = primitiveBounds;
					} else {
						boundingBox.expandToInclude(primitiveBounds.getMinExtents());
						boundingBox.expandToInclude(primitiveBounds.getMaxExtents());
					}
				}
			}
		}
		
		if (!boundingBox.isValid()) {
			boundingBox.setMinMax(glm::vec3(-1.0f), glm::vec3(1.0f));
		}
	}

	// Debug Print
	void printNodeHierarchy() const {
		std::cout << "\n===== MODEL HIERARCHY DETAILS =====\n";
		std::cout << "Model: " << filename << std::endl;

		if (animationManager) {
			printAnimationSummary();
		}
		printSkinSummary();
		printNodeTree();
		
		std::cout << "\n===== END MODEL DETAILS =====\n" << std::endl;
	}

	void printAnimationSummary() const {
		std::cout << "\n--- ANIMATIONS (" << animationManager->getAnimations().size() << ") ---" << std::endl;
		
		for (size_t i = 0; i < animationManager->getAnimations().size(); i++) {
			const auto& animation = animationManager->getAnimations()[i];
			std::cout << "  Animation " << i << ": \"" << animation.name << "\"" << std::endl;
			std::cout << "    Duration: " << animation.duration << " seconds" << std::endl;
			std::cout << "    Channels: " << animation.channels.size() << std::endl;
			
			// Print limited channel info
			const size_t maxChannels = std::min(size_t(5), animation.channels.size());
			for (size_t j = 0; j < maxChannels; j++) {
				printChannelInfo(animation.channels[j], j);
			}
			
			if (animation.channels.size() > maxChannels) {
				std::cout << "      ... and " << (animation.channels.size() - maxChannels) 
						 << " more channels" << std::endl;
			}
		}
	}

	void printChannelInfo(const AnimationChannel& channel, size_t index) const {
		std::cout << "      Channel " << index << ": Target=\"" << channel.targetNodeName << "\", ";
		
		// Print path type and key count
		std::cout << "Path=";
		switch (channel.pathType) {
			case AnimationPathType::TRANSLATION:
				std::cout << "Translation (Keys: " << channel.translationKeys.size() << ")";
				break;
			case AnimationPathType::ROTATION:
				std::cout << "Rotation (Keys: " << channel.rotationKeys.size() << ")";
				break;
			case AnimationPathType::SCALE:
				std::cout << "Scale (Keys: " << channel.scaleKeys.size() << ")";
				break;
			case AnimationPathType::WEIGHTS:
				std::cout << "Weights (Keys: " << channel.weightKeys.size() << ")";
				break;
		}
		
		// Print interpolation type
		std::cout << ", Interp=";
		switch (channel.interpolation) {
			case InterpolationType::LINEAR: std::cout << "Linear"; break;
			case InterpolationType::STEP: std::cout << "Step"; break;
			case InterpolationType::CUBICSPLINE: std::cout << "CubicSpline"; break;
		}
		std::cout << std::endl;
	}

	void printSkinSummary() const {
		std::cout << "\n--- SKINS (" << skins.size() << ") ---" << std::endl;
		
		for (size_t i = 0; i < skins.size(); i++) {
			const auto& skin = skins[i];
			std::cout << "  Skin " << i << ": \"" << skin->name << "\"" << std::endl;
			std::cout << "    Joints: " << skin->joints.size() << std::endl;
			
			// Print limited joint info
			const size_t maxJoints = std::min(size_t(10), skin->joints.size());
			std::cout << "    Joint List:" << std::endl;
			for (size_t j = 0; j < maxJoints; j++) {
				printJointInfo(skin, j);
			}
			
			if (skin->joints.size() > maxJoints) {
				std::cout << "      ... and " << (skin->joints.size() - maxJoints) 
						 << " more joints" << std::endl;
			}
		}
	}

	void printJointInfo(const std::shared_ptr<Skin>& skin, size_t index) const {
		if (auto joint = skin->joints[index].lock()) {
			std::cout << "      Joint " << index << ": \"" << joint->getName() 
					 << "\" (Node Index: " << joint->getIndex() << ")";
			
			if (index < skin->inverseBindMatrices.size()) {
				const auto& ibm = skin->inverseBindMatrices[index];
				std::cout << " IBM[0]: [" << ibm[0][0] << ", " << ibm[1][0] 
						 << ", " << ibm[2][0] << ", " << ibm[3][0] << "]";
			}
			std::cout << std::endl;
		} else {
			std::cout << "      Joint " << index << ": <Invalid Reference>" << std::endl;
		}
	}

	void printNodeTree() const {
		std::cout << "\n--- NODE HIERARCHY ---" << std::endl;
		auto roots = getRootNodes();
		
		for (auto& root : roots) {
			printNodeRecursive(root, 0);
		}
	}

	void printNodeRecursive(std::shared_ptr<Node> node, int depth) const {
		std::string indent(depth * 2, ' ');
		std::cout << indent << "- Node: \"" << node->getName() 
				 << "\" (Index: " << node->getIndex() << ")";
		
		// Print node properties
		if (node->getMesh()) std::cout << " [Has Mesh]";
		if (isJointNode(node)) std::cout << " [Is Joint]";
		
		auto animatedProps = getAnimatedProperties(node);
		if (!animatedProps.empty()) {
			std::cout << " [Animated: ";
			for (size_t i = 0; i < animatedProps.size(); i++) {
				if (i > 0) std::cout << ", ";
				std::cout << animatedProps[i];
			}
			std::cout << "]";
		}
		
		std::cout << std::endl;
		
		// Print transformation
		printNodeTransformation(node, indent);
		
		// Print children
		if (!node->getChildren().empty()) {
			std::cout << indent << "  Children (" << node->getChildren().size() << "):" << std::endl;
			for (auto& child : node->getChildren()) {
				printNodeRecursive(child, depth + 1);
			}
		}
	}

	bool isJointNode(std::shared_ptr<Node> node) const {
		for (const auto& skin : skins) {
			for (const auto& jointWeakPtr : skin->joints) {
				if (auto joint = jointWeakPtr.lock()) {
					if (joint == node) return true;
				}
			}
		}
		return false;
	}

	std::vector<std::string> getAnimatedProperties(std::shared_ptr<Node> node) const {
		std::vector<std::string> properties;
		if (animationManager) {
			for (const auto& animation : animationManager->getAnimations()) {
				for (const auto& channel : animation.channels) {
					if (channel.targetNodeName == node->getName()) {
						switch (channel.pathType) {
							case AnimationPathType::TRANSLATION:
								properties.push_back("translation");
								break;
							case AnimationPathType::ROTATION:
								properties.push_back("rotation");
								break;
							case AnimationPathType::SCALE:
								properties.push_back("scale");
								break;
							case AnimationPathType::WEIGHTS:
								properties.push_back("weights");
								break;
						}
					}
				}
			}
		}
		
		return properties;
	}

	void printNodeTransformation(std::shared_ptr<Node> node, const std::string& indent) const {
		const glm::vec3& pos = node->getPosition();
		const glm::quat& rot = node->getQuaternion();
		const glm::vec3& scale = node->getScale();
		
		std::cout << indent << "  Position: [" << pos.x << ", " << pos.y << ", " << pos.z << "]" << std::endl;
		std::cout << indent << "  Rotation(quat): [" << rot.x << ", " << rot.y << ", " << rot.z << ", " << rot.w << "]" << std::endl;
		std::cout << indent << "  Scale: [" << scale.x << ", " << scale.y << ", " << scale.z << "]" << std::endl;
	}
};

class Shape {
public:
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoord;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

	// Shape generation methods
	static std::vector<uint8_t> generateCube(float size = 1.0f) {
		return verticesToRawData(generateCubeVertices(size));
	}

	static std::vector<uint8_t> generateSphere(float radius = 1.0f, unsigned int segments = 32) {
		return verticesToRawData(generateSphereVertices(radius, segments));
	}

	static std::vector<uint8_t> generateCylinder(float radius = 1.0f, float height = 2.0f, unsigned int segments = 32) {
		return verticesToRawData(generateCylinderVertices(radius, height, segments));
	}

	static std::vector<uint8_t> generatePlane(float width = 2.0f, float height = 2.0f, unsigned int widthSegments = 1, unsigned int heightSegments = 1, float uRepeat = 1.0f, float vRepeat = 1.0f) {
		return verticesToRawData(generatePlaneVertices(width, height, widthSegments, heightSegments, uRepeat, vRepeat));
	}

	static std::vector<uint8_t> generateCapsule(float radius = 0.5f, float height = 2.0f, unsigned int segments = 32) {
		return verticesToRawData(generateCapsuleVertices(radius, height, segments));
	}

	// Index buffer generation methods
	static std::vector<unsigned int> getCubeIndices() {
		return {
			// Front face
			0, 1, 2, 2, 3, 0,
			// Back face (-Z) 
			4, 5, 6, 6, 7, 4,
			// Top face (+Y)
			8, 9, 10, 10, 11, 8,
			// Bottom face (-Y)
			12, 13, 14, 14, 15, 12,
			// Right face (+X)
			16, 17, 18, 18, 19, 16,
			// Left face (-X)
			20, 21, 22, 22, 23, 20
		};
	}

	static std::vector<unsigned int> getSphereIndices(unsigned int segments) {
		std::vector<unsigned int> indices;
		
		for (unsigned int y = 0; y < segments; y++) {
			for (unsigned int x = 0; x < segments; x++) {
				unsigned int first = (y * (segments + 1)) + x;
				unsigned int second = first + segments + 1;
				
				indices.push_back(first);
				indices.push_back(first + 1);
				indices.push_back(second);
				
				indices.push_back(second);
				indices.push_back(first + 1);
				indices.push_back(second + 1);
			}
		}
		
		return indices;
	}

	static std::vector<unsigned int> getCylinderIndices(unsigned int segments) {
		std::vector<unsigned int> indices;
		
		unsigned int topCenterIndex = 0;
		unsigned int bottomCenterIndex = 1;
		
		unsigned int firstTopRim = 2;
		unsigned int firstBottomRim = 3;
		unsigned int firstSide = 4;
		
		// Top face indices
		for (unsigned int i = 0; i < segments; i++) {
			indices.push_back(topCenterIndex);
			indices.push_back(firstTopRim + (i * 4));
			indices.push_back(firstTopRim + (((i + 1) % segments) * 4));
		}
		
		// Bottom face indices
		for (unsigned int i = 0; i < segments; i++) {
			indices.push_back(bottomCenterIndex);
			indices.push_back(firstBottomRim + (((i + 1) % segments) * 4));
			indices.push_back(firstBottomRim + (i * 4));
		}
		
		// Side faces indices
		for (unsigned int i = 0; i < segments; i++) {
			unsigned int tl = firstSide + (i * 4); 
			unsigned int bl = firstSide + (i * 4) + 1; 
			unsigned int tr = firstSide + (((i + 1) % segments) * 4); 
			unsigned int br = firstSide + (((i + 1) % segments) * 4) + 1; 
			
			// First triangle
			indices.push_back(tl);
			indices.push_back(bl);
			indices.push_back(br);
			
			// Second triangle
			indices.push_back(tl);
			indices.push_back(br);
			indices.push_back(tr);
		}
		
		return indices;
	}

	static std::vector<unsigned int> getPlaneIndices(unsigned int widthSegments, unsigned int heightSegments) {
		std::vector<unsigned int> indices;
		
		for (unsigned int y = 0; y < heightSegments; y++) {
			for (unsigned int x = 0; x < widthSegments; x++) {
				unsigned int a = x + (widthSegments + 1) * y;
				unsigned int b = x + (widthSegments + 1) * (y + 1);
				unsigned int c = (x + 1) + (widthSegments + 1) * (y + 1);
				unsigned int d = (x + 1) + (widthSegments + 1) * y;
				
				// First triangle
				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(d);
				
				// Second triangle
				indices.push_back(b);
				indices.push_back(c);
				indices.push_back(d);
			}
		}
		
		return indices;
	}

	static std::vector<unsigned int> getCapsuleIndices(unsigned int segments) {
		std::vector<unsigned int> indices;
		
		unsigned int hemisphereRings = segments / 2;
		unsigned int topHemisphereVertexCount = (hemisphereRings + 1) * (segments + 1);
		
		// Top hemisphere indices
		for (unsigned int y = 0; y < hemisphereRings; y++) {
			for (unsigned int x = 0; x < segments; x++) {
				unsigned int current = y * (segments + 1) + x;
				unsigned int next = current + segments + 1;
				
				// First triangle
				indices.push_back(current);
				indices.push_back(next);
				indices.push_back(current + 1);
				
				// Second triangle
				indices.push_back(next);
				indices.push_back(next + 1);
				indices.push_back(current + 1);
			}
		}
		
		// Cylinder indices
		unsigned int cylinderStart = topHemisphereVertexCount;
		for (unsigned int i = 0; i < segments; i++) {
			unsigned int tl = cylinderStart + i * 2;
			unsigned int bl = cylinderStart + i * 2 + 1;
			unsigned int tr = cylinderStart + ((i + 1) % segments) * 2;
			unsigned int br = cylinderStart + ((i + 1) % segments) * 2 + 1;

			// First triangle
			indices.push_back(tl);
			indices.push_back(bl);
			indices.push_back(br);
			
			// Second triangle
			indices.push_back(tl);
			indices.push_back(br);
			indices.push_back(tr);
		}
		
		// Bottom hemisphere indices
		unsigned int cylinderVertexCount = 2 * (segments + 1);
		unsigned int bottomHemisphereStart = cylinderStart + cylinderVertexCount;
		for (unsigned int y = 0; y < hemisphereRings; y++) {
			for (unsigned int x = 0; x < segments; x++) {
				unsigned int current = bottomHemisphereStart + y * (segments + 1) + x;
				unsigned int next = current + segments + 1;
				
				// First triangle
				indices.push_back(current);
				indices.push_back(next);
				indices.push_back(current + 1);
				
				// Second triangle
				indices.push_back(next);
				indices.push_back(next + 1);
				indices.push_back(current + 1);
			}
		}
		
		return indices;
	}

	static std::vector<GLR::VertexAttribute> getStandardLayout() {
		return {
			{ 0, 3, GL_FLOAT, sizeof(Shape::Vertex), (size_t)offsetof(Shape::Vertex, position) },   // position
			{ 1, 3, GL_FLOAT, sizeof(Shape::Vertex), (size_t)offsetof(Shape::Vertex, normal) },     // normal
			{ 2, 2, GL_FLOAT, sizeof(Shape::Vertex), (size_t)offsetof(Shape::Vertex, texCoord) },   // texCoord
			{ 3, 3, GL_FLOAT, sizeof(Shape::Vertex), (size_t)offsetof(Shape::Vertex, tangent) },    // tangent
			{ 4, 3, GL_FLOAT, sizeof(Shape::Vertex), (size_t)offsetof(Shape::Vertex, bitangent) }   // bitangent
		};
	}

private:
	static std::vector<Shape::Vertex> generateCubeVertices(float size) {
		float halfSize = size / 2.0f;
		
		std::vector<Shape::Vertex> vertices = {
			// Front face
			{{-halfSize, -halfSize,  halfSize}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{ halfSize, -halfSize,  halfSize}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{ halfSize,  halfSize,  halfSize}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{-halfSize,  halfSize,  halfSize}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			
			// Back face
			{{ halfSize, -halfSize, -halfSize}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{-halfSize, -halfSize, -halfSize}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{-halfSize,  halfSize, -halfSize}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{ halfSize,  halfSize, -halfSize}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			
			// Top face
			{{-halfSize,  halfSize, -halfSize}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
			{{-halfSize,  halfSize,  halfSize}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
			{{ halfSize,  halfSize,  halfSize}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
			{{ halfSize,  halfSize, -halfSize}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
			
			// Bottom face
			{{-halfSize, -halfSize,  halfSize}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			{{-halfSize, -halfSize, -halfSize}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			{{ halfSize, -halfSize, -halfSize}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			{{ halfSize, -halfSize,  halfSize}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			
			// Right face
			{{ halfSize, -halfSize,  halfSize}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
			{{ halfSize, -halfSize, -halfSize}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
			{{ halfSize,  halfSize, -halfSize}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
			{{ halfSize,  halfSize,  halfSize}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
			
			// Left face
			{{-halfSize, -halfSize, -halfSize}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
			{{-halfSize, -halfSize,  halfSize}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
			{{-halfSize,  halfSize,  halfSize}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
			{{-halfSize,  halfSize, -halfSize}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}
		};
		
		return vertices;
	}

	static std::vector<Shape::Vertex> generateSphereVertices(float radius, unsigned int segments) {
		std::vector<Shape::Vertex> vertices;
		
		for (unsigned int y = 0; y <= segments; y++) {
			for (unsigned int x = 0; x <= segments; x++) {
				float xSegment = (float)x / (float)segments;
				float ySegment = (float)y / (float)segments;
				float xPos = std::cos(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);
				float yPos = std::cos(ySegment * M_PI);
				float zPos = std::sin(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);
				
				Shape::Vertex vertex;
				vertex.position = {radius * xPos, radius * yPos, radius * zPos};
				vertex.normal = {xPos, yPos, zPos};
				vertex.texCoord = {xSegment, ySegment};

				float sinTheta = std::sin(ySegment * M_PI);
				float cosTheta = std::cos(ySegment * M_PI);
				float sinPhi = std::sin(xSegment * 2.0f * M_PI);
				float cosPhi = std::cos(xSegment * 2.0f * M_PI);

				vertex.tangent = glm::vec3(-sinPhi, 0, cosPhi);
				vertex.bitangent = glm::vec3(cosTheta * cosPhi, -sinTheta, cosTheta * sinPhi);

				vertex.tangent = glm::normalize(vertex.tangent - vertex.normal * glm::dot(vertex.normal, vertex.tangent));
				vertex.bitangent = glm::cross(vertex.normal, vertex.tangent);
				
				vertices.push_back(vertex);
			}
		}
		
		return vertices;
	}

	static std::vector<Shape::Vertex> generateCylinderVertices(float radius, float height, unsigned int segments) {
		std::vector<Shape::Vertex> vertices;
		float halfHeight = height / 2.0f;
		
		vertices.push_back({{0.0f, halfHeight, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}); // Top center vertex
		vertices.push_back({{0.0f, -halfHeight, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}); // Bottom center vertex
		
		for (unsigned int i = 0; i <= segments; i++) {
			float angle = 2.0f * M_PI * i / segments;
			float x = radius * std::cos(angle);
			float z = radius * std::sin(angle);
			float u = static_cast<float>(i) / segments;
			
			glm::vec3 sideNormal = glm::normalize(glm::vec3(x, 0.0f, z));
			glm::vec3 tangent = glm::vec3(-z, 0.0f, x);
			glm::vec3 bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
	
			vertices.push_back({{x, halfHeight, z}, {0.0f, 1.0f, 0.0f}, {(x/radius + 1.0f) * 0.5f, (z/radius + 1.0f) * 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}); // Top rim vertex 
			vertices.push_back({{x, -halfHeight, z}, {0.0f, -1.0f, 0.0f}, {(x/radius + 1.0f) * 0.5f, (z/radius + 1.0f) * 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}); // Bottom rim vertex 
			vertices.push_back({{x, halfHeight, z}, sideNormal, {u, 1.0f}, tangent, bitangent}); // Side top vertex
			vertices.push_back({{x, -halfHeight, z}, sideNormal, {u, 0.0f}, tangent, bitangent}); // Side bottom vertex
		}
		
		return vertices;
	}

	static std::vector<Shape::Vertex> generatePlaneVertices(float width, float height, unsigned int widthSegments, unsigned int heightSegments, float uRepeat, float vRepeat) {
		std::vector<Shape::Vertex> vertices;
		float halfWidth = width / 2.0f;
		float halfHeight = height / 2.0f;
		
		for (unsigned int y = 0; y <= heightSegments; y++) {
			for (unsigned int x = 0; x <= widthSegments; x++) {
				float u = static_cast<float>(x) / widthSegments;
				float v = static_cast<float>(y) / heightSegments;
				float xPos = -halfWidth + width * u;
				float yPos = 0.0f;
				float zPos = -halfHeight + height * v;
				
				Shape::Vertex vertex;
				vertex.position = {xPos, yPos, zPos};
				vertex.normal = {0.0f, 1.0f, 0.0f};
				
				vertex.texCoord = {u * uRepeat, v * vRepeat};
				
				vertex.tangent = {1.0f, 0.0f, 0.0f};
				vertex.bitangent = {0.0f, 0.0f, 1.0f};
				
				vertices.push_back(vertex);
			}
		}
		
		return vertices;
	}

	static std::vector<Shape::Vertex> generateCapsuleVertices(float radius, float height, unsigned int segments) {
		std::vector<Shape::Vertex> vertices;
		
		float cylinderHeight = height - (2.0f * radius);
		float halfCylinderHeight = cylinderHeight / 2.0f;
		unsigned int hemisphereRings = segments / 2;
		
		for (unsigned int y = 0; y <= hemisphereRings; y++) {
			float ySegment = static_cast<float>(y) / segments;
			float yAngle = ySegment * M_PI;
			
			for (unsigned int x = 0; x <= segments; x++) {
				float xSegment = static_cast<float>(x) / segments;
				float xAngle = xSegment * 2.0f * M_PI;
				
				float xPos = std::cos(xAngle) * std::sin(yAngle);
				float yPos = std::cos(yAngle);
				float zPos = std::sin(xAngle) * std::sin(yAngle);
				
				Shape::Vertex vertex;
				vertex.position = {radius * xPos, halfCylinderHeight + radius * yPos, radius * zPos};
				vertex.normal = {xPos, yPos, zPos};
				vertex.texCoord = {xSegment, 1.0f - (ySegment * (radius / height))};
				
				vertex.tangent = glm::normalize(glm::vec3(-zPos, 0.0f, xPos));
				vertex.bitangent = glm::normalize(glm::cross(vertex.normal, vertex.tangent));
				
				vertices.push_back(vertex);
			}
		}
		
		for (unsigned int i = 0; i <= segments; i++) {
			float angle = 2.0f * M_PI * i / segments;
			float x = radius * std::cos(angle);
			float z = radius * std::sin(angle);
			float u = static_cast<float>(i) / segments;
			
			glm::vec3 normal = {x / radius, 0.0f, z / radius};
			glm::vec3 tangent = {-z / radius, 0.0f, x / radius};
			glm::vec3 bitangent = {0.0f, 1.0f, 0.0f};

			vertices.push_back({{x, halfCylinderHeight, z}, normal, {u, 1.0f - (radius / height)}, tangent, bitangent}); // Top of cylinder ring
			vertices.push_back({{x, -halfCylinderHeight, z}, normal, {u, radius / height}, tangent, bitangent}); // Bottom of cylinder ring
		}
		
		for (unsigned int y = 0; y <= hemisphereRings; y++) {
			float ySegment = static_cast<float>(hemisphereRings + y) / segments;
			float yAngle = ySegment * M_PI;
			
			for (unsigned int x = 0; x <= segments; x++) {
				float xSegment = static_cast<float>(x) / segments;
				float xAngle = xSegment * 2.0f * M_PI;
				
				float xPos = std::cos(xAngle) * std::sin(yAngle);
				float yPos = std::cos(yAngle);
				float zPos = std::sin(xAngle) * std::sin(yAngle);
				
				Shape::Vertex vertex;
				vertex.position = {radius * xPos, -halfCylinderHeight + radius * yPos, radius * zPos};
				vertex.normal = {xPos, yPos, zPos};
				vertex.texCoord = {xSegment, (radius / height) - (ySegment - 0.5f) * (radius / height)};

				vertex.tangent = glm::normalize(glm::vec3(-zPos, 0.0f, xPos));
				vertex.bitangent = glm::normalize(glm::cross(vertex.normal, vertex.tangent));

				vertices.push_back(vertex);
			}
		}
		
		return vertices;
	}

	static std::vector<uint8_t> verticesToRawData(const std::vector<Shape::Vertex>& vertices) {
		std::vector<uint8_t> rawData(sizeof(Vertex) * vertices.size());
		std::memcpy(rawData.data(), vertices.data(), rawData.size());
		return rawData;
	}
};

class DebugDrawer {
public:
	struct DebugVertex {
		glm::vec3 position;
		glm::vec3 color;
	};

	DebugDrawer() {}
	~DebugDrawer() {
		lineVertices.clear();
		pointVertices.clear();

		if (lineShader) {
			lineShader->destroy();
			lineShader.reset();
		}
		if (pointShader) {
			pointShader->destroy();
			pointShader.reset();
		}
		lineVAO.destroy();
		lineVBO.destroy();
		pointVAO.destroy();
		pointVBO.destroy();
	}

	bool init() {
		const auto& debugSource = ShaderLibrary::getShader("debug");
		lineShader = std::make_unique<GLR::Shader>(
			debugSource.vertex,
			debugSource.fragment,
			true
		);
		
		if (!lineShader) {
			ERROR("Failed to create line shader");
			return false;
		}

		const auto& debugPointSource = ShaderLibrary::getShader("debug_point");
		pointShader = std::make_unique<GLR::Shader>(
			debugPointSource.vertex,
			debugPointSource.fragment,
			true
		);
		
		if (!pointShader) {
			ERROR("Failed to create point shader");
			return false;
		}

		constexpr size_t initialBufferSize = sizeof(DebugVertex) * 10000;

		// Setup line VAO and VBO
		lineVAO.bind();
		lineVBO.create(std::vector<uint8_t>(initialBufferSize));
		lineVBO.bind();
		
		// Position attribute
		lineVAO.linkAttribute(0, 3, GL_FLOAT, sizeof(DebugVertex), offsetof(DebugVertex, position));
		// Color attribute
		lineVAO.linkAttribute(1, 3, GL_FLOAT, sizeof(DebugVertex), offsetof(DebugVertex, color));
		
		lineVAO.unbind();
		lineVBO.unbind();

		// Setup point VAO and VBO
		pointVAO.bind();
		pointVBO.create(std::vector<uint8_t>(initialBufferSize));
		pointVBO.bind();
		
		pointVAO.linkAttribute(0, 3, GL_FLOAT, sizeof(DebugVertex), offsetof(DebugVertex, position));
		pointVAO.linkAttribute(1, 3, GL_FLOAT, sizeof(DebugVertex), offsetof(DebugVertex, color));

		GL_CHECK();
		
		pointVAO.unbind();
		pointVBO.unbind();

		return true;
	}

	void render(const glm::mat4& viewProjection) {
		if (lineVertices.empty() && pointVertices.empty()) {
			return;
		}
		
		if (!lineShader || !pointShader) {
			ERROR("Debug shaders not initialized");
			return;
		}

		// Enable blending for transparent debug shapes
		glEnable(GL_BLEND);
		GL_CHECK();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GL_CHECK();

		if (!lineVertices.empty()) {
			lineShader->bind();
			lineShader->setMatrix4Float("viewProjection", viewProjection);
			
			const size_t lineDataSize = lineVertices.size() * sizeof(DebugVertex);
			
			// Check if we need to resize the buffer
			GLint currentBufferSize;
			lineVBO.bind();
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &currentBufferSize);
			GL_CHECK();
			
			if (static_cast<size_t>(currentBufferSize) < lineDataSize) {
				const size_t newSize = lineDataSize * 2;
				glBufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_DRAW);
				GL_CHECK();
			}
			
			lineVAO.bind();
			glBufferSubData(GL_ARRAY_BUFFER, 0, lineDataSize, lineVertices.data());
			GL_CHECK();
			
			glLineWidth(1.0f);
			GL_CHECK();
			glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineVertices.size()));
			GL_CHECK();
			
			lineVAO.unbind();
			lineShader->unbind();
		}

		// Render points
		if (!pointVertices.empty()) {
			pointShader->bind();
			pointShader->setMatrix4Float("viewProjection", viewProjection);
			
			const size_t pointDataSize = pointVertices.size() * sizeof(DebugVertex);
			
			GLint currentBufferSize;
			pointVBO.bind();
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &currentBufferSize);
			GL_CHECK();
			
			if (static_cast<size_t>(currentBufferSize) < pointDataSize) {
				const size_t newSize = pointDataSize * 2;
				glBufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_DRAW);
				GL_CHECK();
			}
			
			pointVAO.bind();
			glBufferSubData(GL_ARRAY_BUFFER, 0, pointDataSize, pointVertices.data());
			GL_CHECK();
			
			glPointSize(10.0f);
			GL_CHECK();
			glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(pointVertices.size()));
			GL_CHECK();
			
			pointVAO.unbind();
			pointShader->unbind();
		}

		glDisable(GL_BLEND);
		GL_CHECK();
		
		lineVertices.clear();
		pointVertices.clear();
	}

	// Basic functions
	void drawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color = glm::vec3(1.0f)) {
		lineVertices.push_back({start, color});
		lineVertices.push_back({end, color});
	}

	void drawPoint(const glm::vec3& position, const glm::vec3& color = glm::vec3(1.0f)) {
		pointVertices.push_back({position, color});
	}

	// Simple shapes
	void drawRectangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& v4, const glm::vec3& color = glm::vec3(1.0f)) {
		drawLine(v1, v2, color);
		drawLine(v2, v3, color);
		drawLine(v3, v4, color);
		drawLine(v4, v1, color);
	}

	void drawCircle(const glm::vec3& center, float radius, const glm::vec3& normal, const glm::vec3& color = glm::vec3(1.0f), int segments = 32) {
		if (glm::length(normal) < glm::epsilon<float>()) { return; }
		
		glm::vec3 normalized = glm::normalize(normal);
		glm::vec3 perpendicular1 = calculatePerpendicular(normalized);
		glm::vec3 perpendicular2 = glm::normalize(glm::cross(normalized, perpendicular1));
		
		const int clampedSegments = glm::clamp(segments, 3, 64);
		const float step = 2.0f * glm::pi<float>() / static_cast<float>(clampedSegments);
		
		std::vector<glm::vec3> points;
		points.reserve(clampedSegments);
		
		for (int i = 0; i < clampedSegments; i++) {
			float angle = i * step;
			glm::vec3 point = center + radius * (perpendicular1 * std::cos(angle) + perpendicular2 * std::sin(angle));
			points.push_back(point);
		}
		
		for (int i = 0; i < clampedSegments; i++) {
			drawLine(points[i], points[(i + 1) % clampedSegments], color);
		}
	}

	void drawArrow(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color = glm::vec3(1.0f), float tipSize = 0.1f) {
		if (glm::distance(start, end) < glm::epsilon<float>()) {
			drawPoint(start, color);
			return;
		}
		
		drawLine(start, end, color);
		
		// Calculate the arrow head
		glm::vec3 direction = glm::normalize(end - start);
		
		glm::vec3 perpendicular = calculatePerpendicular(direction);
		glm::vec3 perpendicular2 = glm::normalize(glm::cross(direction, perpendicular));
		
		// Draw arrow head
		const float clampedTipSize = glm::clamp(tipSize, 0.01f, 10.0f);
		glm::vec3 arrow1 = end - direction * clampedTipSize + perpendicular * clampedTipSize * 0.5f;
		glm::vec3 arrow2 = end - direction * clampedTipSize - perpendicular * clampedTipSize * 0.5f;
		glm::vec3 arrow3 = end - direction * clampedTipSize + perpendicular2 * clampedTipSize * 0.5f;
		glm::vec3 arrow4 = end - direction * clampedTipSize - perpendicular2 * clampedTipSize * 0.5f;
		
		drawLine(end, arrow1, color);
		drawLine(end, arrow2, color);
		drawLine(end, arrow3, color);
		drawLine(end, arrow4, color);
	}

	void drawAxes(const glm::vec3& position, float size = 1.0f) {
		const float clampedSize = glm::clamp(size, 0.01f, 1000.0f);
		drawArrow(position, position + glm::vec3(clampedSize, 0, 0), glm::vec3(1, 0, 0), clampedSize * 0.1f);
		drawArrow(position, position + glm::vec3(0, clampedSize, 0), glm::vec3(0, 1, 0), clampedSize * 0.1f);
		drawArrow(position, position + glm::vec3(0, 0, clampedSize), glm::vec3(0, 0, 1), clampedSize * 0.1f);
	}

	// 3D shapes
	void drawBox(const glm::mat4& transform, const glm::vec3& halfExtents, const glm::vec3& color = glm::vec3(1.0f)) {
		const glm::vec3 vertices[8] = {
			glm::vec3(-halfExtents.x, -halfExtents.y, -halfExtents.z),
			glm::vec3( halfExtents.x, -halfExtents.y, -halfExtents.z),
			glm::vec3( halfExtents.x,  halfExtents.y, -halfExtents.z),
			glm::vec3(-halfExtents.x,  halfExtents.y, -halfExtents.z),
			glm::vec3(-halfExtents.x, -halfExtents.y,  halfExtents.z),
			glm::vec3( halfExtents.x, -halfExtents.y,  halfExtents.z),
			glm::vec3( halfExtents.x,  halfExtents.y,  halfExtents.z),
			glm::vec3(-halfExtents.x,  halfExtents.y,  halfExtents.z)
		};

		glm::vec3 transformedVertices[8];
		for (int i = 0; i < 8; i++) {
			glm::vec4 transformedVec = transform * glm::vec4(vertices[i], 1.0f);
			transformedVertices[i] = glm::vec3(transformedVec);
		}

		drawRectangle(transformedVertices[0], transformedVertices[1], transformedVertices[2], transformedVertices[3], color);
		drawRectangle(transformedVertices[4], transformedVertices[5], transformedVertices[6], transformedVertices[7], color);
		
		drawLine(transformedVertices[0], transformedVertices[4], color);
		drawLine(transformedVertices[1], transformedVertices[5], color);
		drawLine(transformedVertices[2], transformedVertices[6], color);
		drawLine(transformedVertices[3], transformedVertices[7], color);
	}

	void drawSphere(const glm::vec3& center, float radius, const glm::vec3& color = glm::vec3(1.0f), int segments = 16) {
		const int clampedSegments = glm::clamp(segments, 3, 64);
		
		drawCircle(center, radius, glm::vec3(0, 0, 1), color, clampedSegments); // XY circle
		drawCircle(center, radius, glm::vec3(0, 1, 0), color, clampedSegments); // XZ circle  
		drawCircle(center, radius, glm::vec3(1, 0, 0), color, clampedSegments); // YZ circle
	}

	void drawCylinder(const glm::vec3& start, const glm::vec3& end, float radius, const glm::vec3& color = glm::vec3(1.0f), int segments = 16) {
		glm::vec3 direction = end - start;
		float height = glm::length(direction);
		
		if (height < glm::epsilon<float>()) {
			return;
		}
		
		direction = glm::normalize(direction);
		
		const int clampedSegments = glm::clamp(segments, 3, 64);
		
		drawCircle(start, radius, direction, color, clampedSegments);
		drawCircle(end, radius, direction, color, clampedSegments);
		
		glm::vec3 perpendicular1 = calculatePerpendicular(direction);
		glm::vec3 perpendicular2 = glm::normalize(glm::cross(direction, perpendicular1));
		
		const float step = 2.0f * glm::pi<float>() / static_cast<float>(clampedSegments);
		
		for (int i = 0; i < clampedSegments; i++) {
			float angle = i * step;
			glm::vec3 offset = radius * (perpendicular1 * std::cos(angle) + perpendicular2 * std::sin(angle));
			drawLine(start + offset, end + offset, color);
		}
	}

	void drawCone(const glm::vec3& apex, const glm::vec3& direction, float height, float radius, const glm::vec3& color = glm::vec3(1.0f), int segments = 16) {
		if (glm::length(direction) < glm::epsilon<float>()) { return; }
		
		glm::vec3 normalizedDir = glm::normalize(direction);
		glm::vec3 baseCenter = apex + normalizedDir * height;
		
		const int clampedSegments = glm::clamp(segments, 3, 64);
		
		drawCircle(baseCenter, radius, normalizedDir, color, clampedSegments);
		
		glm::vec3 perpendicular1 = calculatePerpendicular(normalizedDir);
		glm::vec3 perpendicular2 = glm::normalize(glm::cross(normalizedDir, perpendicular1));
		const float step = 2.0f * glm::pi<float>() / static_cast<float>(clampedSegments);
		
		for (int i = 0; i < clampedSegments; i += clampedSegments / 4) {
			float angle = i * step;
			glm::vec3 offset = radius * (perpendicular1 * std::cos(angle) + perpendicular2 * std::sin(angle));
			drawLine(apex, baseCenter + offset, color);
		}
	}

	void drawPlane(const glm::vec3& center, const glm::vec3& normal, float size = 1.0f, const glm::vec3& color = glm::vec3(1.0f)) {
		if (glm::length(normal) < glm::epsilon<float>()) { return; }
		
		glm::vec3 normalizedNormal = glm::normalize(normal);
		glm::vec3 tangent = calculatePerpendicular(normalizedNormal);
		glm::vec3 bitangent = glm::normalize(glm::cross(normalizedNormal, tangent));
		
		const float clampedSize = glm::clamp(size, 0.01f, 1000.0f);
		float halfSize = clampedSize * 0.5f;
		glm::vec3 v1 = center - tangent * halfSize - bitangent * halfSize;
		glm::vec3 v2 = center + tangent * halfSize - bitangent * halfSize;
		glm::vec3 v3 = center + tangent * halfSize + bitangent * halfSize;
		glm::vec3 v4 = center - tangent * halfSize + bitangent * halfSize;
		
		drawRectangle(v1, v2, v3, v4, color);
		drawArrow(center, center + normalizedNormal * halfSize, color, halfSize * 0.2f);
	}

	// Complex shapes
	void drawCapsule(const glm::vec3& start, const glm::vec3& end, float radius, const glm::vec3& color = glm::vec3(1.0f), int segments = 16) {
		glm::vec3 direction = end - start;
		float height = glm::length(direction);
		
		if (height < glm::epsilon<float>()) {
			drawSphere(start, radius, color, segments);
			return;
		}
		
		direction = glm::normalize(direction);
		
		glm::vec3 perpendicular1 = calculatePerpendicular(direction);
		glm::vec3 perpendicular2 = glm::normalize(glm::cross(direction, perpendicular1));
		
		const int clampedSegments = glm::clamp(segments, 3, 64);
		const float step = 2.0f * glm::pi<float>() / static_cast<float>(clampedSegments);
		
		const int numRings = 3;
		for (int ring = 0; ring <= numRings; ring++) {
			float t = static_cast<float>(ring) / static_cast<float>(numRings);
			glm::vec3 center = glm::mix(start, end, t);
			drawCircle(center, radius, direction, color, clampedSegments);
		}
		
		const int numConnectors = 4;
		for (int i = 0; i < numConnectors; i++) {
			float angle = i * (glm::pi<float>() / 2.0f);
			glm::vec3 offset = radius * (perpendicular1 * std::cos(angle) + perpendicular2 * std::sin(angle));
			drawLine(start + offset, end + offset, color);
		}
		
		const int halfSegments = clampedSegments / 2;
		const float halfStep = glm::pi<float>() / static_cast<float>(halfSegments);
		
		for (int meridian = 0; meridian < clampedSegments; meridian++) {
			float angle1 = meridian * step;
			float angle2 = (meridian + 1) * step;
			
			glm::vec3 dir1 = perpendicular1 * std::cos(angle1) + perpendicular2 * std::sin(angle1);
			glm::vec3 dir2 = perpendicular1 * std::cos(angle2) + perpendicular2 * std::sin(angle2);
			
			for (int i = 0; i < halfSegments; i++) {
				float latitude1 = i * halfStep;
				float latitude2 = (i + 1) * halfStep;
				
				float sinLat1 = std::sin(latitude1);
				float cosLat1 = std::cos(latitude1);
				float sinLat2 = std::sin(latitude2);
				float cosLat2 = std::cos(latitude2);
				
				glm::vec3 p1 = start + radius * (sinLat1 * dir1 - cosLat1 * direction);
				glm::vec3 p2 = start + radius * (sinLat2 * dir1 - cosLat2 * direction);
				glm::vec3 p3 = start + radius * (sinLat1 * dir2 - cosLat1 * direction);
				glm::vec3 p4 = start + radius * (sinLat2 * dir2 - cosLat2 * direction);
				
				if (i > 0) drawLine(p1, p3, color);
				if (i < halfSegments - 1) drawLine(p2, p4, color);
				drawLine(p1, p2, color);
			}
			
			for (int i = 0; i < halfSegments; i++) {
				float latitude1 = i * halfStep;
				float latitude2 = (i + 1) * halfStep;
				
				float sinLat1 = std::sin(latitude1);
				float cosLat1 = std::cos(latitude1);
				float sinLat2 = std::sin(latitude2);
				float cosLat2 = std::cos(latitude2);
				
				glm::vec3 p1 = end + radius * (sinLat1 * dir1 + cosLat1 * direction);
				glm::vec3 p2 = end + radius * (sinLat2 * dir1 + cosLat2 * direction);
				glm::vec3 p3 = end + radius * (sinLat1 * dir2 + cosLat1 * direction);
				glm::vec3 p4 = end + radius * (sinLat2 * dir2 + cosLat2 * direction);
				
				if (i > 0) drawLine(p1, p3, color);
				if (i < halfSegments - 1) drawLine(p2, p4, color);
				drawLine(p1, p2, color);
			}
		}
	}

	void drawFrustum(const glm::mat4& viewProjection, const glm::vec3& color = glm::vec3(1.0f)) {
		glm::mat4 invVP = glm::inverse(viewProjection);
		
		const glm::vec4 clipSpaceCorners[8] = {
			glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
			glm::vec4( 1.0f, -1.0f, -1.0f, 1.0f),
			glm::vec4( 1.0f,  1.0f, -1.0f, 1.0f),
			glm::vec4(-1.0f,  1.0f, -1.0f, 1.0f),
			glm::vec4(-1.0f, -1.0f,  1.0f, 1.0f),
			glm::vec4( 1.0f, -1.0f,  1.0f, 1.0f),
			glm::vec4( 1.0f,  1.0f,  1.0f, 1.0f),
			glm::vec4(-1.0f,  1.0f,  1.0f, 1.0f) 
		};
		
		glm::vec3 worldCorners[8];
		for (int i = 0; i < 8; i++) {
			glm::vec4 worldPos = invVP * clipSpaceCorners[i];
			if (std::abs(worldPos.w) < glm::epsilon<float>()) {
				return;
			}
			worldCorners[i] = glm::vec3(worldPos) / worldPos.w;
		}
		
		drawRectangle(worldCorners[0], worldCorners[1], worldCorners[2], worldCorners[3], color);
		drawRectangle(worldCorners[4], worldCorners[5], worldCorners[6], worldCorners[7], color);
		
		drawLine(worldCorners[0], worldCorners[4], color);
		drawLine(worldCorners[1], worldCorners[5], color);
		drawLine(worldCorners[2], worldCorners[6], color);
		drawLine(worldCorners[3], worldCorners[7], color);
	}

	void drawBone(const glm::mat4& startTransform, const glm::mat4& endTransform, const glm::vec3& color = glm::vec3(1.0f, 0.5f, 0.0f)) {
		glm::vec3 startPos = glm::vec3(startTransform[3]);
		glm::vec3 endPos = glm::vec3(endTransform[3]);
		
		if (glm::distance(startPos, endPos) < glm::epsilon<float>()) {
			drawPoint(startPos, color);
			return;
		}
		
		drawLine(startPos, endPos, color);
		
		float distance = glm::distance(startPos, endPos);
		float markerSize = distance * 0.1f;
		markerSize = glm::clamp(markerSize, 0.01f, 10.0f);
		
		glm::vec3 direction = glm::normalize(endPos - startPos);
		glm::vec3 perpA = calculatePerpendicular(direction);
		glm::vec3 perpB = glm::normalize(glm::cross(direction, perpA));
		
		drawLine(endPos - perpA * markerSize, endPos + perpA * markerSize, color);
		drawLine(endPos - perpB * markerSize, endPos + perpB * markerSize, color);
	}

	// Utility	
	size_t getLineVertexCount() const { return lineVertices.size(); }
	size_t getPointVertexCount() const { return pointVertices.size(); }
	bool isInitialized() const { return lineShader && pointShader; }

private:
	std::unique_ptr<GLR::Shader> lineShader;
	std::unique_ptr<GLR::Shader> pointShader;
	
	GLR::VAO lineVAO;
	GLR::VBO<uint8_t> lineVBO;
	
	GLR::VAO pointVAO;
	GLR::VBO<uint8_t> pointVBO;
	
	std::vector<DebugVertex> lineVertices;
	std::vector<DebugVertex> pointVertices;
	
	glm::vec3 calculatePerpendicular(const glm::vec3& direction) const {
		if (std::abs(direction.x) < 0.707f) {
			return glm::normalize(glm::cross(direction, glm::vec3(1.0f, 0.0f, 0.0f)));
		} 
		else {
			return glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
		}
	}
};

/////////////////////
// Physics Classes //
/////////////////////

struct CollisionEvent {
	Entity* entityA;
	Entity* entityB;
	reactphysics3d::Vector3 contactPoint;
	reactphysics3d::Vector3 contactNormal;
	float penetrationDepth;
};

class ICollisionListener {
public:
	virtual ~ICollisionListener() = default;
	virtual void onCollisionEnter(const CollisionEvent& event) {}
	virtual void onCollisionStay(const CollisionEvent& event) {}
	virtual void onCollisionExit(const CollisionEvent& event) {}
	virtual void onTriggerEnter(const CollisionEvent& event) {}
	virtual void onTriggerExit(const CollisionEvent& event) {}
};

class CollisionEventListener : public reactphysics3d::EventListener {
public:
	// Reactphysics3d event listener
	CollisionEventListener() = default;
	virtual ~CollisionEventListener() = default;
	
	// Overriding the collision callbacks from ReactPhysics3D
	void onContact(const reactphysics3d::CollisionCallback::CallbackData& callbackData) override {
		for (uint32_t p = 0; p < callbackData.getNbContactPairs(); p++) {
			reactphysics3d::CollisionCallback::ContactPair contactPair = callbackData.getContactPair(p);
			
			// Get the colliders
			reactphysics3d::Collider* collider1 = contactPair.getCollider1();
			reactphysics3d::Collider* collider2 = contactPair.getCollider2();
			
			// Get entities from user data
			Entity* entity1 = static_cast<Entity*>(collider1->getBody()->getUserData());
			Entity* entity2 = static_cast<Entity*>(collider2->getBody()->getUserData());
			
			if (!entity1 || !entity2) continue;
			
			// Check if either collider is a trigger
			bool isTrigger = collider1->getIsTrigger() || collider2->getIsTrigger();
			
			// Process contact points
			for (uint32_t c = 0; c < contactPair.getNbContactPoints(); c++) {
				reactphysics3d::CollisionCallback::ContactPoint contactPoint = contactPair.getContactPoint(c);
				
				CollisionEvent event;
				event.entityA = entity1;
				event.entityB = entity2;
				event.contactPoint = contactPoint.getLocalPointOnCollider1();
				event.contactNormal = contactPoint.getWorldNormal();
				event.penetrationDepth = contactPoint.getPenetrationDepth();
				
				if (isTrigger) {
					handleTriggerContact(event);
				} 
				else {
					handleCollisionContact(event);
				}
			}
		}
	}
	void onTrigger(const reactphysics3d::OverlapCallback::CallbackData& callbackData) override {
		for (uint32_t p = 0; p < callbackData.getNbOverlappingPairs(); p++) {
			reactphysics3d::OverlapCallback::OverlapPair overlapPair = callbackData.getOverlappingPair(p);
			
			// Get the colliders
			reactphysics3d::Collider* collider1 = overlapPair.getCollider1();
			reactphysics3d::Collider* collider2 = overlapPair.getCollider2();
			
			// Get entities from user data
			Entity* entity1 = static_cast<Entity*>(collider1->getBody()->getUserData());
			Entity* entity2 = static_cast<Entity*>(collider2->getBody()->getUserData());
			
			if (!entity1 || !entity2) continue;
			
			CollisionEvent event;
			event.entityA = entity1;
			event.entityB = entity2;
			event.contactPoint = reactphysics3d::Vector3(0, 0, 0); // No contact point for overlaps
			event.contactNormal = reactphysics3d::Vector3(0, 0, 0);
			event.penetrationDepth = 0.0f;
			
			// Track trigger state
			std::pair<Entity*, Entity*> entityPair = std::make_pair(std::min(entity1, entity2), std::max(entity1, entity2));
			
			if (activeTriggers.find(entityPair) == activeTriggers.end()) {
				activeTriggers.insert(entityPair);
				notifyTriggerEnter(event);
			}
		}
		
		checkTriggerExits(callbackData);
	}

	void clearCollisionState() { activeCollisions.clear(); }
	void addCollisionListener(ICollisionListener* listener) { listeners.push_back(listener); }
	void removeCollisionListener(ICollisionListener* listener) {
		auto it = std::find(listeners.begin(), listeners.end(), listener);
		if (it != listeners.end()) {
			listeners.erase(it);
		}
	}
	
private:
	std::vector<ICollisionListener*> listeners;
	std::set<std::pair<Entity*, Entity*>> activeCollisions;
	std::set<std::pair<Entity*, Entity*>> activeTriggers;
	
	void handleCollisionContact(const CollisionEvent& event) {
		std::pair<Entity*, Entity*> entityPair = std::make_pair(std::min(event.entityA, event.entityB), std::max(event.entityA, event.entityB));
		
		if (activeCollisions.find(entityPair) == activeCollisions.end()) {
			// New collision
			activeCollisions.insert(entityPair);
			notifyCollisionEnter(event);
		} 
		else {
			// Ongoing collision
			notifyCollisionStay(event);
		}
	}
	
	void handleTriggerContact(const CollisionEvent& event) {
		std::pair<Entity*, Entity*> entityPair = std::make_pair(
			std::min(event.entityA, event.entityB), std::max(event.entityA, event.entityB)
		);
		
		if (activeTriggers.find(entityPair) == activeTriggers.end()) {
			activeTriggers.insert(entityPair);
			notifyTriggerEnter(event);
		}
	}

	// Not implemented yet
	// void checkCollisionExits() {}
	
	void checkTriggerExits(const reactphysics3d::OverlapCallback::CallbackData& callbackData) {
		std::set<std::pair<Entity*, Entity*>> currentTriggers;
		
		for (uint32_t p = 0; p < callbackData.getNbOverlappingPairs(); p++) {
			reactphysics3d::OverlapCallback::OverlapPair overlapPair = callbackData.getOverlappingPair(p);
			
			Entity* entity1 = static_cast<Entity*>(overlapPair.getCollider1()->getBody()->getUserData());
			Entity* entity2 = static_cast<Entity*>(overlapPair.getCollider2()->getBody()->getUserData());
			
			if (entity1 && entity2) {
				currentTriggers.insert(std::make_pair(std::min(entity1, entity2), std::max(entity1, entity2)));
			}
		}
		
		for (const auto& entityPair : activeTriggers) {
			if (currentTriggers.find(entityPair) == currentTriggers.end()) {
				CollisionEvent event;
				event.entityA = entityPair.first;
				event.entityB = entityPair.second;
				event.contactPoint = reactphysics3d::Vector3(0, 0, 0);
				event.contactNormal = reactphysics3d::Vector3(0, 0, 0);
				event.penetrationDepth = 0.0f;
				
				notifyTriggerExit(event);
			}
		}
		
		activeTriggers = currentTriggers;
	}
	
	void notifyCollisionEnter(const CollisionEvent& event) {
		for (auto* listener : listeners) {
			listener->onCollisionEnter(event);
		}
	}
	
	void notifyCollisionStay(const CollisionEvent& event) {
		for (auto* listener : listeners) {
			listener->onCollisionStay(event);
		}
	}

	// Not implemented yet
	// void notifyCollisionExit(const CollisionEvent& event) {}
	
	void notifyTriggerEnter(const CollisionEvent& event) {
		for (auto* listener : listeners) {
			listener->onTriggerEnter(event);
		}
	}
	
	void notifyTriggerExit(const CollisionEvent& event) {
		for (auto* listener : listeners) {
			listener->onTriggerExit(event);
		}
	}
};

class PhysicsWorld {
public:
	PhysicsWorld() {
		isShuttingDown = false;
		physicsCommon = std::make_unique<reactphysics3d::PhysicsCommon>();
		
		// Create the physics world with gravity
		reactphysics3d::Vector3 gravity(0.0f, gravityForce, 0.0f);
		physicsWorld = physicsCommon->createPhysicsWorld();
		physicsWorld->setGravity(gravity);
		collisionListener = std::make_unique<CollisionEventListener>();
		physicsWorld->setEventListener(collisionListener.get());
	}
	
	~PhysicsWorld() {
		isShuttingDown = true;
		
		if (physicsWorld && physicsCommon) {
			physicsCommon->destroyPhysicsWorld(physicsWorld);
		}
		physicsWorld = nullptr;
	}
	
	bool isValid() const {
		return !isShuttingDown && physicsWorld != nullptr && physicsCommon != nullptr;
	}
	
	// Update function made later
	void update(float deltaTime);
	
	void registerRigidBody(RigidBody* rb) {
		rigidBodies.push_back(rb);
	}
	
	void unregisterRigidBody(RigidBody* rb) {
		auto it = std::find(rigidBodies.begin(), rigidBodies.end(), rb);
		if (it != rigidBodies.end()) {
			rigidBodies.erase(it);
		}
	}
	
	// Collision listener management
	void addCollisionListener(ICollisionListener* listener) {
		collisionListener->addCollisionListener(listener);
	}
	
	void removeCollisionListener(ICollisionListener* listener) {
		collisionListener->removeCollisionListener(listener);
	}
	
	// Gravity management
	void setGravity(const reactphysics3d::Vector3& gravity) {
		gravityForce = gravity.y;
		physicsWorld->setGravity(gravity);
	}
	reactphysics3d::Vector3 getGravity() const {
		return physicsWorld->getGravity();
	}
		
	reactphysics3d::PhysicsWorld* getPhysicsWorld() { return physicsWorld; }
	reactphysics3d::PhysicsCommon* getPhysicsCommon() { return physicsCommon.get(); }
	
private:
	std::unique_ptr<reactphysics3d::PhysicsCommon> physicsCommon;
	reactphysics3d::PhysicsWorld* physicsWorld = nullptr;
	std::vector<RigidBody*> rigidBodies;
	std::unique_ptr<CollisionEventListener> collisionListener;
	float gravityForce = -9.81f;
	static bool isShuttingDown;
};
inline bool PhysicsWorld::isShuttingDown = false;
inline PhysicsWorld& GetPhysicsWorld() { // Singleton physics world
	static PhysicsWorld instance;
	return instance;
}

////////////////////////
// ECS and Components //
////////////////////////
class Component {
public:
	virtual ~Component() = default;
	virtual void init() {}
	virtual void update(float deltaTime) {}
	
	void setEntity(Entity* entity) { m_entity = entity; }
	Entity* getEntity() const { return m_entity; }
	
private:
	Entity* m_entity = nullptr;
};

class Entity : public std::enable_shared_from_this<Entity> {
public:
	explicit Entity(const std::string& name = "Entity") : name(name) {}
	
	~Entity() {
		if (auto p = parent.lock()) {
			p->removeChild(shared_from_this());
		}
		
		children.clear();
	}

	void update(float deltaTime) {
		for (auto& [typeId, component] : components) {
			component->update(deltaTime);
		}
	}

	// Component management
	template<typename T, typename... Args>
	T& addComponent(Args&&... args) {
		auto component = std::make_shared<T>(std::forward<Args>(args)...);
		component->setEntity(this);
		components[typeid(T).hash_code()] = component;
		component->init();
		return *component;
	}
	
	template<typename T>
	T& getComponent() {
		auto it = components.find(typeid(T).hash_code());
		if (it != components.end()) {
			return *std::static_pointer_cast<T>(it->second);
		}
		throw std::runtime_error("Component not found");
	}
	
	template<typename T>
	bool hasComponent() const { 
		return components.find(typeid(T).hash_code()) != components.end(); 
	}
	
	template<typename T>
	bool removeComponent() {
		auto it = components.find(typeid(T).hash_code());
		if (it != components.end()) {
			it->second->setEntity(nullptr);
			components.erase(it);
			return true;
		}
		return false;
	}
	
	std::unordered_map<size_t, std::shared_ptr<Component>> getComponents() const {
		return components;
	}

	// Hierarchy management
	void setParent(std::shared_ptr<Entity> newParent) {
		// Remove from current parent if exists
		if (auto currentParent = parent.lock()) {
			currentParent->removeChild(shared_from_this());
		}
		
		parent = newParent;
		
		if (newParent) {
			newParent->addChild(shared_from_this());
		}
	}
	
	std::shared_ptr<Entity> getParent() const {
		return parent.lock();
	}
	
	const std::vector<std::shared_ptr<Entity>>& getChildren() const {
		return children;
	}
	
	std::shared_ptr<Entity> getRoot() {
		auto current = shared_from_this();
		while (auto parent = current->getParent()) {
			current = parent;
		}
		return current;
	}
	
	int getDepth() const {
		int depth = 0;
		auto parent = getParent();
		while (parent) {
			depth++;
			parent = parent->getParent();
		}
		return depth;
	}

	void setParentScene(std::shared_ptr<Scene> scene) {
		parentScene = scene;
	}

	std::shared_ptr<Scene> getScene() { return parentScene; }

	// Hierarchy queries
	std::vector<std::shared_ptr<Entity>> getAllDescendants() const {
		std::vector<std::shared_ptr<Entity>> descendants;
		
		for (const auto& child : children) {
			descendants.push_back(child);
			auto childDescendants = child->getAllDescendants();
			descendants.insert(descendants.end(), childDescendants.begin(), childDescendants.end());
		}
		
		return descendants;
	}
	
	bool isAncestorOf(std::shared_ptr<Entity> entity) const {
		if (!entity) return false;
		
		auto parent = entity->getParent();
		while (parent) {
			if (parent.get() == this) {
				return true;
			}
			parent = parent->getParent();
		}
		return false;
	}
	
	bool isDescendantOf(std::shared_ptr<Entity> entity) {
		if (!entity) return false;
		return entity->isAncestorOf(shared_from_this());
	}
	
	std::shared_ptr<Entity> findChild(const std::string& name) const {
		for (const auto& child : children) {
			if (child->getName() == name) {
				return child;
			}
		}
		return nullptr;
	}
	
	std::shared_ptr<Entity> findDescendant(const std::string& name) const {
		if (auto found = findChild(name)) {
			return found;
		}
		
		for (const auto& child : children) {
			if (auto found = child->findDescendant(name)) {
				return found;
			}
		}
		
		return nullptr;
	}

	// Properties
	void setName(const std::string& newName) { name = newName; }
	const std::string& getName() const { return name; }

private:
	std::string name;
	std::unordered_map<size_t, std::shared_ptr<Component>> components;
	
	// Hierarchy
	std::weak_ptr<Entity> parent;
	std::vector<std::shared_ptr<Entity>> children;
	std::shared_ptr<Scene> parentScene;

	// Private helper methods
	void addChild(std::shared_ptr<Entity> child) {
		if (!child) return;
		
		auto it = std::find(children.begin(), children.end(), child);
		if (it == children.end()) {
			children.push_back(child);
			child->parent = shared_from_this();
		}
	}
	
	bool removeChild(std::shared_ptr<Entity> child) {
		if (!child) return false;
		
		auto it = std::find(children.begin(), children.end(), child);
		if (it != children.end()) {
			(*it)->parent.reset();
			children.erase(it);
			return true;
		}
		return false;
	}
};

class Transform : public Component {
public:
	Transform(const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f))
		: localPosition(position), localEulerRotation(rotation), localScale(scale), localMatrix(1.0f), worldMatrix(1.0f), isDirty(true) {
		localQuaternion = glm::quat(glm::radians(rotation));
		updateMatrices();
	}
	
	Transform(const glm::vec3& position, const glm::quat& quaternion, const glm::vec3& scale = glm::vec3(1.0f))
		: localPosition(position), localQuaternion(quaternion), localScale(scale), localMatrix(1.0f), worldMatrix(1.0f), isDirty(true) {
		localEulerRotation = glm::degrees(glm::eulerAngles(quaternion));
		updateMatrices();
	}
	
	// Local transform setters
	void setPosition(const glm::vec3& position) {
		localPosition = position;
		markDirty();
	}
	
	void setRotation(const glm::vec3& rotation) {
		localEulerRotation = rotation;
		localQuaternion = glm::quat(glm::radians(rotation));
		markDirty();
	}
	
	void setRotation(const glm::quat& quaternion) {
		localQuaternion = quaternion;
		localEulerRotation = glm::degrees(glm::eulerAngles(quaternion));
		markDirty();
	}

	void setScale(const glm::vec3& scale) {
		localScale = scale;
		markDirty();
	}
	
	// World transform setters
	void setWorldPosition(const glm::vec3& worldPosition) {
		if (auto entity = getEntity()) {
			if (auto parent = entity->getParent()) {
				if (parent->hasComponent<Transform>()) {
					auto& parentTransform = parent->getComponent<Transform>();
					glm::mat4 parentWorldMatrix = parentTransform.getMatrix();
					glm::mat4 parentWorldInverse = glm::inverse(parentWorldMatrix);
					glm::vec4 localPos = parentWorldInverse * glm::vec4(worldPosition, 1.0f);
					setPosition(glm::vec3(localPos));
					return;
				}
			}
		}
		setPosition(worldPosition);
	}
	
	void setWorldRotation(const glm::quat& worldRotation) {
		if (auto entity = getEntity()) {
			if (auto parent = entity->getParent()) {
				if (parent->hasComponent<Transform>()) {
					auto& parentTransform = parent->getComponent<Transform>();
					glm::quat parentWorldRotation = parentTransform.getWorldRotation();
					glm::quat localRotation = glm::inverse(parentWorldRotation) * worldRotation;
					setRotation(localRotation);
					return;
				}
			}
		}
		setRotation(worldRotation);
	}
	
	void setWorldScale(const glm::vec3& worldScale) {
		if (auto entity = getEntity()) {
			if (auto parent = entity->getParent()) {
				if (parent->hasComponent<Transform>()) {
					auto& parentTransform = parent->getComponent<Transform>();
					glm::vec3 parentWorldScale = parentTransform.getWorldScale();
					glm::vec3 localScale = worldScale / parentWorldScale;
					setScale(localScale);
					return;
				}
			}
		}
		setScale(worldScale);
	}
	
	// Local transform getters
	const glm::vec3& getPosition() const { return localPosition; }
	const glm::vec3& getRotation() const { return localEulerRotation; }
	const glm::quat& getQuaternion() const { return localQuaternion; }
	const glm::vec3& getScale() const { return localScale; }
	
	// World transform getters
	glm::vec3 getWorldPosition() const {
		updateMatricesIfNeeded();
		return glm::vec3(worldMatrix[3]);
	}
	
	glm::quat getWorldRotation() const {
		updateMatricesIfNeeded();
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(worldMatrix, scale, rotation, translation, skew, perspective);
		return rotation;
	}
	
	glm::vec3 getWorldScale() const {
		updateMatricesIfNeeded();
		glm::vec3 scale;
		scale.x = glm::length(glm::vec3(worldMatrix[0]));
		scale.y = glm::length(glm::vec3(worldMatrix[1]));
		scale.z = glm::length(glm::vec3(worldMatrix[2]));
		return scale;
	}
	
	// Matrix getters
	const glm::mat4& getMatrix() const { 
		updateMatricesIfNeeded();
		return worldMatrix; 
	}
	
	const glm::mat4& getLocalMatrix() const {
		updateMatricesIfNeeded();
		return localMatrix;
	}
	
	// Direction vectors
	glm::vec3 getForward() const { return glm::normalize(getWorldRotation() * glm::vec3(0.0f, 0.0f, -1.0f)); }
	glm::vec3 getRight() const { return glm::normalize(getWorldRotation() * glm::vec3(1.0f, 0.0f, 0.0f)); }
	glm::vec3 getUp() const { return glm::normalize(getWorldRotation() * glm::vec3(0.0f, 1.0f, 0.0f)); }
	
	void update(float deltaTime) override {
		if (isDirty) {
			propagateDirtyToChildren();
		}
	}

private:
	glm::vec3 localPosition;
	glm::vec3 localEulerRotation;
	glm::quat localQuaternion;
	glm::vec3 localScale;
	
	mutable glm::mat4 localMatrix;
	mutable glm::mat4 worldMatrix;
	mutable bool isDirty;
	
	void markDirty() {
		isDirty = true;
		propagateDirtyToChildren();
	}
	
	void propagateDirtyToChildren() {
		if (auto entity = getEntity()) {
			for (auto& child : entity->getChildren()) {
				if (child->hasComponent<Transform>()) {
					child->getComponent<Transform>().markDirty();
				}
			}
		}
	}
	
	void updateMatricesIfNeeded() const {
		if (isDirty) {
			updateMatrices();
			isDirty = false;
		}
	}
	
	void updateMatrices() const {
		localMatrix = glm::mat4(1.0f);
		localMatrix = glm::translate(localMatrix, localPosition);
		localMatrix = localMatrix * glm::mat4_cast(localQuaternion);
		localMatrix = glm::scale(localMatrix, localScale);
		
		if (auto entity = const_cast<Transform*>(this)->getEntity()) {
			if (auto parent = entity->getParent()) {
				if (parent->hasComponent<Transform>()) {
					auto& parentTransform = parent->getComponent<Transform>();
					worldMatrix = parentTransform.getMatrix() * localMatrix;
					return;
				}
			}
		}
		
		worldMatrix = localMatrix;
	}
};

class CameraComponent : public Component {
public:
	CameraComponent(int width = 800, int height = 600, float fov = 45.0f, float near = 0.1f, float far = 100.0f)
		: width(width), height(height), fov(fov), near(near), far(far), view(glm::mat4(1.0f)), projection(glm::mat4(1.0f)) {}
	
	void init() override {
		if (!getEntity()->hasComponent<Transform>()) {
			ERROR("Camera Component has no Transform, attaching empty one.");
			getEntity()->addComponent<Transform>();
		}

		update(0.0f); // Camera update doesnt use deltaTime
	}
	
	void update(float deltaTime) override {
		auto& transform = getEntity()->getComponent<Transform>();
		
		aspectRatio = (float)width / (float)height;
		
		glm::vec3 position = transform.getPosition();
		glm::vec3 forward = transform.getForward();
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		
		view = glm::lookAt(position, position + forward, up);
		projection = glm::perspective(glm::radians(fov), aspectRatio, near, far);
	}
	
	void setFov(float newFov) { fov = newFov; }
	void setNear(float newNear) { near = newNear; }
	void setFar(float newFar) { far = newFar; }
	void setFOV(float newFOV) { fov = newFOV; }
	void setAspectRatio(int n_width, int n_height) { 
		width = n_width; 
		height = n_height; 
	}
	
	float getFOV() const { return fov; }
	float getNear() const { return near; }
	float getFar() const { return far; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	float getAspectRatio() const { return aspectRatio; }
	
	const glm::mat4& getViewMatrix() const { return view; }
	const glm::mat4& getProjectionMatrix() const { return projection; }
	
private:
	int width, height;
	float aspectRatio;
	float near, far;
	float fov;
	
	glm::mat4 view;
	glm::mat4 projection;
};

class MeshRenderer : public Component {
public:
	MeshRenderer() {}
	MeshRenderer(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
		: mesh(mesh), material(material) {}
	
	void setMesh(std::shared_ptr<Mesh> mesh) { mesh = mesh; }
	void setMaterial(std::shared_ptr<Material> material) { material = material; }

	std::shared_ptr<Mesh> getMesh() { return mesh; }
	std::shared_ptr<Material> getMaterial() { return material; }
	
private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};

class ModelRenderer : public Component {
public:
	ModelRenderer() {}
	ModelRenderer(std::shared_ptr<Model> model) : m_model(model) {}
	
	void setModel(std::shared_ptr<Model> model) { m_model = model; }
	std::shared_ptr<Model> getModel() const {  return m_model; }
	
private:
	std::shared_ptr<Model> m_model;
};

class SkyboxRenderer : public Component {
public:
	SkyboxRenderer() {}
	SkyboxRenderer(std::vector<std::string> cubemapFaces) : cubemap(std::make_unique<CubemapTexture>(cubemapFaces)) {}
	
	void init() override {
		const auto& skyboxSource = ShaderLibrary::getShader("skybox");
		skyboxShader = std::make_unique<GLR::Shader>(
			skyboxSource.vertex,
			skyboxSource.fragment,
			true
		);

		// creating full screen quad
		std::vector<float> quadVertices = {
			-1.0f,  1.0f,  // top left
			-1.0f, -1.0f,  // bottom left
			 1.0f, -1.0f,  // bottom right
			 1.0f,  1.0f   // top right
		};
		
		std::vector<GLuint> indices = {
			0, 1, 2,  // first triangle
			0, 2, 3   // second triangle
		};
		
		// Convert to uint8_t vector
		std::vector<uint8_t> vertices;
		vertices.resize(quadVertices.size() * sizeof(float));
		std::memcpy(vertices.data(), quadVertices.data(), vertices.size());
		
		// vertex layout
		std::vector<VertexAttribute> layout = {
			{0, 2, GL_FLOAT, sizeof(float) * 2, 0} // Position attribute
		};
		
		mesh = std::make_shared<Mesh>();
		mesh->create(vertices, indices, layout);
		
		skyboxShader->bind();
		skyboxShader->setInt("skybox", 0);
		skyboxShader->unbind();
	}
			
	void render(const glm::mat4& view, const glm::mat4& projection) {
		glDepthMask(GL_FALSE);  // Dont write to depth buffer
		glDepthFunc(GL_LEQUAL); // Set skybox at maximum depth
		
		skyboxShader->bind();
		
		glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
		glm::mat4 invViewProjection = glm::inverse(projection * viewNoTranslation);
		skyboxShader->setMatrix4Float("invViewProjection", invViewProjection);
		
		cubemap->bind(0);
		
		mesh->draw();
		
		// Restore depth settings
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		
		GL_CHECK();
	}		
private:
	std::shared_ptr<CubemapTexture> cubemap;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Shader> skyboxShader;
};

class RigidBody : public Component {
public:
	enum class BodyType {
		STATIC,
		KINEMATIC,
		DYNAMIC
	};
	
	RigidBody(BodyType type = BodyType::DYNAMIC, bool useGravity = true) : bodyType(type), rigidBody(nullptr), useGravity(useGravity) {}
	
	~RigidBody() {
	    if (rigidBody && GetPhysicsWorld().isValid()) {
	        unsigned int numColliders = rigidBody->getNbColliders();
	        std::vector<reactphysics3d::Collider*> collidersToRemove;
	        collidersToRemove.reserve(numColliders);
	        
	        for (unsigned int i = 0; i < numColliders; ++i) {
	            collidersToRemove.push_back(rigidBody->getCollider(i));
	        }
	        
	        for (auto* collider : collidersToRemove) {
	            rigidBody->removeCollider(collider);
	        }
	        
	        GetPhysicsWorld().unregisterRigidBody(this);
	        GetPhysicsWorld().getPhysicsWorld()->destroyRigidBody(rigidBody);
	        rigidBody = nullptr;
	    }
	}
		
	void init() override {
		auto& transform = getEntity()->getComponent<Transform>();
		
		reactphysics3d::PhysicsWorld* world = GetPhysicsWorld().getPhysicsWorld();
		
		const glm::vec3& worldPosition = transform.getWorldPosition();
		const glm::quat& worldRotation = transform.getWorldRotation();
		
		reactphysics3d::Transform physicsTransform;
		physicsTransform.setPosition(reactphysics3d::Vector3(worldPosition.x, worldPosition.y, worldPosition.z));
		physicsTransform.setOrientation(reactphysics3d::Quaternion(worldRotation.x, worldRotation.y, worldRotation.z, worldRotation.w));
		
		// Create the rigid body
		rigidBody = world->createRigidBody(physicsTransform);
		rigidBody->setUserData(getEntity());
		
		switch (bodyType) {
			case BodyType::STATIC:
				rigidBody->setType(reactphysics3d::BodyType::STATIC);
				break;
			case BodyType::KINEMATIC:
				rigidBody->setType(reactphysics3d::BodyType::KINEMATIC);
				break;
			case BodyType::DYNAMIC:
				rigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
				break;
		}
		
		rigidBody->enableGravity(useGravity);
		
		GetPhysicsWorld().registerRigidBody(this);
		
		// After creating the rigid body find and attach all child colliders
		attachChildCollidersRecursive(getEntity());
	}
	
	void syncTransformFromPhysics() {
	    if (!isValid() || bodyType != BodyType::DYNAMIC) return;
	    
	    auto& transform = getEntity()->getComponent<Transform>();
	    
	    const reactphysics3d::Transform& physicsTransform = rigidBody->getTransform();
	    const reactphysics3d::Vector3& position = physicsTransform.getPosition();
	    const reactphysics3d::Quaternion& rotation = physicsTransform.getOrientation();

	    glm::vec3 worldPosition(position.x, position.y, position.z);
	    glm::quat worldRotation(rotation.w, rotation.x, rotation.y, rotation.z);
	    
	    transform.setWorldPosition(worldPosition);
	    transform.setWorldRotation(worldRotation);
	}

	void syncTransformToPhysics() {
	    if (!isValid()) return;
	    
	    auto& transform = getEntity()->getComponent<Transform>();
	    
	    const glm::vec3& worldPosition = transform.getWorldPosition();
	    const glm::quat& worldRotation = transform.getWorldRotation();
	    
	    reactphysics3d::Transform physicsTransform;
	    physicsTransform.setPosition(reactphysics3d::Vector3(worldPosition.x, worldPosition.y, worldPosition.z));
	    physicsTransform.setOrientation(reactphysics3d::Quaternion(worldRotation.x, worldRotation.y, worldRotation.z, worldRotation.w));

    	rigidBody->setTransform(physicsTransform);
	}
	
	// Force Application Methods
	void applyForce(const glm::vec3& force) {
		if (bodyType == BodyType::DYNAMIC && rigidBody) {
			rigidBody->applyWorldForceAtCenterOfMass(reactphysics3d::Vector3(force.x, force.y, force.z));
		}
	}

	void applyTorque(const glm::vec3& torque) {
		if (bodyType == BodyType::DYNAMIC && rigidBody) {
			rigidBody->applyWorldTorque(reactphysics3d::Vector3(torque.x, torque.y, torque.z));
		}
	}
	
	void applyForceAtPoint(const glm::vec3& force, const glm::vec3& point) {
		if (bodyType == BodyType::DYNAMIC && rigidBody) {
			rigidBody->applyWorldForceAtWorldPosition(
				reactphysics3d::Vector3(force.x, force.y, force.z),
				reactphysics3d::Vector3(point.x, point.y, point.z)
			);
		}
	}
	
	void applyLocalForce(const glm::vec3& force) {
		if (bodyType == BodyType::DYNAMIC && rigidBody) {
			rigidBody->applyLocalForceAtCenterOfMass(reactphysics3d::Vector3(force.x, force.y, force.z));
		}
	}
	
	void applyLocalTorque(const glm::vec3& torque) {
		if (bodyType == BodyType::DYNAMIC && rigidBody) {
			rigidBody->applyLocalTorque(reactphysics3d::Vector3(torque.x, torque.y, torque.z));
		}
	}
	
	void applyLocalForceAtPoint(const glm::vec3& force, const glm::vec3& localPoint) {
		if (bodyType == BodyType::DYNAMIC && rigidBody) {
			rigidBody->applyLocalForceAtLocalPosition(
				reactphysics3d::Vector3(force.x, force.y, force.z),
				reactphysics3d::Vector3(localPoint.x, localPoint.y, localPoint.z)
			);
		}
	}
	
	// Velocity Methods
	void setLinearVelocity(const glm::vec3& velocity) {
		if (rigidBody) {
			rigidBody->setLinearVelocity(reactphysics3d::Vector3(velocity.x, velocity.y, velocity.z));
		}
	}
	
	glm::vec3 getLinearVelocity() const {
		if (rigidBody) {
			const reactphysics3d::Vector3& vel = rigidBody->getLinearVelocity();
			return glm::vec3(vel.x, vel.y, vel.z);
		}
		return glm::vec3(0.0f);
	}
	
	void setAngularVelocity(const glm::vec3& velocity) {
		if (rigidBody) {
			rigidBody->setAngularVelocity(reactphysics3d::Vector3(velocity.x, velocity.y, velocity.z));
		}
	}
	
	glm::vec3 getAngularVelocity() const {
		if (rigidBody) {
			const reactphysics3d::Vector3& vel = rigidBody->getAngularVelocity();
			return glm::vec3(vel.x, vel.y, vel.z);
		}
		return glm::vec3(0.0f);
	}
	
	// Mass and Inertia Methods
	void setMass(float mass) {
		if (rigidBody && bodyType == BodyType::DYNAMIC) {
			rigidBody->setMass(mass);
		}
	}
	
	float getMass() const {
		if (rigidBody) {
			return rigidBody->getMass();
		}
		return 0.0f;
	}
	
	void setLocalInertiaTensor(const glm::vec3& inertia) {
		if (rigidBody && bodyType == BodyType::DYNAMIC) {
			rigidBody->setLocalInertiaTensor(reactphysics3d::Vector3(inertia.x, inertia.y, inertia.z));
		}
	}
	
	glm::vec3 getLocalInertiaTensor() const {
		if (rigidBody) {
			const reactphysics3d::Vector3& inertia = rigidBody->getLocalInertiaTensor();
			return glm::vec3(inertia.x, inertia.y, inertia.z);
		}
		return glm::vec3(0.0f);
	}
	
	// Damping Methods
	void setLinearDamping(float damping) {
		if (rigidBody) {
			rigidBody->setLinearDamping(damping);
		}
	}
	
	float getLinearDamping() const {
		if (rigidBody) {
			return rigidBody->getLinearDamping();
		}
		return 0.0f;
	}
	
	void setAngularDamping(float damping) {
		if (rigidBody) {
			rigidBody->setAngularDamping(damping);
		}
	}
	
	float getAngularDamping() const {
		if (rigidBody) {
			return rigidBody->getAngularDamping();
		}
		return 0.0f;
	}
	
	// Lock and Constraint Methods
	void setLinearLockAxisFactor(const glm::vec3& factors) {
		if (rigidBody) {
			rigidBody->setLinearLockAxisFactor(reactphysics3d::Vector3(factors.x, factors.y, factors.z));
		}
	}
	
	glm::vec3 getLinearLockAxisFactor() const {
		if (rigidBody) {
			const reactphysics3d::Vector3& factors = rigidBody->getLinearLockAxisFactor();
			return glm::vec3(factors.x, factors.y, factors.z);
		}
		return glm::vec3(1.0f);
	}
	
	void setAngularLockAxisFactor(const glm::vec3& factors) {
		if (rigidBody) {
			rigidBody->setAngularLockAxisFactor(reactphysics3d::Vector3(factors.x, factors.y, factors.z));
		}
	}
	
	glm::vec3 getAngularLockAxisFactor() const {
		if (rigidBody) {
			const reactphysics3d::Vector3& factors = rigidBody->getAngularLockAxisFactor();
			return glm::vec3(factors.x, factors.y, factors.z);
		}
		return glm::vec3(1.0f);
	}
	
	// Sleep Methods
	void setIsAllowedToSleep(bool allowed) {
		if (rigidBody) {
			rigidBody->setIsAllowedToSleep(allowed);
		}
	}
	
	bool isAllowedToSleep() const {
		if (rigidBody) {
			return rigidBody->isAllowedToSleep();
		}
		return false;
	}
	
	void setIsSleeping(bool sleeping) {
		if (rigidBody) {
			rigidBody->setIsSleeping(sleeping);
		}
	}
	
	bool isSleeping() const {
		if (rigidBody) {
			return rigidBody->isSleeping();
		}
		return false;
	}
	
	// Center of Mass Methods
	void setLocalCenterOfMass(const glm::vec3& center) {
		if (rigidBody && bodyType == BodyType::DYNAMIC) {
			rigidBody->setLocalCenterOfMass(reactphysics3d::Vector3(center.x, center.y, center.z));
		}
	}
	
	glm::vec3 getLocalCenterOfMass() const {
		if (rigidBody) {
			const reactphysics3d::Vector3& center = rigidBody->getLocalCenterOfMass();
			return glm::vec3(center.x, center.y, center.z);
		}
		return glm::vec3(0.0f);
	}
	
	// Gravity Methods
	void setUseGravity(bool useGravityValue) {
		useGravity = useGravityValue;
		if (rigidBody) {
			rigidBody->enableGravity(useGravityValue);
		}
	}
	
	bool getUseGravity() const {
		return useGravity;
	}
	
	// State Methods
	void setIsActive(bool active) {
		if (rigidBody) {
			rigidBody->setIsActive(active);
		}
	}
	
	bool isActive() const {
		if (rigidBody) {
			return rigidBody->isActive();
		}
		return false;
	}
	
	// Reset Methods
	void resetForce() {
		if (rigidBody) {
			rigidBody->resetForce();
		}
	}
	
	void resetTorque() {
		if (rigidBody) {
			rigidBody->resetTorque();
		}
	}
	
	reactphysics3d::RigidBody* getInternalBody() { return rigidBody; }
	BodyType getBodyType() const { return bodyType; }
	bool isValid() const { return rigidBody != nullptr && GetPhysicsWorld().isValid() && getEntity() != nullptr; }
	
private:
	BodyType bodyType;
	reactphysics3d::RigidBody* rigidBody;
	bool useGravity;

	void recreateCollidersOnEntity(Entity* entity);
	
	void attachChildCollidersRecursive(Entity* entity) {
		for (auto& child : entity->getChildren()) {
			// If child has its own rigid body skip it and its children
			if (child->hasComponent<RigidBody>()) {
				continue;
			}
			
			recreateCollidersOnEntity(child.get());
			attachChildCollidersRecursive(child.get());
		}
	}
};

inline void PhysicsWorld::update(float deltaTime) {
    std::vector<RigidBody*> validRigidBodies;
    validRigidBodies.reserve(rigidBodies.size());
    
    for (auto* rb : rigidBodies) {
        if (rb && rb->getInternalBody() && rb->getEntity()) {
            rb->syncTransformToPhysics();
            validRigidBodies.push_back(rb);
        }
    }

    physicsWorld->update(deltaTime);
    
    for (auto* rb : validRigidBodies) {
        if (rb && rb->getInternalBody() && rb->getEntity()) {
            rb->syncTransformFromPhysics();
        }
    }

    collisionListener->clearCollisionState();
}
	
class CollisionResponder : public Component, public ICollisionListener {
public:
	void init() override {
		GetPhysicsWorld().addCollisionListener(this);
	}
	
	~CollisionResponder() {
		if (GetPhysicsWorld().isValid()) {
			GetPhysicsWorld().removeCollisionListener(this);
		}
	}
	
	void onCollisionEnter(const CollisionEvent& event) override {
		if (event.entityA == getEntity() || event.entityB == getEntity()) {
			Entity* otherEntity = (event.entityA == getEntity()) ? event.entityB : event.entityA;
			handleCollisionEnter(otherEntity, event);
		}
	}
	
	void onCollisionExit(const CollisionEvent& event) override {
		if (event.entityA == getEntity() || event.entityB == getEntity()) {
			Entity* otherEntity = (event.entityA == getEntity()) ? event.entityB : event.entityA;
			handleCollisionExit(otherEntity, event);
		}
	}
	
	void onTriggerEnter(const CollisionEvent& event) override {
		if (event.entityA == getEntity() || event.entityB == getEntity()) {
			Entity* otherEntity = (event.entityA == getEntity()) ? event.entityB : event.entityA;
			handleTriggerEnter(otherEntity, event);
		}
	}
	
	void onTriggerExit(const CollisionEvent& event) override {
		if (event.entityA == getEntity() || event.entityB == getEntity()) {
			Entity* otherEntity = (event.entityA == getEntity()) ? event.entityB : event.entityA;
			handleTriggerExit(otherEntity, event);
		}
	}
	
protected:
	// Override these in derived classes for specific behavior
	virtual void handleCollisionEnter(Entity* otherEntity, const CollisionEvent& event) {}
	virtual void handleCollisionExit(Entity* otherEntity, const CollisionEvent& event) {}
	virtual void handleTriggerEnter(Entity* otherEntity, const CollisionEvent& event) {}
	virtual void handleTriggerExit(Entity* otherEntity, const CollisionEvent& event) {}
};

class Collider : public Component {
public:    
	virtual ~Collider() {
	    if (collider && rigidBody && rigidBody->getInternalBody() && GetPhysicsWorld().isValid()) {
	        try {
	            rigidBody->getInternalBody()->removeCollider(collider);
	        } 
	        catch (...) {
	            // Ignore errors during destruction
	        }
	    }
	    collider = nullptr;
	    rigidBody = nullptr;
	}
	
	virtual void createCollisionShape() = 0;	    

	void setMass(float mass) {
		reactphysics3d::Material& material = collider->getMaterial();
		material.setMassDensity(mass);
	}
	
	void setBounciness(float bounciness) {
		reactphysics3d::Material& material = collider->getMaterial();
		material.setBounciness(bounciness);
	}
	
	void setFriction(float friction) {
		reactphysics3d::Material& material = collider->getMaterial();
		material.setFrictionCoefficient(friction);
	}
	
	void setIsTrigger(bool isTrigger) {
		collider->setIsTrigger(isTrigger);
	}

	RigidBody* findParentRigidBody() {
		auto* entity = getEntity();
		if (!entity) return nullptr;
		
		// Check self first
		if (entity->hasComponent<RigidBody>()) {
			return &entity->getComponent<RigidBody>();
		}
		
		// Walk up the hierarchy
		auto parent = entity->getParent();
		while (parent) {
			if (parent->hasComponent<RigidBody>()) {
				return &parent->getComponent<RigidBody>();
			}
			parent = parent->getParent();
		}

		return nullptr;
	}

	float getMass() const { return collider->getMaterial().getMassDensity(); }
	float getBounciness() const { return collider->getMaterial().getBounciness(); }
	float getFriction() const { return collider->getMaterial().getFrictionCoefficient(); }
	bool getIsTrigger() const { return collider->getIsTrigger(); }

	reactphysics3d::Collider* getInternalCollider() { return collider; }

protected:
	reactphysics3d::Collider* collider;
	RigidBody* rigidBody; // Cached rigid body
};

class BoxCollider : public Collider {
public:
	BoxCollider(glm::vec3 halfExtents = glm::vec3(0.5f)) : halfExtents(halfExtents), shape(nullptr) {}
	
	~BoxCollider() {
		shape = nullptr;
	}
	
	void init() override {
		createCollisionShape();
	}
	
	void createCollisionShape() override {
		if (!getEntity()) return;
		
		RigidBody* rigidBodyPtr = findParentRigidBody();
		if (!rigidBodyPtr) {
			ERROR("BoxCollider requires a RigidBody component in the entity or a parent!");
			return;
		}
		
		rigidBody = rigidBodyPtr;
		auto* body = rigidBody->getInternalBody();
		if (!body) {
			ERROR("RigidBody has no internal body!");
			return;
		}
		
		// Create the box shape
		auto* physicsCommon = GetPhysicsWorld().getPhysicsCommon();
		shape = physicsCommon->createBoxShape(reactphysics3d::Vector3(halfExtents.x, halfExtents.y, halfExtents.z));
		
		// Add collider to the rigid body with identity transform
		reactphysics3d::Transform transform = reactphysics3d::Transform::identity();
		collider = body->addCollider(shape, transform);
	}
	
	glm::vec3 getHalfExtents() const { return halfExtents; }
	
private:
	reactphysics3d::BoxShape* shape;
	glm::vec3 halfExtents;
};

class SphereCollider : public Collider {
public:
	SphereCollider(float radius = 0.2f) : radius(radius), shape(nullptr) {}
	
	~SphereCollider() {
		shape = nullptr;
	}
	
	void init() override {
		createCollisionShape();
	}
	
	void createCollisionShape() override {
		if (!getEntity()) return;
		
		RigidBody* rigidBodyPtr = findParentRigidBody();
		if (!rigidBodyPtr) {
			ERROR("SphereCollider requires a RigidBody component in the entity or a parent!");
			return;
		}
		
		rigidBody = rigidBodyPtr;
		auto* body = rigidBody->getInternalBody();
		if (!body) {
			ERROR("RigidBody has no internal body!");
			return;
		}
		
		// Create the sphere shape
		auto* physicsCommon = GetPhysicsWorld().getPhysicsCommon();
		shape = physicsCommon->createSphereShape(radius);
		
		// Add collider to the rigid body with identity transform
		reactphysics3d::Transform transform = reactphysics3d::Transform::identity();
		collider = body->addCollider(shape, transform);
	}
	
	float getRadius() const { return radius; }
	
private:
	reactphysics3d::SphereShape* shape;
	float radius;
};

class CapsuleCollider : public Collider {
public:
	CapsuleCollider(float radius = 0.5f, float height = 1.0f) : radius(radius), height(height), shape(nullptr) {}
	
	~CapsuleCollider() {
		shape = nullptr;
	}
	
	void init() override {
		createCollisionShape();
	}
	
	void createCollisionShape() override {
		if (!getEntity()) return;
		
		RigidBody* rigidBodyPtr = findParentRigidBody();
		if (!rigidBodyPtr) {
			ERROR("CapsuleCollider requires a RigidBody component in the entity or a parent!");
			return;
		}
		
		rigidBody = rigidBodyPtr;
		auto* body = rigidBody->getInternalBody();
		if (!body) {
			ERROR("RigidBody has no internal body!");
			return;
		}
		
		// Create the capsule shape
		auto* physicsCommon = GetPhysicsWorld().getPhysicsCommon();
		shape = physicsCommon->createCapsuleShape(radius, height);
		
		// Add collider to the rigid body with identity transform
		reactphysics3d::Transform transform = reactphysics3d::Transform::identity();
		collider = body->addCollider(shape, transform);
	}
   
	float getRadius() const { return radius; }
	float getHeight() const { return height; }
	
private:
	reactphysics3d::CapsuleShape* shape;
	float radius;
	float height;
};

inline void RigidBody::recreateCollidersOnEntity(Entity* entity) {
	if (entity->hasComponent<BoxCollider>()) {
		auto& boxCollider = entity->getComponent<BoxCollider>();
		boxCollider.createCollisionShape();
	}
	
	if (entity->hasComponent<SphereCollider>()) {
		auto& sphereCollider = entity->getComponent<SphereCollider>();
		sphereCollider.createCollisionShape();
	}
	
	if (entity->hasComponent<CapsuleCollider>()) {
		auto& capsuleCollider = entity->getComponent<CapsuleCollider>();
		capsuleCollider.createCollisionShape();
	}
}

class Light : public Component {
public:
	Light(const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f) : color(color), intensity(intensity), isActive(true) {}
	
	virtual void init() override {}
	virtual void update(float deltaTime) override {}
	
	// Common light properties
	const glm::vec3& getColor() const { return color; }
	void setColor(const glm::vec3& color) { this->color = color; }
	
	float getIntensity() const { return intensity; }
	void setIntensity(float intensity) { this->intensity = intensity; }
	
	bool getIsActive() const { return isActive; }
	void setActive(bool active) { this->isActive = active; }
	
	// Each light type will implement this to set its specific shader uniforms
	virtual void setupLight(std::shared_ptr<Shader> shader, const std::string& prefix, int index) const = 0;
	
public:
	glm::vec3 color;
	float intensity;
	bool isActive;
};

class DirectionalLight : public Light {
public:
	DirectionalLight(
		const glm::vec3& direction = glm::vec3(0.0f, -1.0f, 0.0f),
		const glm::vec3& color = glm::vec3(1.0f),
		float intensity = 1.0f,
		bool castShadows = false,
		float size = 13.0f,
		float nearPlane = 50.0f,
		float farPlane = 200.0f,
		int shadowMapResolution = 2048
	) : Light(color, intensity), 
		direction(glm::normalize(direction)), 
		castShadows(castShadows),
		shadowMapResolution(shadowMapResolution),
		shadowBias(0.0005f),
		shadowMapInitialized(false),
		lightSpaceMatrix(1.0f) {
		
		orthoProjection.left = -size;
		orthoProjection.right = size;
		orthoProjection.bottom = -size;
		orthoProjection.top = size;
		orthoProjection.nearPlane = nearPlane;
		orthoProjection.farPlane = farPlane;
	}
	
	void init() override {
		if (castShadows && !shadowMapInitialized) {
			initShadowMap();
		}
	}
	
	void setupLight(std::shared_ptr<Shader> shader, const std::string& prefix, int index) const override {
		shader->setVector3Float(prefix + std::to_string(index) + "].direction", direction);
		shader->setVector3Float(prefix + std::to_string(index) + "].color", color);
		shader->setInt(prefix + std::to_string(index) + "].castShadows", castShadows ? 1 : 0);
		shader->setFloat(prefix + std::to_string(index) + "].intensity", intensity);
		
		if (castShadows) {
			shader->setMatrix4Float(prefix + std::to_string(index) + "].lightSpaceMatrix", lightSpaceMatrix);
			shader->setFloat(prefix + std::to_string(index) + "].shadowBias", shadowBias);
		}
	}
			
	// Shadow casting getters and setters
	void setCastShadows(bool castShadows) {
		this->castShadows = castShadows;
		if (this->castShadows && !shadowMapInitialized) {
			initShadowMap();
		}
	}
	bool getCastShadows() const { return castShadows; }
			
	// Shadow map resolution getters and setters
	void setShadowMapResolution(int resolution) {
		shadowMapResolution = resolution;
		if (shadowMapInitialized) {
			shadowMapInitialized = false;
			initShadowMap();
		}
	}
	int getShadowMapResolution() const { return shadowMapResolution; }
	
	// Orthographic projection getters and setters
	void setShadowOrthoProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
		orthoProjection.left = left;
		orthoProjection.right = right;
		orthoProjection.bottom = bottom;
		orthoProjection.top = top;
		orthoProjection.nearPlane = nearPlane;
		orthoProjection.farPlane = farPlane;
	}
	
	void setShadowOrthoSize(float size) {
		orthoProjection.left = -size;
		orthoProjection.right = size;
		orthoProjection.bottom = -size;
		orthoProjection.top = size;
	}
	
	void setShadowOrthoNearFar(float nearPlane, float farPlane) {
		orthoProjection.nearPlane = nearPlane;
		orthoProjection.farPlane = farPlane;
	}
	
	float getShadowOrthoLeft() const { return orthoProjection.left; }
	float getShadowOrthoRight() const { return orthoProjection.right; }
	float getShadowOrthoBottom() const { return orthoProjection.bottom; }
	float getShadowOrthoTop() const { return orthoProjection.top; }
	float getShadowOrthoNear() const { return orthoProjection.nearPlane; }
	float getShadowOrthoFar() const { return orthoProjection.farPlane; }
	
	float getShadowOrthoSize() const {
		// Assuming symmetric projection
		return orthoProjection.right;
	}
			
	void updateLightSpaceMatrix(const glm::vec3& sceneCenter, float sceneRadius) {
		auto& transform = getEntity()->getComponent<Transform>();

		glm::vec3 lightPos = transform.getPosition() - direction * sceneRadius * 2.0f;
		glm::mat4 lightView = glm::lookAt(lightPos, transform.getPosition(), glm::vec3(0.0f, 1.0f, 0.0f));
		
		glm::mat4 lightProjection = glm::ortho(
			orthoProjection.left, 
			orthoProjection.right, 
			orthoProjection.bottom, 
			orthoProjection.top, 
			orthoProjection.nearPlane, 
			orthoProjection.farPlane
		);
		
		// Light space matrix for shadow mapping
		lightSpaceMatrix = lightProjection * lightView;
	}

	std::shared_ptr<Texture> getShadowMap() { return shadowMap; }
	std::shared_ptr<FBO> getShadowMapFBO() { return shadowMapFBO; }
	
	const glm::mat4& getLightSpaceMatrix() const { return lightSpaceMatrix; }
	bool isShadowMapInitialized() const { return shadowMapInitialized; }

	void setDirection(const glm::vec3& direction) { this->direction = glm::normalize(direction); }
	const glm::vec3& getDirection() const { return direction; }

	void setShadowBias(float bias) { shadowBias = bias; }
	float getShadowBias() const { return shadowBias; }

private:
	glm::vec3 direction;
	bool castShadows;
	int shadowMapResolution;
	float shadowBias;
	bool shadowMapInitialized;
	
	// Shadow mapping
	std::shared_ptr<Texture> shadowMap;
	std::shared_ptr<FBO> shadowMapFBO;
	glm::mat4 lightSpaceMatrix;
	
	// Orthographic projection properties
	struct { float left, right, bottom, top, nearPlane, farPlane; } orthoProjection;
	
	void initShadowMap() {
		// Create depth texture for shadow mapping
		shadowMap = std::make_shared<Texture>(
			shadowMapResolution, 
			shadowMapResolution, 
			GL_DEPTH_COMPONENT, 
			GL_DEPTH_COMPONENT, 
			GL_FLOAT
		);
		
		glBindTexture(GL_TEXTURE_2D, shadowMap->getID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glBindTexture(GL_TEXTURE_2D, 0);
		GL_CHECK();
		
		shadowMapFBO = std::make_shared<FBO>();
		shadowMapFBO->attachTexture(shadowMap, GL_DEPTH_ATTACHMENT);
		shadowMapInitialized = true;
	}
};

class PointLight : public Light {
public:
	enum class FalloffType {
		CUSTOM,
		SHARP,
		NORMAL,
		SMOOTH,
		LINEAR,
		QUADRATIC
	};

	PointLight(
		float radius = 10.0f,
		const glm::vec3& color = glm::vec3(1.0f),
		float intensity = 1.0f,
		bool castShadows = false,
		FalloffType falloffType = FalloffType::SHARP,
		int shadowMapResolution = 1024
	) : Light(color, intensity), 
		radius(radius),
		castShadows(castShadows),
		shadowMapResolution(shadowMapResolution),
		shadowMapInitialized(false),
		nearPlane(0.1f),
		farPlane(radius * 3.0f),
		falloffType(falloffType),
		bias(0.001f) {
		updateAttenuationFactors();
	}
	
	void init() override {
		if (castShadows && !shadowMapInitialized) {
			initShadowMap();
		}
	}
	
	void setupLight(std::shared_ptr<Shader> shader, const std::string& prefix, int index) const override {
		auto& transform = getEntity()->getComponent<Transform>();
		glm::vec3 position = transform.getPosition();
		
		shader->setVector3Float(prefix + std::to_string(index) + "].position", position);
		shader->setVector3Float(prefix + std::to_string(index) + "].color", color);
		shader->setFloat(prefix + std::to_string(index) + "].intensity", intensity);
		shader->setFloat(prefix + std::to_string(index) + "].radius", radius);
		shader->setInt(prefix + std::to_string(index) + "].castShadows", castShadows ? 1 : 0);
		
		shader->setFloat(prefix + std::to_string(index) + "].constant", constant);
		shader->setFloat(prefix + std::to_string(index) + "].linear", linear);
		shader->setFloat(prefix + std::to_string(index) + "].quadratic", quadratic);
		
		if (castShadows) {
			shader->setFloat(prefix + std::to_string(index) + "].farPlane", farPlane);
			shader->setFloat(prefix + std::to_string(index) + "].shadowBias", bias);
		}
	}
	
	void setRadius(float radius) {
		this->radius = radius;
		farPlane = radius * 2.0f;
		if (falloffType != FalloffType::CUSTOM) {
			updateAttenuationFactors();
		}
	}
	
	// Attenuation management
	void setAttenuationFactors(float constant, float linear, float quadratic) {
		this->constant = constant;
		this->linear = linear;
		this->quadratic = quadratic;
		this->falloffType = FalloffType::CUSTOM;
	}
	
	void setFalloffType(FalloffType type) {
		this->falloffType = type;
		updateAttenuationFactors();
	}
	
	// Get current attenuation factors
	void getAttenuationFactors(float& outConstant, float& outLinear, float& outQuadratic) const {
		outConstant = constant;
		outLinear = linear;
		outQuadratic = quadratic;
	}
	
	void setCastShadows(bool castShadows) {
		this->castShadows = castShadows;
		if (this->castShadows && !shadowMapInitialized) {
			initShadowMap();
		}
	}
	
	glm::mat4 getShadowProjectionMatrix() const {
		return glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
	}
	
	std::vector<glm::mat4> getShadowViewMatrices(const glm::vec3& lightPos) const {
		std::vector<glm::mat4> viewMatrices;
		
		viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		viewMatrices.push_back(glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		
		return viewMatrices;
	}

	float getRadius() const { return radius; }
	bool getCastShadows() const { return castShadows; }
	int getShadowMapResolution() const { return shadowMapResolution; }
	bool getShadowMapInitialized() const { return shadowMapInitialized; }
	float getNearPlane() const { return nearPlane; }
	float getFarPlane() const { return farPlane; }
	FalloffType getFalloffType() const { return falloffType; }
	float getBias() const { return bias; }
	float getConstant() const { return constant; }
	float getLinear() const { return linear; }
	float getQuadratic() const { return quadratic; }
	GLuint getCubemapId() const { return shadowCubemap ? shadowCubemap->getID() : 0; }
	
	void setBias(float bias) { this->bias = bias; }
	void setShadowMapResolution(int resolution) { this->shadowMapResolution = resolution; }
	void setNearPlane(float nearPlane) { this->nearPlane = nearPlane; }
	void setFarPlane(float farPlane) { this->farPlane = farPlane; }
	void setConstant(float constant) { this->constant = constant; }
	void setLinear(float linear) { this->linear = linear; }
	void setQuadratic(float quadratic) { this->quadratic = quadratic; }
	
	std::shared_ptr<CubemapTexture> getShadowCubemap() { return shadowCubemap; }
	std::shared_ptr<FBO> getShadowMapFBO() { return shadowMapFBO; }

private:
	float radius;
	bool castShadows;
	int shadowMapResolution;
	bool shadowMapInitialized;
	float nearPlane;
	float farPlane;
	float bias;
	
	// Shadow mapping resources
	std::shared_ptr<CubemapTexture> shadowCubemap;
	std::shared_ptr<FBO> shadowMapFBO;
	
	// Attenuation factors
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;
	FalloffType falloffType;
	
	void updateAttenuationFactors() {
		switch (falloffType) {
			case FalloffType::SHARP:
				constant = 1.0f;
				linear = 4.5f / radius;
				quadratic = 75.0f / (radius * radius);
				break;
				
			case FalloffType::NORMAL:
				constant = 1.0f;
				linear = 2.0f / radius;
				quadratic = 1.0f / (radius * radius);
				break;
				
			case FalloffType::SMOOTH:
				constant = 1.0f;
				linear = 1.0f / radius;
				quadratic = 0.5f / (radius * radius);
				break;
				
			case FalloffType::LINEAR:
				constant = 1.0f;
				linear = 1.0f / radius;
				quadratic = 0.0f;
				break;
				
			case FalloffType::QUADRATIC:
				constant = 1.0f;
				linear = 0.0f;
				quadratic = 1.0f / (radius * radius);
				break;
				
			case FalloffType::CUSTOM:
				// the user has set custom values
				break;
		}
	}
	
	void initShadowMap() {
		shadowCubemap = std::make_shared<CubemapTexture>(shadowMapResolution, GL_DEPTH_COMPONENT, GL_FLOAT);
		
		// each face will be attached when rendering
		shadowMapFBO = std::make_shared<FBO>();
		
		shadowMapInitialized = true;
		GL_CHECK();
	}
};

class SpotLight : public Light {
public:
	enum class FalloffType {
		CUSTOM,
		SHARP,
		NORMAL,
		SMOOTH,
		LINEAR,
		QUADRATIC
	};

	SpotLight(
		float innerCutoffDegrees = 12.5f,
		float outerCutoffDegrees = 17.5f,
		float radius = 20.0f,
		const glm::vec3& color = glm::vec3(1.0f),
		float intensity = 1.0f,
		FalloffType falloffType = FalloffType::NORMAL
	) : Light(color, intensity),
		innerCutoffDegrees(innerCutoffDegrees),
		outerCutoffDegrees(outerCutoffDegrees),
		radius(radius),
		falloffType(falloffType) {
		
		innerCutoff = glm::cos(glm::radians(innerCutoffDegrees));
		outerCutoff = glm::cos(glm::radians(outerCutoffDegrees));
		
		updateAttenuationFactors();
	}
	
	void setupLight(std::shared_ptr<Shader> shader, const std::string& prefix, int index) const override {
		auto& transform = getEntity()->getComponent<Transform>();
		glm::vec3 position = transform.getPosition();
		glm::vec3 direction = transform.getForward();
		
		shader->setVector3Float(prefix + std::to_string(index) + "].position", position);
		shader->setVector3Float(prefix + std::to_string(index) + "].direction", direction);
		shader->setVector3Float(prefix + std::to_string(index) + "].color", color);
		shader->setFloat(prefix + std::to_string(index) + "].intensity", intensity);
		shader->setFloat(prefix + std::to_string(index) + "].radius", radius);
		shader->setFloat(prefix + std::to_string(index) + "].innerCutoff", innerCutoff);
		shader->setFloat(prefix + std::to_string(index) + "].outerCutoff", outerCutoff);
		
		// Set attenuation factors
		shader->setFloat(prefix + std::to_string(index) + "].constant", constant);
		shader->setFloat(prefix + std::to_string(index) + "].linear", linear);
		shader->setFloat(prefix + std::to_string(index) + "].quadratic", quadratic);
	}
	
	void setCutoffAngles(float innerDegrees, float outerDegrees) {
		innerCutoffDegrees = innerDegrees;
		outerCutoffDegrees = outerDegrees;
		
		innerCutoff = glm::cos(glm::radians(innerCutoffDegrees));
		outerCutoff = glm::cos(glm::radians(outerCutoffDegrees));
	}
	
	void setRadius(float radius) {
		this->radius = radius;
		if (falloffType != FalloffType::CUSTOM) {
			updateAttenuationFactors();
		}
	}
	
	// Attenuation
	void setAttenuationFactors(float constant, float linear, float quadratic) {
		this->constant = constant;
		this->linear = linear;
		this->quadratic = quadratic;
		this->falloffType = FalloffType::CUSTOM;
	}
	
	void setFalloffType(FalloffType type) {
		this->falloffType = type;
		updateAttenuationFactors();
	}
	
	// Get current attenuation factors
	void getAttenuationFactors(float& outConstant, float& outLinear, float& outQuadratic) const {
		outConstant = constant;
		outLinear = linear;
		outQuadratic = quadratic;
	}
	
	// Getters
	float getInnerCutoffDegrees() const { return innerCutoffDegrees; }
	float getOuterCutoffDegrees() const { return outerCutoffDegrees; }
	float getInnerCutoff() const { return innerCutoff; }
	float getOuterCutoff() const { return outerCutoff; }
	float getRadius() const { return radius; }
	FalloffType getFalloffType() const { return falloffType; }
	float getConstant() const { return constant; }
	float getLinear() const { return linear; }
	float getQuadratic() const { return quadratic; }
	
	// Setters
	void setConstant(float constant) { this->constant = constant; }
	void setLinear(float linear) { this->linear = linear; }
	void setQuadratic(float quadratic) { this->quadratic = quadratic; }

private:
	float innerCutoffDegrees;
	float outerCutoffDegrees;
	float innerCutoff;
	float outerCutoff;
	float radius;
	
	// Attenuation factors
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;
	FalloffType falloffType;
	
	void updateAttenuationFactors() {
		switch (falloffType) {
			case FalloffType::SHARP:
				constant = 1.0f;
				linear = 4.5f / radius;
				quadratic = 75.0f / (radius * radius);
				break;
				
			case FalloffType::NORMAL:
				constant = 1.0f;
				linear = 2.0f / radius;
				quadratic = 1.0f / (radius * radius);
				break;
				
			case FalloffType::SMOOTH:
				constant = 1.0f;
				linear = 1.0f / radius;
				quadratic = 0.5f / (radius * radius);
				break;
				
			case FalloffType::LINEAR:
				constant = 1.0f;
				linear = 1.0f / radius;
				quadratic = 0.0f;
				break;
				
			case FalloffType::QUADRATIC:
				constant = 1.0f;
				linear = 0.0f;
				quadratic = 1.0f / (radius * radius);
				break;
				
			case FalloffType::CUSTOM:
				// User has set custom values
				break;
		}
	}
};

/////////////////
//  Rendering  //
/////////////////

class Scene : public std::enable_shared_from_this<Scene> {
public:
	Scene() {}
	~Scene() {
		clearEntities();
	}

	std::shared_ptr<Entity> createEntity(const std::string& name = "Entity") {
		auto entity = std::make_shared<Entity>(name);
		entity->setParentScene(shared_from_this());
		entities.push_back(entity);
		return entity;
	}

	bool removeEntity(const Entity& entity) {
		auto it = std::find_if(entities.begin(), entities.end(),
			[&entity](const std::shared_ptr<Entity>& ptr) {
				return ptr.get() == &entity;
			});
		
		if (it != entities.end()) {
			return removeEntityWithChildren(*it);
		}
		return false;
	}
	bool removeEntity(std::shared_ptr<Entity> entity) {
		if (!entity) return false;
		
		auto it = std::find(entities.begin(), entities.end(), entity);
		if (it != entities.end()) {
			return removeEntityWithChildren(entity);
		}
		return false;
	}
	// Remove entity by name, removes first match
	bool removeEntity(const std::string& name) {
		auto it = std::find_if(entities.begin(), entities.end(),
			[&name](const std::shared_ptr<Entity>& entity) {
				return entity->getName() == name;
			});
		
		if (it != entities.end()) {
			return removeEntityWithChildren(*it);
		}
		return false;
	}

	void clearEntities() {
		entities.clear();
		mainCameraEntity = nullptr;
	}
	
	std::shared_ptr<Entity> findEntityByName(const std::string& name) {
		for (const auto& entity : entities) {
			if (entity->getName() == name) {
				return entity;
			}
		}
		return nullptr;
	}

	// getters
	std::vector<std::shared_ptr<Entity>>& getEntities() { 
		return entities; 
	}

	std::vector<std::shared_ptr<Entity>> getRootEntities() {
		std::vector<std::shared_ptr<Entity>> rootEntities;
		for (const auto& entity : entities) {
			if (!entity->getParent()) {
				rootEntities.push_back(entity);
			}
		}
		return rootEntities;
	}

	std::shared_ptr<GLR::Entity> getMainCameraEntity() {
		if (mainCameraEntity && mainCameraEntity->hasComponent<CameraComponent>()) {
			return mainCameraEntity;
		}
		return nullptr;
	}

	CameraComponent* getMainCameraComponent() {
		if (mainCameraEntity && mainCameraEntity->hasComponent<CameraComponent>()) {
			return &mainCameraEntity->getComponent<CameraComponent>();
		}
		return nullptr;
	}

	// setters
	void setMainCamera(std::shared_ptr<Entity> cameraEntity) {
		if (cameraEntity->hasComponent<CameraComponent>()) {
			mainCameraEntity = cameraEntity;
		}
	}

	void update(float deltaTime) {
		GetPhysicsWorld().update(deltaTime);
		updateEntitiesHierarchical(getRootEntities(), deltaTime);
	}

private:
	std::vector<std::shared_ptr<Entity>> entities;
	std::shared_ptr<Entity> mainCameraEntity;
	
	bool removeEntityWithChildren(std::shared_ptr<Entity> entity) {
		if (!entity) return false;
		
		auto descendants = entity->getAllDescendants();
		
		// Remove all descendants
		for (const auto& descendant : descendants) {
			auto it = std::find(entities.begin(), entities.end(), descendant);
			if (it != entities.end()) {
				entities.erase(it);
			}
		}
		
		// Remove itself
		auto it = std::find(entities.begin(), entities.end(), entity);
		if (it != entities.end()) {
			entities.erase(it);
			return true;
		}
		
		return false;
	}

	void updateEntitiesHierarchical(const std::vector<std::shared_ptr<Entity>>& entities, float deltaTime) {
		for (const auto& entity : entities) {

			entity->update(deltaTime);
			
			// model animation updates
			if (entity->hasComponent<ModelRenderer>()) {
				auto& renderer = entity->getComponent<ModelRenderer>();
				auto model = renderer.getModel();
				if (model) {
					if (model->getAnimationManager()) {
						model->getAnimationManager()->update(deltaTime);
					}
				}
			}
			
			// update children
			updateEntitiesHierarchical(entity->getChildren(), deltaTime);
		}
	}
};

class Renderer {
public:
	struct Settings {
	    // Debug settings
	    bool renderDebug = false;
	    bool wireframeMode = false;
	    
	    // Culling settings  
	    bool enableFaceCulling = true;
	    bool forceSingleSided = false;
	    bool frustumCulling = true;
	    
	    // Post processing settings
	    bool enablePostProcessing = true;
	    bool enableBloom = true;
	    float bloomIntensity = 1.0f;
	    float bloomThreshold = 1.0f;
	    
	    struct PostProcessingSettings {
	        // Tone mapping
	        float gamma = 2.2f;
	        float exposure = 1.0f;
	        
	        // Color enhancement
	        float saturation = 1.0f;
	        float contrast = 1.0f;  
	        float brightness = 0.0f;
	        float vibrancy = 0.0f;  
	        float colorBoost = 1.0f;
	        
	        // Vignette
	        float vignetteIntensity = 0.0f;
	        glm::vec3 vignetteColor = glm::vec3(0.0f);
	    } postProcessing;
	    
	    enum class RenderMode {
	        DEFAULT, ALBEDO, NORMAL, ROUGHNESS, METALLIC, LIGHT, SHADOW
	    };
	    RenderMode renderMode = RenderMode::DEFAULT;
	};

	Renderer(int windowWidth, int windowHeight) : windowWidth(windowWidth), windowHeight(windowHeight) {
		
		initDebugDrawer();
		initShaders();
		initResources();
		initLightingSystem();
		initFramebuffer();
		setupOpenGLState();
		
		applySettings();
	}

	~Renderer() {}
	
	void render(Scene& scene, Color clearColor) {
		CameraComponent* camera = scene.getMainCameraComponent();
		
		if (!camera) {
			ERROR("No camera available for rendering!");
			return;
		}
		
		resetStatistics();
		
		if (!settings.enablePostProcessing) {
			// Direct rendering to screen
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, windowWidth, windowHeight);
			glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			renderScene(scene);
		} 
		else {
			// Render to framebuffer then post process
			renderMainPass(scene, clearColor);
			renderPostProcess();
		}
	}
	
	void resize(int width, int height) {
		windowWidth = width;
		windowHeight = height;
		initFramebuffer();
	}
	
	// Settings management
	void updateSettings(const Settings& newSettings) {
		bool requiresFramebufferReinit = settingsRequireFramebufferReinit(settings, newSettings);
		
		settings = newSettings;
		applySettings();
		
		if (requiresFramebufferReinit) {
			initFramebuffer();
		}
	}

	void resetPostProcessingToDefaults() {
		settings.postProcessing = Settings::PostProcessingSettings{};
	}
	
	void applyPostProcessingPreset(const std::string& presetName) {
		auto& pp = settings.postProcessing;
		
		if (presetName == "natural") {
			pp.gamma = 2.2f;
			pp.exposure = 1.0f;
			pp.saturation = 1.0f;
			pp.contrast = 1.0f;
			pp.brightness = 0.0f;
			pp.vibrancy = 0.0f;
			pp.colorBoost = 1.0f;
		}
		else if (presetName == "vibrant") {
			pp.gamma = 2.0f;
			pp.exposure = 1.1f;
			pp.saturation = 1.5f;
			pp.contrast = 1.2f;
			pp.brightness = 0.02f;
			pp.vibrancy = 0.3f;
			pp.colorBoost = 1.1f;
		}
		else if (presetName == "cinematic") {
			pp.gamma = 2.4f;
			pp.exposure = 0.9f;
			pp.saturation = 1.2f;
			pp.contrast = 1.3f;
			pp.brightness = -0.01f;
			pp.vibrancy = 0.2f;
			pp.colorBoost = 1.05f;
		}
		else if (presetName == "stylized") {
			pp.gamma = 2.2f;
			pp.exposure = 1.0f;
			pp.saturation = 1.5f;
			pp.contrast = 1.2f;
			pp.brightness = 0.02f;
			pp.vibrancy = 0.3f;
			pp.colorBoost = 1.1f;
		}
	}
	
	Settings& getSettings() { return settings; }
	const Settings& getSettings() const { return settings; }
	
	// Accessors
	std::unique_ptr<GLR::DebugDrawer>& getDebugDrawer() { return debugDrawer; }
	std::shared_ptr<GLR::Texture>& getColorTexture() { return framebuffer.colorTexture; }
	
	// Statistics
	int getCulledMeshCount() const { return culledMeshCount; }
	int getCulledModelCount() const { return culledModelCount; }
	int getRenderedMeshCount() const { return renderedMeshCount; }
	int getRenderedModelCount() const { return renderedModelCount; }

	// Shaders
	std::shared_ptr<GLR::Shader> getObjectShader() { return shaders.object; }

private:
	// Core data
	Settings settings;
	int windowWidth, windowHeight;
	
	// All shaders
	struct Shaders {
		std::shared_ptr<GLR::Shader> object;
		std::shared_ptr<GLR::Shader> shadow;
		std::shared_ptr<GLR::Shader> pointShadow;
		std::shared_ptr<GLR::Shader> postProcess;
	} shaders;
	
	// Framebuffer resources
	struct FramebufferData {
		std::unique_ptr<GLR::FBO> sceneFBO;
		std::shared_ptr<GLR::Texture> colorTexture;
		std::shared_ptr<GLR::Texture> bloomTexture;
		std::shared_ptr<GLR::Texture> depthTexture;
	} framebuffer;
	
	// Lights
	struct Lights {
		static const int MAX_DIRECTIONAL = 2;
		static const int MAX_POINT = 16;
		static const int MAX_SPOT = 8;
		static const int MAX_SHADOW_CASTING_POINT = 4;
		
		std::vector<std::shared_ptr<DirectionalLight>> directional;
		std::vector<std::shared_ptr<PointLight>> point;
		std::vector<std::shared_ptr<SpotLight>> spot;
		std::unique_ptr<CubemapTexture> dummyPointShadowCubemap;
	} lights;
	
	// Frustum culling
	struct Frustum {
		glm::vec4 planes[6]; // Left, Right, Bottom, Top, Near, Far
		
		void extractFromMatrix(const glm::mat4& viewProjectionMatrix) {
			// Left plane
			planes[0] = glm::vec4(
				viewProjectionMatrix[0][3] + viewProjectionMatrix[0][0],
				viewProjectionMatrix[1][3] + viewProjectionMatrix[1][0],
				viewProjectionMatrix[2][3] + viewProjectionMatrix[2][0],
				viewProjectionMatrix[3][3] + viewProjectionMatrix[3][0]
			);
			
			// Right plane
			planes[1] = glm::vec4(
				viewProjectionMatrix[0][3] - viewProjectionMatrix[0][0],
				viewProjectionMatrix[1][3] - viewProjectionMatrix[1][0],
				viewProjectionMatrix[2][3] - viewProjectionMatrix[2][0],
				viewProjectionMatrix[3][3] - viewProjectionMatrix[3][0]
			);
			
			// Bottom plane
			planes[2] = glm::vec4(
				viewProjectionMatrix[0][3] + viewProjectionMatrix[0][1],
				viewProjectionMatrix[1][3] + viewProjectionMatrix[1][1],
				viewProjectionMatrix[2][3] + viewProjectionMatrix[2][1],
				viewProjectionMatrix[3][3] + viewProjectionMatrix[3][1]
			);
			
			// Top plane
			planes[3] = glm::vec4(
				viewProjectionMatrix[0][3] - viewProjectionMatrix[0][1],
				viewProjectionMatrix[1][3] - viewProjectionMatrix[1][1],
				viewProjectionMatrix[2][3] - viewProjectionMatrix[2][1],
				viewProjectionMatrix[3][3] - viewProjectionMatrix[3][1]
			);
			
			// Near plane
			planes[4] = glm::vec4(
				viewProjectionMatrix[0][3] + viewProjectionMatrix[0][2],
				viewProjectionMatrix[1][3] + viewProjectionMatrix[1][2],
				viewProjectionMatrix[2][3] + viewProjectionMatrix[2][2],
				viewProjectionMatrix[3][3] + viewProjectionMatrix[3][2]
			);
			
			// Far plane
			planes[5] = glm::vec4(
				viewProjectionMatrix[0][3] - viewProjectionMatrix[0][2],
				viewProjectionMatrix[1][3] - viewProjectionMatrix[1][2],
				viewProjectionMatrix[2][3] - viewProjectionMatrix[2][2],
				viewProjectionMatrix[3][3] - viewProjectionMatrix[3][2]
			);
			
			// Normalize all planes
			for (int i = 0; i < 6; i++) {
				float length = glm::length(glm::vec3(planes[i]));
				if (length > 0.0f) {
					planes[i] /= length;
				}
			}
		}
		
		bool isBoxInFrustum(const glm::vec3& center, const glm::vec3& halfExtents) const {
			for (int i = 0; i < 6; i++) {
				glm::vec3 normal = glm::vec3(planes[i]);
				float distance = planes[i].w;
				
				glm::vec3 positiveVertex = center + glm::vec3(
					(normal.x >= 0.0f) ? halfExtents.x : -halfExtents.x,
					(normal.y >= 0.0f) ? halfExtents.y : -halfExtents.y,
					(normal.z >= 0.0f) ? halfExtents.z : -halfExtents.z
				);
				
				if (glm::dot(normal, positiveVertex) + distance < 0.0f) {
					return false;
				}
			}
			
			return true;
		}
	} frustum;
	
	// Resources
	std::shared_ptr<GLR::Material> fallbackMaterial;
	std::unique_ptr<GLR::DebugDrawer> debugDrawer;
	std::shared_ptr<GLR::VAO> screenQuadVAO; 

	// Statistics
	mutable int culledMeshCount = 0;
	mutable int culledModelCount = 0;
	mutable int renderedMeshCount = 0;
	mutable int renderedModelCount = 0;
	
	// Initialization
	void initShaders() {

		const auto& shadowSource = ShaderLibrary::getShader("shadow");
		shaders.shadow = std::make_shared<GLR::Shader>(
			shadowSource.vertex,
			shadowSource.fragment,
			true
		);

		const auto& pointShadowSource = ShaderLibrary::getShader("point_shadow");
		shaders.pointShadow = std::make_shared<GLR::Shader>(
			pointShadowSource.vertex,
			pointShadowSource.fragment,
			true
		);

		const auto& postProcessSource = ShaderLibrary::getShader("postprocess");
		shaders.postProcess = std::make_shared<GLR::Shader>(
			postProcessSource.vertex,
			postProcessSource.fragment,
			true
		);

		const auto& objectSource = ShaderLibrary::getShader("main");
		shaders.object = std::make_shared<GLR::Shader>(
			objectSource.vertex,
			objectSource.fragment,
			true
		);
	}
	
	void initFramebuffer() {
		if (!settings.enablePostProcessing) return;
		
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		
		framebuffer.sceneFBO = std::make_unique<GLR::FBO>();

		// Color texture
		framebuffer.colorTexture = std::make_shared<GLR::Texture>(
			windowWidth, windowHeight, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE
		);
		
		if (framebuffer.colorTexture) {
			framebuffer.colorTexture->bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			framebuffer.colorTexture->unbind();
		}

		// Bloom texture (if enabled)
		if (settings.enableBloom) {
			framebuffer.bloomTexture = std::make_shared<GLR::Texture>(
				windowWidth, windowHeight, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE
			);
			
			if (framebuffer.bloomTexture) {
				framebuffer.bloomTexture->bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				framebuffer.bloomTexture->unbind();
			}
		}

		// Depth texture
		framebuffer.depthTexture = std::make_shared<GLR::Texture>(
			windowWidth, windowHeight, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT
		);
		
		if (framebuffer.depthTexture) {
			framebuffer.depthTexture->bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			framebuffer.depthTexture->unbind();
		}

		// Attach textures to FBO
		if (framebuffer.sceneFBO && framebuffer.colorTexture && framebuffer.depthTexture) {
			framebuffer.sceneFBO->attachTexture(framebuffer.colorTexture, GL_COLOR_ATTACHMENT0);
			
			if (settings.enableBloom && framebuffer.bloomTexture) {
				framebuffer.sceneFBO->attachTexture(framebuffer.bloomTexture, GL_COLOR_ATTACHMENT1);
				
				GLenum drawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
				framebuffer.sceneFBO->bind();
				glDrawBuffers(2, drawBuffers);
				framebuffer.sceneFBO->unbind();
				
				framebuffer.sceneFBO->finalize(2);
			} 
			else {
				framebuffer.sceneFBO->finalize(1);
			}
			
			framebuffer.sceneFBO->attachTexture(framebuffer.depthTexture, GL_DEPTH_ATTACHMENT);
			
			framebuffer.sceneFBO->bind();
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			framebuffer.sceneFBO->unbind();
		}
	}
	
	void initResources() {
		fallbackMaterial = std::make_shared<GLR::Material>(shaders.object);
		fallbackMaterial->setVector4("baseColorFactor", glm::vec4(0.988, 0.012, 0.972, 1.0));
		screenQuadVAO = std::make_shared<GLR::VAO>();
	}
	
	void initDebugDrawer() {
		debugDrawer = std::make_unique<GLR::DebugDrawer>();
		if (!debugDrawer->init()) {
			ERROR("Failed to initialize debug renderer");
		}
	}
	
	void initLightingSystem() {
		lights.dummyPointShadowCubemap = std::make_unique<CubemapTexture>(1, GL_DEPTH_COMPONENT, GL_FLOAT);

		lights.directional.clear();
		lights.point.clear();
		lights.spot.clear();
	}
	
	void setupOpenGLState() {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	
	// Main render pipeline
	void renderScene(Scene& scene) {
		CameraComponent* camera = scene.getMainCameraComponent();
		if (!camera) return;
		
		collectSceneLights(scene);
		setupFrustumCulling(camera);
		
		renderShadowMaps(scene);
		
		std::vector<std::shared_ptr<GLR::Entity>> opaqueEntities;
		std::vector<std::shared_ptr<GLR::Entity>> transparentEntities;
		std::shared_ptr<GLR::Entity> skyboxEntity = nullptr;
		
		categorizeEntities(scene, opaqueEntities, transparentEntities, skyboxEntity);
		
		// Render skybox first
		if (skyboxEntity) {
			glDepthMask(GL_FALSE);
			glDepthFunc(GL_LEQUAL);
			
			bool wasCullingEnabled = settings.enableFaceCulling;
			if (wasCullingEnabled) {
				glDisable(GL_CULL_FACE);
			}
			
			glm::mat4 viewMatrix = camera->getViewMatrix();
			viewMatrix[3][0] = viewMatrix[3][1] = viewMatrix[3][2] = 0.0f; 
			
			skyboxEntity->getComponent<SkyboxRenderer>().render(
				viewMatrix,
				camera->getProjectionMatrix()
			);
			
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);
			
			if (wasCullingEnabled) {
				glEnable(GL_CULL_FACE);
			}
		}
		
		applyWireframeSettings();
		
		// Render opaque objects
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		
		for (auto& entity : opaqueEntities) {
			renderEntity(entity, camera);
		}
		
		// Render transparent objects
		// if (!transparentEntities.empty()) {
		// 	sortTransparentEntities(transparentEntities, camera);
			
		// 	glEnable(GL_BLEND);
		// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// 	glDepthMask(GL_FALSE);
			
		// 	for (auto& entity : transparentEntities) {
		// 		renderEntity(entity, camera);
		// 	}
			
		// 	glDepthMask(GL_TRUE);
		// 	glDisable(GL_BLEND);
		// }

		if (!transparentEntities.empty()) {
		    sortTransparentEntities(transparentEntities, camera);
		    
		    glEnable(GL_BLEND);
		    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		    
		    // For blueprint effect, keep depth writing ON but use depth testing
		    glDepthMask(GL_TRUE);  // Changed from GL_FALSE
		    glEnable(GL_DEPTH_TEST);
		    glDepthFunc(GL_LEQUAL);
		    
		    for (auto& entity : transparentEntities) {
		        renderEntity(entity, camera);
		    }
		    
		    glDepthFunc(GL_LESS);
		    glDisable(GL_BLEND);
		}
				
		// Render debug visualizations
		if (settings.renderDebug) {
			renderDebugVisualization(scene, camera);
		}
	}
	
	void renderShadowMaps(Scene& scene) {
		renderDirectionalShadows(scene);
		renderPointShadows(scene);
	}
	
	void renderMainPass(Scene& scene, Color clearColor) {
		CameraComponent* camera = scene.getMainCameraComponent();
		if (!framebuffer.sceneFBO || !camera) return;
		
		framebuffer.sceneFBO->bind();
		glViewport(0, 0, windowWidth, windowHeight);
		
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		renderScene(scene);
		
		framebuffer.sceneFBO->unbind();
	}

	void renderPostProcess() {
	    if (!shaders.postProcess) return;
	    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	    glViewport(0, 0, windowWidth, windowHeight);
	    
	    glClear(GL_COLOR_BUFFER_BIT);
	    glDisable(GL_DEPTH_TEST);
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	    
	    shaders.postProcess->bind();
	    
	    // Bind textures
	    framebuffer.colorTexture->bind(0);
	    shaders.postProcess->setInt("screenTexture", 0);
	    
	    if (settings.enableBloom && framebuffer.bloomTexture) {
	        framebuffer.bloomTexture->bind(1);
	        shaders.postProcess->setInt("bloomTexture", 1);
	        shaders.postProcess->setInt("enableBloom", 1);
	        shaders.postProcess->setFloat("bloomIntensity", settings.bloomIntensity);
	    } 
	    else {
	        shaders.postProcess->setInt("enableBloom", 0);
	    }
	            
	    const auto& pp = settings.postProcessing;
	    
	    // Tone mapping
	    shaders.postProcess->setFloat("gamma", pp.gamma);
	    shaders.postProcess->setFloat("exposure", pp.exposure);
	    shaders.postProcess->setFloat("saturation", pp.saturation);
	    shaders.postProcess->setFloat("contrast", pp.contrast);
	    shaders.postProcess->setFloat("brightness", pp.brightness);
	    shaders.postProcess->setFloat("vibrancy", pp.vibrancy);
	    shaders.postProcess->setFloat("colorBoost", pp.colorBoost);
	    
	    // Vignette settings
	    shaders.postProcess->setFloat("vignetteIntensity", pp.vignetteIntensity);
	    shaders.postProcess->setVector3Float("vignetteColor", pp.vignetteColor);
	    
	    screenQuadVAO->bind();
	    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	    screenQuadVAO->unbind();
	    
	    shaders.postProcess->unbind();
	    framebuffer.colorTexture->unbind();
	    if (settings.enableBloom && framebuffer.bloomTexture) {
	        framebuffer.bloomTexture->unbind();
	    }
	    
	    glEnable(GL_DEPTH_TEST);
	    applyWireframeSettings();
	}
	
	// Entity rendering methods
	void renderEntity(std::shared_ptr<GLR::Entity> entity, CameraComponent* camera) {
		if (!entity || !camera) return;
		
		if (entity->hasComponent<MeshRenderer>()) {
			renderMeshEntity(entity, camera);
		} 
		else if (entity->hasComponent<ModelRenderer>()) {
			renderModelEntity(entity, camera);
		}
	}
	
	void renderMeshEntity(std::shared_ptr<GLR::Entity> entity, CameraComponent* camera) {
		if (!entity || !camera) return;
		
		auto& meshRenderer = entity->getComponent<MeshRenderer>();
		auto mesh = meshRenderer.getMesh();
		auto material = meshRenderer.getMaterial();
		
		if (!mesh) return;
		if (!material) { material = fallbackMaterial; }
		if (!material) return;
		
		setupCullingForMaterial(material);
		
		auto shader = material->getShader();
		if (!shader) return;

		auto& transform = entity->getComponent<Transform>();
		
		material->bind();
		shader->bind();
		shader->setMatrix4Float("view", camera->getViewMatrix());
		shader->setMatrix4Float("projection", camera->getProjectionMatrix());
		shader->setMatrix4Float("model", transform.getMatrix());
		shader->setVector3Float("viewPosition", camera->getEntity()->getComponent<GLR::Transform>().getPosition());
		shader->setInt("isSkinned", 0);
		shader->setFloat("bloomThreshold", settings.bloomThreshold);

		setShaderDebugMode(shader);
		setupLightingUniforms(shader);
		
		mesh->draw();
		
		shader->unbind();
		material->unbind();
		
		applyFaceCullingSettings();
	}
	
	void renderModelEntity(std::shared_ptr<GLR::Entity> entity, CameraComponent* camera) {
		if (!entity || !camera) return;
		
		auto& modelRenderer = entity->getComponent<ModelRenderer>();
		auto model = modelRenderer.getModel();
		if (!model) return;
		
		auto rootNodes = model->getRootNodes();
		if (rootNodes.empty()) {
			auto allNodes = model->getNodes();
			for (const auto& node : allNodes) {
				renderNodeMesh(node, entity, camera);
			}
		} 
		else {
			for (const auto& rootNode : rootNodes) {
				renderModelNode(rootNode, entity, camera);
			}
		}
	}
	
	void renderModelNode(std::shared_ptr<Node> node, std::shared_ptr<GLR::Entity> entity, CameraComponent* camera) {
		if (!node || !camera) return;
		
		if (node->getMesh()) {
			renderNodeMesh(node, entity, camera);
		}
		
		for (const auto& child : node->getChildren()) {
			renderModelNode(child, entity, camera);
		}
	}
	
	void renderNodeMesh(std::shared_ptr<Node> node, std::shared_ptr<GLR::Entity> entity, CameraComponent* camera) {
		if (!node || !node->getMesh() || !entity || !camera) return;
		
		auto& modelRenderer = entity->getComponent<ModelRenderer>();

		auto model = modelRenderer.getModel();
		auto material = node->getMaterial();

		if (!model) return;
		if (!material) { material = fallbackMaterial; }
		if (!material) return;

		setupCullingForMaterial(material);

		auto shader = material->getShader();
		
		material->bind();
		shader->bind();
		shader->setMatrix4Float("view", camera->getViewMatrix());
		shader->setMatrix4Float("projection", camera->getProjectionMatrix());
		shader->setVector3Float("viewPosition", camera->getEntity()->getComponent<GLR::Transform>().getPosition());

		auto& entityTransform = entity->getComponent<Transform>();
		glm::mat4 finalMatrix = entityTransform.getMatrix() * node->getMatrix();
		shader->setMatrix4Float("model", finalMatrix);
		
		bool hasSkins = model && !model->getSkins().empty();
		shader->setInt("isSkinned", hasSkins ? 1 : 0);
		
		if (hasSkins) {
			setupSkinningUniforms(shader, model);
		}

		shader->setFloat("bloomThreshold", settings.bloomThreshold);

		setShaderDebugMode(shader);
		setupLightingUniforms(shader);
		
		if (node->getMesh() && node->getMesh()->isValid()) {
			node->getMesh()->draw();
		}
		
		shader->unbind();
		material->unbind();
		
		applyFaceCullingSettings();
	}
	
	// Lighting helpers
	void collectSceneLights(Scene& scene) {
		lights.directional.clear();
		lights.point.clear();
		lights.spot.clear();
		
		for (auto& entity : scene.getEntities()) {
			if (!entity) continue;
			
			if (entity->hasComponent<DirectionalLight>()) {
				auto& light = entity->getComponent<DirectionalLight>();
				if (light.getIsActive() && lights.directional.size() < lights.MAX_DIRECTIONAL) {
					lights.directional.push_back(std::make_shared<DirectionalLight>(light));
				}
			}
			
			if (entity->hasComponent<PointLight>()) {
				auto& light = entity->getComponent<PointLight>();
				if (light.getIsActive() && lights.point.size() < lights.MAX_POINT) {
					lights.point.push_back(std::make_shared<PointLight>(light));
				}
			}
			
			if (entity->hasComponent<SpotLight>()) {
				auto& light = entity->getComponent<SpotLight>();
				if (light.getIsActive() && lights.spot.size() < lights.MAX_SPOT) {
					lights.spot.push_back(std::make_shared<SpotLight>(light));
				}
			}
		}
	}
	
	void setupLightingUniforms(std::shared_ptr<GLR::Shader> shader) {
		shader->setInt("directionalLightCount", static_cast<int>(lights.directional.size()));
		shader->setInt("pointLightCount", static_cast<int>(lights.point.size()));
		shader->setInt("spotLightCount", static_cast<int>(lights.spot.size()));
		
		// Setup directional lights
		for (size_t i = 0; i < lights.directional.size(); i++) {
			lights.directional[i]->setupLight(shader, "directionalLights[", i);
			
			if (lights.directional[i]->getCastShadows()) {
				GLint textureUnit = 4 + static_cast<GLint>(i);
				glActiveTexture(GL_TEXTURE0 + textureUnit);
				glBindTexture(GL_TEXTURE_2D, lights.directional[i]->getShadowMap()->getID());
				shader->setInt("directionalLightShadowMaps[" + std::to_string(i) + "]", textureUnit);
			}
		}

		// Setup point lights with shadow mapping
		int shadowCastingIndex = 0;
		for (size_t i = 0; i < lights.point.size(); i++) {
			lights.point[i]->setupLight(shader, "pointLights[", i);
			
			if (lights.point[i]->getCastShadows() && shadowCastingIndex < lights.MAX_SHADOW_CASTING_POINT) {
				shader->setInt("pointLights[" + std::to_string(i) + "].shadowMapIndex", shadowCastingIndex);
				
				GLint textureUnit = 4 + lights.MAX_DIRECTIONAL + shadowCastingIndex;
				glActiveTexture(GL_TEXTURE0 + textureUnit);
				glBindTexture(GL_TEXTURE_CUBE_MAP, lights.point[i]->getCubemapId());
				shader->setInt("pointLightShadowCubemaps[" + std::to_string(shadowCastingIndex) + "]", textureUnit);
				
				shadowCastingIndex++;
			} else {
				shader->setInt("pointLights[" + std::to_string(i) + "].shadowMapIndex", -1);
			}
		}
		
		// Bind dummy cubemaps to remaining slots
		for (int i = shadowCastingIndex; i < lights.MAX_SHADOW_CASTING_POINT; i++) {
			GLint textureUnit = 4 + lights.MAX_DIRECTIONAL + i;
			lights.dummyPointShadowCubemap->bind(textureUnit);
			shader->setInt("pointLightShadowCubemaps[" + std::to_string(i) + "]", textureUnit);
		}
		
		// Setup spot lights
		for (size_t i = 0; i < lights.spot.size(); i++) {
			lights.spot[i]->setupLight(shader, "spotLights[", i);
		}
	}
	
	// Culling helpers
	void setupFrustumCulling(CameraComponent* camera) {
		if (settings.frustumCulling && camera) {
			glm::mat4 viewProjection = camera->getProjectionMatrix() * camera->getViewMatrix();
			frustum.extractFromMatrix(viewProjection);
		}
	}
	
	bool isEntityInFrustum(std::shared_ptr<GLR::Entity> entity) {
		if (!entity || !entity->hasComponent<Transform>() || !settings.frustumCulling) {
			return true;
		}
		
		auto& transform = entity->getComponent<Transform>();
		glm::mat4 entityMatrix = transform.getMatrix();
		
		// Check MeshRenderer
		if (entity->hasComponent<MeshRenderer>()) {
			auto& meshRenderer = entity->getComponent<MeshRenderer>();
			auto mesh = meshRenderer.getMesh();
			
			if (mesh) {
				glm::vec3 center = glm::vec3(entityMatrix * glm::vec4(mesh->getCenter(), 1.0f));
				
				glm::vec3 scale = glm::vec3(
					glm::length(glm::vec3(entityMatrix[0])),
					glm::length(glm::vec3(entityMatrix[1])),
					glm::length(glm::vec3(entityMatrix[2]))
				);
				glm::vec3 halfExtents = mesh->getHalfExtents() * scale;
				
				bool inFrustum = frustum.isBoxInFrustum(center, halfExtents);
				if (!inFrustum) {
					culledMeshCount++;
				} 
				else {
					renderedMeshCount++;
				}
				return inFrustum;
			}
		}
		
		// Check ModelRenderer
		if (entity->hasComponent<ModelRenderer>()) {
			auto& modelRenderer = entity->getComponent<ModelRenderer>();
			auto model = modelRenderer.getModel();
			
			if (model) {
				glm::vec3 center = glm::vec3(entityMatrix * glm::vec4(model->getCenter(), 1.0f));
				
				glm::vec3 scale = glm::vec3(
					glm::length(glm::vec3(entityMatrix[0])),
					glm::length(glm::vec3(entityMatrix[1])),
					glm::length(glm::vec3(entityMatrix[2]))
				);
				glm::vec3 halfExtents = model->getHalfExtents() * scale;
				
				bool inFrustum = frustum.isBoxInFrustum(center, halfExtents);
				if (!inFrustum) {
					culledModelCount++;
				} 
				else {
					renderedModelCount++;
				}
				return inFrustum;
			}
		}
		
		return true;
	}
	
	bool isEntityTransparent(std::shared_ptr<GLR::Entity> entity) {
		if (!entity) return false;
		
		// Check MeshRenderer for transparency
		if (entity->hasComponent<MeshRenderer>()) {
			auto& meshRenderer = entity->getComponent<MeshRenderer>();
			auto material = meshRenderer.getMaterial();
			if (material) {
				return material->isTransparent();
			}
		}
		
		// Check ModelRenderer for transparency
		if (entity->hasComponent<ModelRenderer>()) {
			auto& modelRenderer = entity->getComponent<ModelRenderer>();
			auto model = modelRenderer.getModel();
			if (!model) return false;

			auto nodes = model->getNodes();
			for (auto& node : nodes) {
				if (node && node->getMesh()) {
					auto material = node->getMaterial();
					if (material && material->isTransparent()) {
						return true;
					}
				}
			}
		}
		
		return false;
	}
	
	void sortTransparentEntities(std::vector<std::shared_ptr<GLR::Entity>>& entities, CameraComponent* camera) {
		glm::vec3 cameraPos = camera->getEntity()->getComponent<GLR::Transform>().getPosition();
		
		std::sort(entities.begin(), entities.end(), 
			[&cameraPos](const std::shared_ptr<GLR::Entity>& a, const std::shared_ptr<GLR::Entity>& b) {
				if (!a || !b) return false;
				
				glm::vec3 posA(0.0f);
				glm::vec3 posB(0.0f);
				
				if (a->hasComponent<Transform>()) {
					posA = a->getComponent<Transform>().getPosition();
				}
				
				if (b->hasComponent<Transform>()) {
					posB = b->getComponent<Transform>().getPosition();
				}
				
				float distA = glm::distance2(cameraPos, posA);
				float distB = glm::distance2(cameraPos, posB);
				
				return distA > distB; // Sort back to front
			});
	}
	
	void categorizeEntities(Scene& scene, 
						  std::vector<std::shared_ptr<GLR::Entity>>& opaqueEntities,
						  std::vector<std::shared_ptr<GLR::Entity>>& transparentEntities,
						  std::shared_ptr<GLR::Entity>& skyboxEntity) {
		auto& entities = scene.getEntities();
		opaqueEntities.reserve(entities.size());
		transparentEntities.reserve(entities.size());
		
		for (auto& entity : entities) {
			if (!entity) continue;
			
			if (entity->hasComponent<SkyboxRenderer>()) {
				if (!skyboxEntity) {
					skyboxEntity = entity;
				}
			} 
			else if (isEntityTransparent(entity)) {
				if (isEntityInFrustum(entity)) {
					transparentEntities.push_back(entity);
				}
			} 
			else {
				if (isEntityInFrustum(entity)) {
					opaqueEntities.push_back(entity);
				}
			}
		}
	}
	
	// Shadow helpers
	void renderDirectionalShadows(Scene& scene) {
		if (!shaders.shadow || !hasDirectionalShadowCasters()) return;
		
		GLint previousFBO;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFBO);
		
		GLint originalViewport[4];
		glGetIntegerv(GL_VIEWPORT, originalViewport);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		glm::vec3 sceneCenter(0.0f);
		float sceneRadius = 50.0f;
		
		for (auto& light : lights.directional) {
			if (!light || !light->getCastShadows()) continue;
			
			light->updateLightSpaceMatrix(sceneCenter, sceneRadius);
			
			auto shadowMapFBO = light->getShadowMapFBO();
			auto shadowMap = light->getShadowMap();
			
			if (shadowMapFBO && shadowMap) {
				shadowMapFBO->bind();
				
				int shadowMapWidth = shadowMap->getWidth();
				int shadowMapHeight = shadowMap->getHeight();
				glViewport(0, 0, shadowMapWidth, shadowMapHeight);
				
				glClear(GL_DEPTH_BUFFER_BIT);
				
				renderSceneDepth(scene, light->getLightSpaceMatrix());
				
				shadowMapFBO->unbind();
			}
		}
		
		glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);
		glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
		applyWireframeSettings();
	}
	
	void renderPointShadows(Scene& scene) {
		if (!shaders.pointShadow || !hasPointShadowCasters()) return;
		
		GLint previousFBO;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFBO);
		
		GLint originalViewport[4];
		glGetIntegerv(GL_VIEWPORT, originalViewport);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		for (auto& entity : scene.getEntities()) {
			if (!entity || !entity->hasComponent<PointLight>()) continue;
			
			auto& light = entity->getComponent<PointLight>();
			if (!light.getCastShadows()) continue;
			
			auto& transform = entity->getComponent<Transform>();
			glm::vec3 lightPos = transform.getPosition();
			
			auto shadowMapFBO = light.getShadowMapFBO();
			if (shadowMapFBO) {
				shadowMapFBO->bind();
				
				glViewport(0, 0, light.getShadowMapResolution(), light.getShadowMapResolution());
				
				glm::mat4 shadowProj = light.getShadowProjectionMatrix();
				std::vector<glm::mat4> shadowViews = light.getShadowViewMatrices(lightPos);
				
				for (unsigned int face = 0; face < 6; ++face) {
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, light.getCubemapId(), 0);
					
					glClear(GL_DEPTH_BUFFER_BIT);
					
					glm::mat4 lightSpaceMatrix = shadowProj * shadowViews[face];
					
					renderSceneDepthPointLight(scene, lightSpaceMatrix, lightPos, light.getFarPlane());
				}
				
				shadowMapFBO->unbind();
			}
		}
		
		glViewport(originalViewport[0], originalViewport[1], originalViewport[2], originalViewport[3]);
		glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
		applyWireframeSettings();
	}
	
	void renderSceneDepth(Scene& scene, const glm::mat4& lightSpaceMatrix) {
		if (!shaders.shadow) return;
		
		shaders.shadow->bind();
		shaders.shadow->setMatrix4Float("lightSpaceMatrix", lightSpaceMatrix);
		
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		
		for (auto& entity : scene.getEntities()) {
			if (!entity) continue;
			
			renderEntityDepth(entity, shaders.shadow);
		}
		
		shaders.shadow->unbind();
		
		glCullFace(GL_BACK);
		applyFaceCullingSettings();
	}
	
	void renderSceneDepthPointLight(Scene& scene, const glm::mat4& lightSpaceMatrix, const glm::vec3& lightPos, float farPlane) {
		if (!shaders.pointShadow) return;
		
		shaders.pointShadow->bind();
		shaders.pointShadow->setMatrix4Float("lightSpaceMatrix", lightSpaceMatrix);
		shaders.pointShadow->setVector3Float("lightPos", lightPos);
		shaders.pointShadow->setFloat("farPlane", farPlane);
		
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		
		for (auto& entity : scene.getEntities()) {
			if (!entity) continue;
			
			renderEntityDepth(entity, shaders.pointShadow);
		}
		
		shaders.pointShadow->unbind();
		
		glCullFace(GL_BACK);
		applyFaceCullingSettings();
	}
	
	void renderEntityDepth(std::shared_ptr<GLR::Entity> entity, std::shared_ptr<GLR::Shader> depthShader) {
		if (!entity || !depthShader) return;
		
		if (entity->hasComponent<MeshRenderer>() && entity->hasComponent<Transform>()) {
			auto& meshRenderer = entity->getComponent<MeshRenderer>();
			auto mesh = meshRenderer.getMesh();
			
			if (mesh) {
				auto& transform = entity->getComponent<Transform>();
				glm::mat4 modelMatrix = transform.getMatrix();
				depthShader->setMatrix4Float("model", modelMatrix);
				depthShader->setInt("isSkinned", 0);
				
				mesh->draw();
			}
		}
		else if (entity->hasComponent<ModelRenderer>() && entity->hasComponent<Transform>()) {
			auto& modelRenderer = entity->getComponent<ModelRenderer>();
			auto& entityTransform = entity->getComponent<Transform>();
			auto model = modelRenderer.getModel();
			
			if (!model) return;
			
			bool hasSkins = model && !model->getSkins().empty();
			
			auto nodes = model->getNodes();
			for (auto& node : nodes) {
				if (node && node->getMesh()) {
					glm::mat4 finalMatrix = entityTransform.getMatrix() * node->getMatrix();
					
					depthShader->setMatrix4Float("model", finalMatrix);
					depthShader->setInt("isSkinned", hasSkins ? 1 : 0);
					
					if (hasSkins) {
						setupSkinningUniforms(depthShader, model);
					}
					
					if (node->getMesh()->isValid()) {
						node->getMesh()->draw();
					}
				}
			}
		}
	}
	
	// Material helpers
	void setupCullingForMaterial(std::shared_ptr<GLR::Material> material) {
		if (!material) return;
		
		bool isDoubleSided = material->isDoubleSided();
		
		if (settings.forceSingleSided) {
			isDoubleSided = false;
		}
		
		if (isDoubleSided) {
			glDisable(GL_CULL_FACE);
		} 
		else {
			if (settings.enableFaceCulling) {
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				glFrontFace(GL_CCW);
			} 
			else {
				glDisable(GL_CULL_FACE);
			}
		}
	}
	
	// Utility helpers
	void resetStatistics() {
		culledMeshCount = culledModelCount = 0;
		renderedMeshCount = renderedModelCount = 0;
	}
	
	bool hasDirectionalShadowCasters() const {
		for (auto& light : lights.directional) {
			if (light && light->getCastShadows()) {
				return true;
			}
		}
		return false;
	}
	
	bool hasPointShadowCasters() const {
		for (auto& light : lights.point) {
			if (light && light->getCastShadows()) {
				return true;
			}
		}
		return false;
	}
	
	void setShaderDebugMode(std::shared_ptr<GLR::Shader> shader) {
		if (!shader) return;
		
		int debugMode = 0;
		switch (settings.renderMode) {
			case Settings::RenderMode::DEFAULT:  debugMode = 0; break;
			case Settings::RenderMode::ALBEDO:   debugMode = 1; break;
			case Settings::RenderMode::NORMAL:   debugMode = 2; break;
			case Settings::RenderMode::ROUGHNESS: debugMode = 3; break;
			case Settings::RenderMode::METALLIC: debugMode = 4; break;
			case Settings::RenderMode::LIGHT:    debugMode = 5; break;
			case Settings::RenderMode::SHADOW:   debugMode = 6; break;
		}
		shader->setInt("debugMode", debugMode);
	}
	
	void setupSkinningUniforms(std::shared_ptr<GLR::Shader> shader, std::shared_ptr<Model> model) {
		if (!shader || !model || model->getSkins().empty()) return;
		
		auto skin = model->getSkins()[0];
		std::vector<glm::mat4> jointMatrices;
		jointMatrices.reserve(skin->joints.size());
		
		for (size_t i = 0; i < skin->joints.size(); i++) {
			auto jointNode = skin->joints[i].lock();
			if (jointNode) {
				glm::mat4 jointMatrix = jointNode->getMatrix() * skin->inverseBindMatrices[i];
				jointMatrices.push_back(jointMatrix);
			} 
			else {
				jointMatrices.push_back(glm::mat4(1.0f));
			}
		}
		
		for (size_t i = 0; i < jointMatrices.size() && i < 100; i++) {
			std::string uniformName = "jointMatrices[" + std::to_string(i) + "]";
			shader->setMatrix4Float(uniformName, jointMatrices[i]);
		}
	}
	
	// Debug helpers
	void renderDebugVisualization(Scene& scene, CameraComponent* camera) {
		if (!debugDrawer) return;
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_DEPTH_TEST);
		
		updateDebugVisualization(scene);
		
		debugDrawer->render(camera->getProjectionMatrix() * camera->getViewMatrix());
		applyWireframeSettings();
	}
	
	void updateDebugVisualization(Scene& scene) {
		for (const auto& entity : scene.getEntities()) {
			drawEntityDebug(entity);
		}
	}
	
	void drawEntityDebug(std::shared_ptr<GLR::Entity> entity) {
		drawPhysicsDebug(entity);
		drawModelDebug(entity);
		drawLightDebug(entity);
		drawCameraDebug(entity);
	}
	
	void drawPhysicsDebug(std::shared_ptr<GLR::Entity> entity) {
		bool hasCollider = entity->hasComponent<BoxCollider>() || entity->hasComponent<SphereCollider>() || entity->hasComponent<CapsuleCollider>();
		
		if (!hasCollider || !entity->hasComponent<Transform>()) {
			return;
		}
		
		auto& colliderEntityTransform = entity->getComponent<Transform>();
		glm::vec3 colliderEntityWorldPos = colliderEntityTransform.getWorldPosition();
		glm::quat colliderEntityWorldRot = colliderEntityTransform.getWorldRotation();
		
		glm::mat4 colliderEntityWorldMatrix = glm::translate(glm::mat4(1.0f), colliderEntityWorldPos) * glm::toMat4(colliderEntityWorldRot);
		
		if (entity->hasComponent<BoxCollider>()) {
			auto& boxCollider = entity->getComponent<BoxCollider>();
			debugDrawer->drawBox(colliderEntityWorldMatrix, boxCollider.getHalfExtents(), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		
		if (entity->hasComponent<SphereCollider>()) {
			auto& sphereCollider = entity->getComponent<SphereCollider>();
			debugDrawer->drawSphere(colliderEntityWorldPos, sphereCollider.getRadius(), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		
		if (entity->hasComponent<CapsuleCollider>()) {
			auto& capsuleCollider = entity->getComponent<CapsuleCollider>();
			
			glm::vec3 start = colliderEntityWorldPos + colliderEntityWorldRot * glm::vec3(0.0f, -capsuleCollider.getHeight() / 2.0f, 0.0f);
			glm::vec3 end = colliderEntityWorldPos + colliderEntityWorldRot * glm::vec3(0.0f, capsuleCollider.getHeight() / 2.0f, 0.0f);
			
			debugDrawer->drawCapsule(start, end, capsuleCollider.getRadius(), glm::vec3(0.0f, 0.0f, 1.0f));
		}
	}
	
	void drawModelDebug(std::shared_ptr<GLR::Entity> entity) {
		if (!entity->hasComponent<ModelRenderer>()) {
			return;
		}
		
		auto& modelRenderer = entity->getComponent<ModelRenderer>();
		auto model = modelRenderer.getModel();
		
		if (!model) {
			return;
		}
		
		auto& entityTransform = entity->getComponent<Transform>();
		glm::mat4 entityMatrix = entityTransform.getMatrix();
		
		// Draw nodes
		auto nodes = model->getNodes();
		for (auto node : nodes) {
			glm::mat4 finalTransform = entityMatrix * node->getMatrix();
			
			debugDrawer->drawSphere(glm::vec3(finalTransform[3]), 0.1f, glm::vec3(1.0f, 0.0f, 0.0f));
			debugDrawer->drawAxes(glm::vec3(finalTransform[3]), 0.2f);
		}
		
		// Draw joints and bones
		const auto& joints = model->getJoints();
		for (const auto& joint : joints) {
			auto node = joint.lock();
			if (!node) continue;
			
			glm::mat4 finalJointTransform = entityMatrix * node->getMatrix();
			
			glm::vec3 jointColor = glm::vec3(0.0f, 1.0f, 0.0f);
			debugDrawer->drawSphere(glm::vec3(finalJointTransform[3]), 0.05f, jointColor);
			debugDrawer->drawAxes(glm::vec3(finalJointTransform[3]), 0.1f);
			
			for (const auto& child : node->getChildren()) {
				bool isJoint = false;
				for (const auto& j : joints) {
					if (j.lock() == child) {
						isJoint = true;
						break;
					}
				}
				
				if (isJoint) {
					glm::mat4 finalChildTransform = entityMatrix * child->getMatrix();
					glm::vec3 boneColor = glm::vec3(1.0f, 0.5f, 0.0f);
					
					debugDrawer->drawBone(finalJointTransform, finalChildTransform, boneColor);
				}
			}
		}

		drawModelBoundingBox(model, entityMatrix);
	}
	
	void drawModelBoundingBox(std::shared_ptr<Model> model, const glm::mat4& entityMatrix) {
		glm::vec3 center = model->getCenter();
		glm::vec3 halfExtents = model->getHalfExtents();
		
		glm::vec3 worldCenter = glm::vec3(entityMatrix * glm::vec4(center, 1.0f));
		
		glm::vec3 scale = glm::vec3(glm::length(glm::vec3(entityMatrix[0])), glm::length(glm::vec3(entityMatrix[1])), glm::length(glm::vec3(entityMatrix[2])));
		
		glm::mat4 boundingBoxTransform = glm::translate(glm::mat4(1.0f), worldCenter);
		
		glm::mat3 rotationMatrix = glm::mat3(entityMatrix);
		rotationMatrix[0] = normalize(rotationMatrix[0]);
		rotationMatrix[1] = normalize(rotationMatrix[1]);
		rotationMatrix[2] = normalize(rotationMatrix[2]);
		boundingBoxTransform *= glm::mat4(rotationMatrix);
		
		glm::vec3 scaledHalfExtents = halfExtents * scale;
		
		glm::vec3 boundingBoxColor = glm::vec3(1.0f, 1.0f, 0.0f);
		debugDrawer->drawBox(boundingBoxTransform, scaledHalfExtents, boundingBoxColor);
		
		debugDrawer->drawSphere(worldCenter, 0.05f, glm::vec3(1.0f, 0.0f, 1.0f));
	}
	
	void drawLightDebug(std::shared_ptr<GLR::Entity> entity) {
		if (entity->hasComponent<DirectionalLight>()) {
			auto& light = entity->getComponent<DirectionalLight>();
			if (light.getIsActive()) {
				auto& transform = entity->getComponent<Transform>();
				glm::vec3 position = transform.getPosition();
				glm::vec3 direction = light.getDirection();
				
				glm::vec3 endPoint = position + direction * 2.0f;
				debugDrawer->drawLine(position, endPoint, glm::vec3(1.0f, 1.0f, 0.0f));
				debugDrawer->drawArrow(position, endPoint, glm::vec3(1.0f, 1.0f, 0.0f), 0.2f);
			}
		}
		
		if (entity->hasComponent<PointLight>()) {
			auto& light = entity->getComponent<PointLight>();
			if (light.getIsActive()) {
				auto& transform = entity->getComponent<Transform>();
				glm::vec3 position = transform.getPosition();
				
				debugDrawer->drawSphere(position, 0.2f, light.getColor());
				debugDrawer->drawSphere(position, light.getRadius(), glm::vec3(0.5f, 0.5f, 0.5f));
			}
		}
		
		if (entity->hasComponent<SpotLight>()) {
			auto& light = entity->getComponent<SpotLight>();
			if (light.getIsActive()) {
				auto& transform = entity->getComponent<Transform>();
				glm::vec3 position = transform.getPosition();
				glm::vec3 direction = transform.getForward();
				
				float outerAngle = glm::radians(light.getOuterCutoffDegrees());
				float radius = std::tan(outerAngle) * light.getRadius();
				debugDrawer->drawCone(position, direction, light.getRadius(), radius, light.getColor());
				debugDrawer->drawLine(position, position + direction * 2.0f, glm::vec3(1.0f, 1.0f, 0.0f));
			}
		}
	}
	
	void drawCameraDebug(std::shared_ptr<GLR::Entity> entity) {
		if (!entity->hasComponent<CameraComponent>()) {
			return;
		}
		
		auto& camera = entity->getComponent<CameraComponent>();
		auto& transform = entity->getComponent<Transform>();
		
		glm::vec3 position = transform.getPosition();
		glm::vec3 forward = transform.getForward();
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(forward, up));
		up = glm::normalize(glm::cross(right, forward));
		
		float fov = glm::radians(camera.getFOV());
		float aspect = camera.getAspectRatio();
		float distance = 2.0f;
		
		float halfFOVVertical = fov * 0.5f;
		float halfFOVHorizontal = std::atan(std::tan(halfFOVVertical) * aspect);
		
		glm::vec3 topDir = glm::normalize(forward + up * std::tan(halfFOVVertical));
		glm::vec3 bottomDir = glm::normalize(forward - up * std::tan(halfFOVVertical));
		glm::vec3 leftDir = glm::normalize(forward - right * std::tan(halfFOVHorizontal));
		glm::vec3 rightDir = glm::normalize(forward + right * std::tan(halfFOVHorizontal));
		
		debugDrawer->drawLine(position, position + topDir * distance, Color::toVec3(Color::Cyan()) * 0.6f);
		debugDrawer->drawLine(position, position + bottomDir * distance, Color::toVec3(Color::Cyan()) * 0.6f);
		debugDrawer->drawLine(position, position + leftDir * distance, Color::toVec3(Color::Cyan()) * 0.6f);
		debugDrawer->drawLine(position, position + rightDir * distance, Color::toVec3(Color::Cyan()) * 0.6f);
		
		glm::vec3 center = position + forward * distance;
		float frameHeight = distance * std::tan(halfFOVVertical) * 2.0f;
		float frameWidth = frameHeight * aspect;
		
		glm::vec3 topLeft = center + up * (frameHeight * 0.5f) - right * (frameWidth * 0.5f);
		glm::vec3 topRight = center + up * (frameHeight * 0.5f) + right * (frameWidth * 0.5f);
		glm::vec3 bottomLeft = center - up * (frameHeight * 0.5f) - right * (frameWidth * 0.5f);
		glm::vec3 bottomRight = center - up * (frameHeight * 0.5f) + right * (frameWidth * 0.5f);
		
		debugDrawer->drawLine(topLeft, topRight, Color::toVec3(Color::Cyan()) * 0.5f);
		debugDrawer->drawLine(topRight, bottomRight, Color::toVec3(Color::Cyan()) * 0.5f);
		debugDrawer->drawLine(bottomRight, bottomLeft, Color::toVec3(Color::Cyan()) * 0.5f);
		debugDrawer->drawLine(bottomLeft, topLeft, Color::toVec3(Color::Cyan()) * 0.5f);
	}
	
	// Settings helpers
	void applySettings() {
		applyFaceCullingSettings();
		applyWireframeSettings();
	}
	
	void applyFaceCullingSettings() {
		if (settings.enableFaceCulling) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);
		} 
		else {
			glDisable(GL_CULL_FACE);
		}
	}
	
	void applyWireframeSettings() {
		if (settings.wireframeMode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} 
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	
	bool settingsRequireFramebufferReinit(const Settings& oldSettings, const Settings& newSettings) {
		return oldSettings.enablePostProcessing != newSettings.enablePostProcessing || oldSettings.enableBloom != newSettings.enableBloom;
	}
};

}