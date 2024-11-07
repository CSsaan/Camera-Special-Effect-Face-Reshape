#version 300 es
#pragma debug(on)

layout(binding = 0) uniform sampler2D mask_Texture; //原图

uniform lowp float has_face; //有人脸1，否则0
//uniform lowp vec2 points[8];

in lowp vec2 TexCoord;
out lowp vec4 out_color;

// ************************************************************************************************
void main()
{
    //vec4 tex = texture(m_Texture, TexCoord); //原图
    if(has_face>0.98)
    {
        out_color = texture(mask_Texture, vec2(TexCoord.x,1.0-TexCoord.y)); //MASK
    }else{
        out_color = vec4(0.0);
    }
}

