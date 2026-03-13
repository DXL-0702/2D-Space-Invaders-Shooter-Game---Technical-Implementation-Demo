#include "shader.h"

#include <cstdio>

void validate_shader(GLuint shader, const char* label) {
    static const unsigned int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    GLsizei length = 0;
    buffer[0] = '\0';

    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
    if (length > 0) {
        std::fprintf(stderr, "Shader %u (%s) compile log:\n%s\n",
                     shader, (label ? label : ""), buffer);
    }
}

bool validate_program(GLuint program) {
    static const GLsizei BUFFER_SIZE = 1024;
    GLchar buffer[BUFFER_SIZE];
    GLsizei length = 0;
    buffer[0] = '\0';

    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
    if (length > 0) {
        std::fprintf(stderr, "Program %u link log:\n%s\n", program, buffer);
        return false;
    }
    return true;
}