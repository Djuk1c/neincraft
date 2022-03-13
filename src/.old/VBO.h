// Vertex buffer object
#pragma once
#include <glad/glad.h>

class VBO
{
    public:
        unsigned int ID;
        VBO(float* vertices, GLsizeiptr size);

        void Bind();
        void Unbind();
        void Delete();
};
