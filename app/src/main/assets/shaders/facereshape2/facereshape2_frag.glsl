#version 300 es

layout(binding = 0) uniform sampler2D m_Texture; //原图

uniform float has_face; //有人脸1，否则0
uniform vec2 points[98];
uniform float Vstrength;
uniform float strength;

in mediump vec2 TexCoord;
out vec4 out_color;

// ************************************************************************************************
// 向一点拉伸            当前坐标              脸颊点                目标点              瞳距
vec2 curveWarp(vec2 textureCoord, vec2 originPosition, vec2 targetPosition, float radius)
{
    vec2 offset = vec2(0.0);
    vec2 result = vec2(0.0);
    vec2 direction = targetPosition - originPosition;
    float infect = distance(textureCoord, originPosition)/radius;
    infect = 1.0 - infect;
    infect = clamp(infect, 0.0, 1.0); //圆心:1, 圆上:0
    offset = direction * infect;
    result = textureCoord - offset;
    return result;
}

// ************************************************************************************************
// 瘦脸            像素坐标       两眼距离
vec2 faceLift(vec2 coord, float radius)
{
    vec2 currentPoint = vec2(0.0);
    vec2 destPoint = vec2(0.0);
    float VfaceLiftScale = Vstrength*0.05; //拉伸强度 原本无*0.01
    float faceLiftScale = strength*0.1; //拉伸强度 原本无*0.01

    //颊点的9个点（间隔4）
    for(int i=1; i<8; i++)
    {
        currentPoint = points[i*4];
        destPoint = currentPoint + (points[51] - currentPoint) * VfaceLiftScale; //原本没有*0.1
        coord = curveWarp(coord, currentPoint, destPoint, (1.0/32.0*pow(float(i),2.0)-1.0/4.0*float(i)+1.0)*radius); //越靠近下巴让半径越小，当i=4时最小到0.4倍数，当i=0和i=8时最大为1.0。是一个开口向上二次函数y=ax2+bx+c
    }
    return coord;
}

// ************************************************************************************************
void main()
{
//    vec4 tex = texture(m_Texture, TexCoord); //原图
//    out_color = tex;
//    out_color = vec4(1.0,0.0,0.0,1.0);

    // ************************************************************************************************
    // point索引 [0]:左脸颊, [1]:右脸颊, [2]:左下巴, [3]:右下巴, [4]:左眼, [5]:右眼, [6]:脸颊中点, [7]:下巴中点
    // ************************************************************************************************
    if(has_face>0.1)
    {
        // 将坐标转成图像大小，这里是为了方便计算
        vec2 coordinate = vec2(TexCoord.x*float(textureSize(m_Texture,0).x), TexCoord.y*float(textureSize(m_Texture,0).y));
        float eyeDistance = distance(points[96], points[97]); // 两个瞳孔的距离
        // 瘦脸
        coordinate = faceLift(coordinate, eyeDistance); //原本无*1.5
        // 转变回原来的纹理坐标系
        coordinate = vec2(coordinate.x/float(textureSize(m_Texture,0).x), coordinate.y/float(textureSize(m_Texture,0).y));
        // 输出图像
        out_color = texture(m_Texture, coordinate);
    }
    else
    {
        out_color = texture(m_Texture, TexCoord);
//        out_color = vec4(1.0,0.0,0.0,1.0);
    }


    // ************************************************************************************************
//    if(gl_FragCoord.y<points[7].y) //*float(textureSize(m_Texture,0).y)
//    {
//        out_color = vec4(1.0, 0.0, 0.0, 1.0);
//    }
//    else
//    {
//        out_color = texture(m_Texture, TexCoord);
//    }

}

