//
// Created by CS on 2022/12/7.
// 将FBO结果 普通2D渲染。
// 将原图与斑马纹进行透明度mix混合。
//

#version 300 es

layout(binding = 0) uniform sampler2D s_TextureFBO; //关键点

in mediump vec2 tex_coord;
out lowp vec4 out_color;

// ************************************************************************************************
void main()
{
    vec4 tex = texture(s_TextureFBO, tex_coord); //关键点

//    out_color = vec4(1.0,0.0,0.0,1.0);
    out_color = tex;
}
