#version 300 es
layout(location = 0) in vec4 pos;
uniform mat4 frame_rotation;// 旋转矩阵
out vec2 TexCoord;
void main()
{
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    TexCoord = vec2(pos.z, pos.w);
}
