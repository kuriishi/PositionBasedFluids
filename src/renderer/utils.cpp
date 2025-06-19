#include "utils.hpp"


#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include <iostream>

#include "../common/common.hpp"
#include "../../include/stb_image.h"

namespace renderer {
    namespace utils {
        unsigned int loadTexture(const char* path) {
            unsigned int textureID;
            glGenTextures(1, &textureID);

            int width, height, nrComponents;
            unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
            if (data) {
                GLenum format;
                if (nrComponents == 1)
                    format = GL_RED;
                else if (nrComponents == 3)
                    format = GL_RGB;
                else if (nrComponents == 4)
                    format = GL_RGBA;

                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            else {
                std::cerr << "Texture failed to load at path: " << path << std::endl;
            }

            stbi_image_free(data);

            return textureID;
        }

        unsigned int loadCubemap(std::vector<std::string> faces) {
            unsigned int textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        
            int width, height, nrChannels;
            for (unsigned int i = 0; i < faces.size(); i++) {
                unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
                if (data) {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                }
                else {
                    std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                }
                stbi_image_free(data);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            return textureID;
        }

        GLuint generateTexture(GLuint minFilter, GLuint mgFilter, GLuint warpS, GLuint warpT,
                             GLuint internalFormat, GLuint width, GLuint height, GLuint format, GLuint type, GLuint* data) {
            GLuint id;
            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mgFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, warpS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, warpT);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);

            return id;
        }

        GLuint generateTextureR32F(GLuint width, GLuint height) {
            return generateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 
                                   GL_R32F, width, height, GL_RED, GL_FLOAT, NULL);
        }

        GLuint generateTextureDepth32F(GLuint width, GLuint height) {
            return generateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 
                                   GL_DEPTH_COMPONENT32F, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }

        GLuint generateTextureR8I(GLuint width, GLuint height) {
            return generateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 
                                   GL_R8I, width, height, GL_RED_INTEGER, GL_BYTE, NULL);
        }
        
        GLuint generateTextureRGBA32F(GLuint width, GLuint height) {
            return generateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 
                                   GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT, NULL);
        }
        
        void bindDepthAttachment(GLuint texture) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
        }

        void bindColorAttachment(GLuint texture, unsigned int target) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + target, GL_TEXTURE_2D, texture, 0);
        }

        void setDrawbuffersLayout(GLuint maxIndex) {
            GLenum* drawbuffers = new GLenum[maxIndex + 1];
            for (GLuint i = 0; i <= maxIndex; i++) {
                drawbuffers[i] = GL_COLOR_ATTACHMENT0 + i;
            }

            glDrawBuffers(maxIndex + 1, drawbuffers);

            delete[] drawbuffers;
        }

        void bindTexture2D(Shader &shader, std::string textureName, GLuint texture, GLuint target) {
            shader.setInt(textureName, target);
            glActiveTexture(GL_TEXTURE0 + target);
            glBindTexture(GL_TEXTURE_2D, texture);
        }

        void bindTextureCubeMap(Shader &shader, std::string textureName, GLuint texture, GLuint target) {
            shader.setInt(textureName, target);
            glActiveTexture(GL_TEXTURE0 + target);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        }

        void bindTextureWithLayer0(GLuint texture, GLuint target, GLuint format, GLuint readOrWrite) {
            glBindImageTexture(target, texture, 0, GL_FALSE, 0, readOrWrite, format);
        }

        // screen quad
        const std::vector<float> screenQuadVertices = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
        };
        GLuint screenQuadVAO;
        GLuint screenQuadVBO;
        void initScreenQuad() {
            if (screenQuadVAO != 0) {
                return;
            }

            glGenVertexArrays(1, &screenQuadVAO);
            glGenBuffers(1, &screenQuadVBO);

            glBindVertexArray(screenQuadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
            glBufferData(GL_ARRAY_BUFFER, screenQuadVertices.size() * sizeof(float), screenQuadVertices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        void drawScreenQuad() {
            if (screenQuadVAO == 0) {
                initScreenQuad();
            }

            glBindVertexArray(screenQuadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }

        void copySSBO2VBO(GLuint ssbo, GLuint vbo, GLuint sizeInBytes) {
            glBindBuffer(GL_COPY_READ_BUFFER, ssbo);
            glBindBuffer(GL_COPY_WRITE_BUFFER, vbo);
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeInBytes);
            glBindBuffer(GL_COPY_READ_BUFFER, 0);
            glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
        }

        void copyTexture2D(GLuint srcTexture, GLuint dstTexture, GLuint width, GLuint height) {
            glCopyImageSubData(
                srcTexture, GL_TEXTURE_2D, 0, 
                0, 0, 0,
                dstTexture, GL_TEXTURE_2D, 0,
                0, 0, 0,
                width, height, 1
            );

            glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
        }
    }
}
