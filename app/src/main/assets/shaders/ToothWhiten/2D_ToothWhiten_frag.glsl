//
// Created by CS on 2022/12/7.
// 将FBO结果 普通2D渲染。
// 将原图与斑马纹进行透明度mix混合。
//

#version 300 es
#pragma debug(on)

layout(binding = 0) uniform sampler2D s_TextureMASK; //MAKS
layout(binding = 1) uniform sampler2D s_Texture; //原图
layout(binding = 2) uniform sampler2D s_TextureLUT; //关键点

uniform lowp float strength;

in lowp vec2 tex_coord;
out lowp vec4 out_color;

// ************************************************************************************************
vec4 LUT4x4(vec4 inColor, sampler2D lutImageTexture)
{
    highp float blueColor = inColor.b * 15.0;
    highp vec2 quad1;
    quad1.y = floor(floor(blueColor) / 4.0);
    quad1.x = floor(blueColor) - (quad1.y * 4.0);
    highp vec2 quad2;
    quad2.y = floor(ceil(blueColor) / 3.9999);
    quad2.x = ceil(blueColor) - (quad2.y * 4.0);
    highp vec2 texPos1;
    texPos1.x = (quad1.x * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * inColor.r);
    texPos1.y = (quad1.y * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * inColor.g);
    highp vec2 texPos2;
    texPos2.x = (quad2.x * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * inColor.r);
    texPos2.y = (quad2.y * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * inColor.g);
    lowp vec4 newColor2_1 = texture(lutImageTexture, texPos1);
    lowp vec4 newColor2_2 = texture(lutImageTexture, texPos2);
    lowp vec4 newColor22 = mix(newColor2_1, newColor2_2, fract(blueColor));
    return newColor22;
}

// ************************************************************************************************
void main()
{
    lowp vec4 tex = texture(s_Texture, tex_coord); //原图
    lowp vec4 whiten_dstColor = LUT4x4(tex, s_TextureLUT);

    lowp vec4 tex_MASK = texture(s_TextureMASK, tex_coord); //MASK

    out_color = mix(tex,whiten_dstColor, strength*pow(tex_MASK.r,2.0));
    //out_color = whiten_dstColor;
}
