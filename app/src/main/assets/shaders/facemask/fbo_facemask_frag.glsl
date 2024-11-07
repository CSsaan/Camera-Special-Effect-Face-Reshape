#version 300 es

layout(binding = 0) uniform sampler2D m_Texture; //原图
layout(binding = 1) uniform sampler2D mask_Texture; //原图

uniform float has_face; //有人脸1，否则0
uniform vec2 points[8];
uniform float Vstrength;
uniform float strength;

in mediump vec2 TexCoord;
out vec4 out_color;

//// ************************************************************************************************
//// 向一点拉伸            当前坐标              脸颊点                目标点              瞳距
//vec2 curveWarp(vec2 textureCoord, vec2 originPosition, vec2 targetPosition, float radius)
//{
//    vec2 offset = vec2(0.0);
//    vec2 result = vec2(0.0);
//    vec2 direction = targetPosition - originPosition;
//    float infect = distance(textureCoord, originPosition)/radius;
//    infect = 1.0 - infect;
//    infect = clamp(infect, 0.0, 1.0); //圆心:1, 圆上:0
//    offset = direction * infect;
//    result = textureCoord - offset;
//    return result;
//}
//
//// ************************************************************************************************
//// 瘦脸            像素坐标       两眼距离
//vec2 faceLift(vec2 coord, float radius)
//{
//    vec2 currentPoint = vec2(0.0);
//    vec2 destPoint = vec2(0.0);
//    float VfaceLiftScale = Vstrength*0.1; //拉伸强度 原本无*0.01
//    float faceLiftScale = strength*0.1; //拉伸强度 原本无*0.01
//    //左脸颊点
//    currentPoint = points[0];
//    //currentPoint = currentPoint + (currentPoint-points[6])*0.1; //当前起始位置移到脸部外轮廓
//    destPoint = currentPoint + (points[6] - currentPoint) * VfaceLiftScale; //原本没有*0.1
//    coord = curveWarp(coord, currentPoint, destPoint, radius);
//    //右脸颊点
//    currentPoint = points[1];
//    //currentPoint = currentPoint + (currentPoint-points[6])*0.1; //当前起始位置移到脸部外轮廓
//    destPoint = currentPoint + (points[6] - currentPoint) * VfaceLiftScale; //原本没有*0.1
//    coord = curveWarp(coord, currentPoint, destPoint, radius);
//    //左下巴点
//    radius = radius * 0.8;
//    currentPoint = points[2]; //-vec2(0.0,0.8)
//    //currentPoint = points[2] + (points[2]-points[7]); //当前起始位置移到脸部外轮廓
//    destPoint = points[2] + (points[7] - points[2]) * (faceLiftScale*3.); // 3.0决定拉伸强度
//    coord = curveWarp(coord, currentPoint, destPoint, radius*0.5); // 0.5决定拉伸范围
//    //右下巴点
//    currentPoint = points[3]; //-vec2(0.0,0.8)
//    //currentPoint = points[3] + (points[3]-points[7]); //当前起始位置移到脸部外轮廓
//    destPoint = points[3] + (points[7] - points[3]) * (faceLiftScale*3.); // 3.0决定拉伸强度
//    coord = curveWarp(coord, currentPoint, destPoint, radius*0.5); // 0.5决定拉伸范围
//    return coord;
//}

// ************************************************************************************************
void main()
{

    //vec4 tex = texture(m_Texture, TexCoord); //原图
    vec4 MASK = texture(mask_Texture, vec2(TexCoord.x,1.0-TexCoord.y)); //MASK
    //out_color = mix(tex,MASK,1.0-MASK.r);
    if(has_face>0.1)
    {
        out_color = MASK;
    }else{
        out_color = vec4(0.0);
    }
    //out_color = vec4(1.0,0.0,0.0,1.0);

//    // ************************************************************************************************
//    // point索引 [0]:左脸颊, [1]:右脸颊, [2]:左下巴, [3]:右下巴, [4]:左眼, [5]:右眼, [6]:脸颊中点, [7]:下巴中点
//    // ************************************************************************************************
//    if(has_face>0.1)
//    {
//        // 将坐标转成图像大小，这里是为了方便计算
//        vec2 coordinate = vec2(TexCoord.x*float(textureSize(m_Texture,0).x), TexCoord.y*float(textureSize(m_Texture,0).y));
//        float eyeDistance = distance(points[4], points[5]); // 两个瞳孔的距离
//        // 瘦脸
//        coordinate = faceLift(coordinate, eyeDistance); //原本无*1.5
//        // 转变回原来的纹理坐标系
//        coordinate = vec2(coordinate.x/float(textureSize(m_Texture,0).x), coordinate.y/float(textureSize(m_Texture,0).y));
//        // 输出图像
//        out_color = texture(m_Texture, coordinate);
//    }
//    else
//    {
//        out_color = texture(m_Texture, TexCoord);
//    }
//
//
//    // ************************************************************************************************
////    if(gl_FragCoord.y<points[7].y) //*float(textureSize(m_Texture,0).y)
////    {
////        out_color = vec4(1.0, 0.0, 0.0, 1.0);
////    }
////    else
////    {
////        out_color = texture(m_Texture, TexCoord);
////    }

}

