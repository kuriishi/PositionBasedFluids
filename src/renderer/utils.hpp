#pragma once

#include <vector>
#include <string>

#include "glad/glad.h"

#include "../common/shader.hpp"

namespace renderer {
    namespace utils {
        GLuint loadTexture(const char* path);
        GLuint loadCubemap(std::vector<std::string> faces);

        GLuint generateTexture(GLuint minFilter, GLuint mgFilter, GLuint warpS, GLuint warpT,
                             GLuint internalFormat, GLuint width, GLuint height, GLuint format, 
                             GLuint type, GLuint* data);
        GLuint generateTextureR32F(GLuint width, GLuint height);
        GLuint generateTextureDepth32F(GLuint width, GLuint height);
        GLuint generateTextureR8I(GLuint width, GLuint height);
        GLuint generateTextureRGBA32F(GLuint width, GLuint height);

        void bindDepthAttachment(GLuint texture);
        void bindColorAttachment(GLuint texture, unsigned int target);

        void setDrawbuffersLayout(GLuint maxIndex);

        void bindTexture2D(Shader &shader, std::string textureName, GLuint texture, GLuint target);
        void bindTextureCubeMap(Shader &shader, std::string textureName, GLuint texture, GLuint target);

        // equal to `glBindImageTexture(0, texture, 0, GL_FALSE, 0, readOrWrite, format)`
        void bindTextureWithLayer0(GLuint texture, GLuint target, GLuint format, GLuint readOrWrite);

        extern GLuint screenQuadVAO;
        void initScreenQuad();
        void drawScreenQuad();

        // e.g. sizeInBytes = PARTICLE_COUNT * sizeof(glm::vec4)
        void copySSBO2VBO(GLuint ssbo, GLuint vbo, GLuint sizeInBytes);

        void copyTexture2D(GLuint srcTexture, GLuint dstTexture, GLuint width, GLuint height);
    }
}
