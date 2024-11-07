//
// Created by chenshuai on 2022/11/10.
//

#ifndef CAMERASPECIALEFFECT_FACEMASK_H
#define CAMERASPECIALEFFECT_FACEMASK_H

#include "BaseVideoRender.h"
#include <memory>
#include <GLES3/gl3.h>
#include "../utils/ProgramLoader.h"
//#include "../utils/PngTool.h"
#include "../faces/UltraFace.hpp"
//#include "../faces/UltraFaceNCNN.hpp"
#include "../faces/Pfld.hpp"

class FaceMask:public BaseVideoRender
{
public:
    FaceMask();
    ~FaceMask();


private:
    void render();
    float mask_Coords[4*6]; //4*使用关键点数

    //float points[3*2];
    void init_FaceMask_data();
    void render_FaceMask();
    void readBasePixel(); //读取glReadPixels的RGB图像
    void get_points();
    void Cal_Mesh(); //循环计算顶点坐标
    // *人脸程序管理*
    std::shared_ptr<ProgramLoader> face_program = nullptr;
    std::shared_ptr<ProgramLoader> my2D_program = nullptr;
    // *vao 和 vbo*
    GLuint MASK_vbo{};
    GLuint MASK_vao{};
    GLuint face_vao{};
    GLuint face_vbo{};
    GLuint D2_vbo{};
    GLuint D2_vao{};
    GLuint m_FboTextureId{}; //FBO绑定的纹理
    GLuint m_MASK_TextureId{}; //FBO绑定MASK的纹理
    GLuint m_TextureId{}; //原图纹理
    GLuint m_FboId{};
    GLint has_face = 0;

    GLint transform_mat_loc{}; // 变换矩阵的位置
//    GLint x1y1_loc{};// 人脸坐标
//    GLint x2y2_loc{};
    GLint has_face_loc{};
    GLint Vstrength_loc{};
    GLint strength_loc{};
    GLint points_loc{};
    int step = 0;

    // *人脸检测*
    std::shared_ptr<UltraFace> detect_model = nullptr;
    // *人脸关键点模型*
    std::shared_ptr<Pfld> pts_model = nullptr;

    uint8_t *pBuffer = nullptr; //glReadPixels的RGB图像指针
    bool first = true;

    unsigned char *MASK_png_data{}; // stb_image.h
    int width{}, height{}, nrChannels{};

    long long GetSysCurrentTime() //计算延时用
    {
        struct timeval time;
        gettimeofday(&time, NULL);
        long long curTime = ((long long)(time.tv_sec))*1000+time.tv_usec/1000;
        return curTime;
    }
};

#endif //CAMERASPECIALEFFECT_FACEMASK_H
