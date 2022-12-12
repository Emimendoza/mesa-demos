#version 450

layout(set = 0, binding = 0) uniform block {
    uniform mat4 projection;
};

layout(push_constant) uniform constants
{
    uniform mat4 modelview;
    vec3 material_color;
};

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec3 in_normal;

layout(location = 0) out vec4 out_color;

const vec3 L = normalize(vec3(5.0, 5.0, 10.0));

void main()
{
    vec3 N = normalize(mat3(modelview) * in_normal);

    float diffuse = max(0.0, dot(N, L));
    float ambient = 0.2;
    out_color = vec4((ambient + diffuse) * material_color, 1.0);

    gl_Position = projection * (modelview * in_position);
}
