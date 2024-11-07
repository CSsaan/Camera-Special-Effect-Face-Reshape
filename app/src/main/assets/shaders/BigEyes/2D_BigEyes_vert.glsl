//
// Created by CS on 2022/12/7.
//

#version 300 es
//layout(location = 0) in vec4 vert_pos;
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
uniform lowp mat4 frame_rotation;  // 旋转矩阵，因为画面是有旋转的
out lowp vec2 tex_coord;

void main()
{
    //1.直接带入公式计算法
    gl_Position = vec4(aPos.x, -aPos.y, 0.0, 1.0);
    tex_coord = aTexCoord;
}


