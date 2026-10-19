#include <glm/glm.hpp>
#include <array>


struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
    
    // Vertex binding describes at which rate to load data from memory throughout the verticies (number of bytes)
    static vk::VertexInputBindingDescription get_binding_description()
    {
        return {.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex};
    }
    
    // Two attributes for vertices : position and color
    static std::array<vk::VertexInputAttributeDescription, 2> get_attribute_descriptions()
    {
        return {
            {
                {.location = 0, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, pos)},
                {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)}
            }
        };
    }
};

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};
