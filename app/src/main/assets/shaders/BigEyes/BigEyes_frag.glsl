#version 300 es
precision highp float;

layout(binding = 0) uniform sampler2D m_Texture; //原图

in highp vec2 TexCoord;
out vec4 out_color;

uniform float has_face; //有人脸1，否则0
uniform highp vec2 points[98];
uniform lowp float u_ScaleRatio;//放大系数
uniform lowp float u_Radius;// 影响半径

// ************************************************************************************************
highp vec2 warpEyes(vec2 centerPos, vec2 curPos, float radius, float scaleRatio)
{
    highp vec2 ImgSize = vec2(float(textureSize(m_Texture,0).x), float(textureSize(m_Texture,0).y));

    highp vec2 result = curPos;
    highp vec2 imgCurPos = curPos * ImgSize;
    float d = distance(imgCurPos, centerPos);

    if (d < radius)
    {
        float gamma = 1.0 - scaleRatio * pow(smoothstep(0.0,1.0,d/radius)-1.0, 2.0);
        result = centerPos + gamma * (imgCurPos - centerPos);
        result = result / ImgSize;
    }
    return result;
}

// ************************************************************************************************
void main()
{
    if(has_face>0.1)
    {
//        //固定缩放半径
//        float eyeDistance = distance(points[96], points[97]); // 两个瞳孔的距离
//        float radius = eyeDistance/2.0;

        highp vec2 newTexCoord = warpEyes(points[96], TexCoord, u_Radius*1000.0, u_ScaleRatio);
        newTexCoord = warpEyes(points[97], newTexCoord, u_Radius*1000.0, u_ScaleRatio);
        out_color = texture(m_Texture, newTexCoord);
    }
    else
    {
        out_color = texture(m_Texture, TexCoord);
    }
}


























