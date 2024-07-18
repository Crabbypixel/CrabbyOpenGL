#pragma once

// GLM math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

#include <iostream>
#include <vector>
#include <string>
#include <string_view>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    // Positions
    glm::vec3 vPosition;
    // Normals
    glm::vec3 vNormal;
    // Texture coordinates
    glm::vec2 vTexCoords;
    // Tangents
    glm::vec3 vTangent;
    // Bitangents
    glm::vec3 vBitangent;
    // Bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    // Weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    // Mesh data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;
    unsigned int VAO;

    // Constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // Set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // Render the mesh
    void Draw(Shader& shader)
    {
        // Bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;

#ifdef CPP_STRING   
        std::string name;
        std::string number;

        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
            // Retrieve texture number (the N in diffuse_textureN)

            name = textures[i].type;

            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);  // Transfer unsigned int to string
            else if (name == "texture_normal")
                number = std::to_string(normalNr++);    // Transfer unsigned int to string
            else if (name == "texture_height")
                number = std::to_string(heightNr++);    // Transfer unsigned int to string

            // Now set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shader.id, (name + number).c_str()), i);

            // And finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
#endif
        
#ifndef CPP_STRING          
        constexpr int MAX_SIZE = 20;
        char name[MAX_SIZE];
        char number[3];

        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
            // Retrieve texture number (the N in diffuse_textureN)

            strcpy_s(name, static_cast<rsize_t>(MAX_SIZE)-1, textures[i].type.c_str());

            if (strcmp(name, "texture_diffuse") == 0)
                sprintf_s(number, 3, "%d", diffuseNr++);
            else if (strcmp(name, "texture_specular") == 0)
                sprintf_s(number, 3, "%d", specularNr++);
            else if (strcmp(name, "texture_normal") == 0)
                sprintf_s(number, 3, "%d", normalNr++);
            else if (strcmp(name, "texture_height") == 0)
                sprintf_s(number, 3, "%d", heightNr++);
            else
            {
                number[0] = '\0';
                std::cerr << "Error reading name1!" << std::endl;
            }

            // Now set the sampler to the correct texture unit
            strncat_s(name, MAX_SIZE, number, 3);
            glUniform1i(glGetUniformLocation(shader.id, name), i);

            // And finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
#endif
        // Bind vertex array
        glBindVertexArray(VAO);

        // Draw mesh
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        
        // Always good practice to set everything back to defaults once configured.
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // Render data 
    unsigned int VBO, EBO;

    // Initializes all the buffer objects/arrays
    void setupMesh()
    {
        // Create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // Load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Set the vertex attribute pointers
        // Vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // Vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vNormal));
        // Vertex texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vTexCoords));
        // Vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vTangent));
        // Vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vBitangent));
        // IDs
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // Weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};