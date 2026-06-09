#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class Sphere {
public:
    unsigned int vao, vbo, ebo;
    int indexCount;

    Sphere(float radius = 1.0f, int sectors = 36, int stacks = 18) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        float x, y, z, xy;                              // vertex position
        float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
        float s, t;                                     // vertex texCoord

        float sectorStep = 2 * glm::pi<float>() / sectors;
        float stackStep = glm::pi<float>() / stacks;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stacks; ++i) {
            stackAngle = glm::pi<float>() / 2 - i * stackStep;        // من pi/2 لـ -pi/2
            xy = radius * cosf(stackAngle);
            z = radius * sinf(stackAngle);

            for (int j = 0; j <= sectors; ++j) {
                sectorAngle = j * sectorStep;           // من 0 لـ 2pi

                // 1. الأماكن (Positions)
                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // 2. إحداثيات الصورة (Texture Coords)
                s = (float)j / sectors;
                t = (float)i / stacks;
                vertices.push_back(s);
                vertices.push_back(t);

                // 3. الـ Normals (مهمة جداً للإضاءة)
                // في الكرة، الـ Normal هو نفسه الـ Position بس مقسوم على الـ radius
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                vertices.push_back(nx);
                vertices.push_back(ny);
                vertices.push_back(nz);
            }
        }

        // توليد المثلثات (Indices)
        for (int i = 0; i < stacks; ++i) {
            int k1 = i * (sectors + 1);
            int k2 = k1 + sectors + 1;
            for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
                if (i != 0) {
                    indices.push_back(k1); indices.push_back(k2); indices.push_back(k1 + 1);
                }
                if (i != (stacks - 1)) {
                    indices.push_back(k1 + 1); indices.push_back(k2); indices.push_back(k2 + 1);
                }
            }
        }
        indexCount = (int)indices.size();

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // تعديل الـ Stride ليكون 8 (3 positions + 2 tex + 3 normals)
        int stride = (3 + 2 + 3) * sizeof(float);

        // Position (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        // TexCoord (location = 1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Normals (location = 2) - ده اللي كان ناقص!
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    void draw() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
};
#endif