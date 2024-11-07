#version 300 es

layout(binding = 0) uniform sampler2D y_samp;
layout(binding = 1) uniform sampler2D u_samp;
layout(binding = 2) uniform sampler2D v_samp;

in vec2 tex_coord;
out vec4 out_color;

// ************************************************************************************************
// *YUV转RGB*
vec3 YUV2RGB(vec2 Coord)
{
    vec3 rgb = mat3(1.0, 1.0, 1.0,
                    0, -0.344, 1.770,
                    1.403, -0.714, 0.0)
            * vec3(texture(y_samp, Coord).r,
                    texture(u_samp, Coord).r - 0.5,
                    texture(v_samp, Coord).r - 0.5);
    return vec3(rgb);
}

// ************************************************************************************************
void main()
{
    vec3 rgb = YUV2RGB(tex_coord);
    out_color = vec4(rgb, 1.0);

//    //伪彩色
//    float luminance = rgb.r*0.299 + rgb.g*0.587 + rgb.b*0.114;
//    out_color = vec4(luminance,abs(abs(luminance+luminance-1.0)-1.0),1.0-luminance, 1.0);

//    //gamma校正
//    float gamma = 1.0;
//    out_color.rgb = pow(out_color.rgb, vec3(1.0/gamma));

}
