// Author: JoeyDeVries
// Link: https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader_s.h

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "common.hpp"

class ComputeShader
{
public:
    unsigned int ID;
    std::string filePath;
    // default constructor creates an empty program object
    ComputeShader() : ID(0) {}

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    ComputeShader(const char* computePath) : filePath(computePath)
    {
        // 1. retrieve the compute shader source code from filePath
        std::string computeCode;
        std::ifstream cShaderFile;
        cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            cShaderFile.open(computePath);
            std::stringstream cShaderStream;
            cShaderStream << cShaderFile.rdbuf();
            cShaderFile.close();
            computeCode = cShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::COMPUTE_SHADER::FILE_NOT_FOUND\n"
                      << "Path: " << computePath << "\n"
                      << "Error: " << e.what() << "\n";
        }
        const char* cShaderCode = computeCode.c_str();

        // 2. compile compute shader
        unsigned int compute;
        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cShaderCode, NULL);
        glCompileShader(compute);
        checkCompileErrors(compute, "COMPUTE");

        // 3. create shader program
        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        
        // delete the shader as it's linked into our program now and no longer necessary
        glDeleteShader(compute);
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() 
    { 
        glUseProgram(ID); 
    }
    // ------------------------------------------------------------------------
    void dispatchCompute(unsigned int x, unsigned int y = 1, unsigned int z = 1, unsigned int invocationPerWorkgroup = common::INVOCATION_PER_WORKGROUP)
    {
        glDispatchCompute(ceilWithInvocationPerWorkgroup(x, invocationPerWorkgroup), 
                          ceilWithInvocationPerWorkgroup(y, invocationPerWorkgroup), 
                          ceilWithInvocationPerWorkgroup(z, invocationPerWorkgroup));
    }
    // ------------------------------------------------------------------------
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setUint(const std::string &name, unsigned int value) const
    {
        glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, glm::vec2 value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    // ------------------------------------------------------------------------
    void setIvec2(const std::string &name, glm::ivec2 value) const
    {
        glUniform2iv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, glm::vec3 value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, glm::mat3 value) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, glm::mat4 value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, const std::string& type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::COMPUTE_SHADER::COMPILATION_ERROR\n"
                          << "File: " << filePath << "\n"
                          << "Type: " << type << "\n"
                          << infoLog << "\n----------------------------------\n";
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR\n"
                          << "File: " << filePath << "\n"
                          << "Type: " << type << "\n"
                          << infoLog << "\n----------------------------------\n";
            }
        }
    }

    unsigned int ceilWithInvocationPerWorkgroup(unsigned int x, unsigned int invocationPerWorkgroup)
    {
        return static_cast<unsigned int>(ceil(static_cast<double>(x) / static_cast<double>(invocationPerWorkgroup)));
    }
};