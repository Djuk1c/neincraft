#pragma once

#include <glad/glad.h>
#include "../include/VBO.h"

class VAO
{
    public:
        unsigned int ID;
        VAO();

        void LinkVBO(VBO& VBO);
        void Bind();
        void Unbind();
        void Delete();
};
