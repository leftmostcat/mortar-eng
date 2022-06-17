#ifndef MORTAR_SHADER_H
#define MORTAR_SHADER_H

#include <GL/gl.h>
#include <vector>

enum Shader {
    UNLIT,
    SKIN,
    BASIC,
    SHADER_COUNT,
};

class EffectManager {
    public:
        EffectManager();
        ~EffectManager();

        int Initialize();
        GLuint GetShaderProgram(Shader shaderType);

    private:
        class ShaderProgram {
            public:
                ShaderProgram();
                ~ShaderProgram();

                int Initialize(const char *vertexShaderSrc, const char *fragmentShaderSrc);
                GLuint GetShaderProgram();

            private:
                GLuint program;
                GLuint vertexShader;
                GLuint fragmentShader;
        };

        std::vector<ShaderProgram *> shaderPrograms;
};

#endif
