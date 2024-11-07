//
// Created by chenshuai on 2022/11/23.
//

#ifndef CAMERASPECIALEFFECT_FBOHIST_H
#define CAMERASPECIALEFFECT_FBOHIST_H

#include "BaseVideoRender.h"
#include "../utils/ProgramLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include "opencv2/opencv.hpp"
#include <vector>
using namespace std;
//using namespace cv;

class FBOHist: public BaseVideoRender
{
public:
    FBOHist();
    ~FBOHist();
    void render();
    void init_FBOHist_data();

private:
    int m_RenderDataSize = 256; //一共12个正方形
    // 顶点、纹理坐标
    glm::vec3 *m_pVerticesCoords = nullptr;
//    glm::vec2 *m_pTextureCoords = nullptr;
    glm::vec4 *m_pCombile = nullptr;
    short *m_HistData = nullptr;

    void UpdateMesh();
    static glm::vec3 texCoordToVertexCoord(const glm::vec2 &texCoord);
    static glm::vec4 Combile(const glm::vec3 &VertCoord, glm::vec2 &texCoord);

    int MAX_VALUE_LEVEL = 0;

    GLint m_MVPMatLoc{};
    GLint m_Loc{};

    GLuint fbo_vbo{};
    GLuint fbo_vao{};
    GLuint D2_vbo{};
    GLuint D2_vao{};
    unsigned int m_FboId{};

    GLuint tex_id{};
    GLuint m_FboTextureId{};
    GLuint texture1{};

    std::shared_ptr<ProgramLoader> fbo_program = nullptr; //创建FBO离屏渲染program
    std::shared_ptr<ProgramLoader> my_program = nullptr; //创建普通2D渲染program
    void render_FBOHist();
    void render_Mesh();
    void readBasePixel();
    void CalHistgramPointer(const uint8_t *imgDataPtr);
    uint8_t *pBuffer = nullptr;
    bool first = true;
    long long GetSysCurrentTime() //计算延时用
    {
        struct timeval time;
        gettimeofday(&time, NULL);
        long long curTime = ((long long)(time.tv_sec))*1000+time.tv_usec/1000;
        return curTime;
    }
};

#endif //CAMERASPECIALEFFECT_FBOHIST_H
