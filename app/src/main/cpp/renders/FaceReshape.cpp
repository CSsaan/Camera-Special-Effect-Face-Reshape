//
// Created by chenshuai on 2022/11/10.
//
#include "FaceReshape.h"
#include "../faces/ImageUtils.h"
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include<numeric>

//float *cpoints = nullptr;
//FBO显示关键点
#define FACERESHAPE_VERY_SHADER_FILE "shaders/facereshape/facereshape_vert.glsl"
#define FACERESHAPE_FRAG_SHADER_FILE "shaders/facereshape/facereshape_frag.glsl"
//普通2D显示的着色器（显示FBO结果）
#define D2_FACERESHAPE_VERT_SHADER_FILE "shaders/facereshape/2D_face_reshape_vert.glsl"
#define D2_FACERESHAPE_FRAG_SHADER_FILE "shaders/facereshape/2D_face_reshape_frag.glsl"

FaceReshape::FaceReshape(): BaseVideoRender(false,"","")
{
    // *创建程序*
    face_program = std::shared_ptr<ProgramLoader>(new ProgramLoader("FaceReshape", FACERESHAPE_VERY_SHADER_FILE, FACERESHAPE_FRAG_SHADER_FILE, asset_mgr));
    my2D_program = std::shared_ptr<ProgramLoader>(new ProgramLoader("FaceReshape_2D", D2_FACERESHAPE_VERT_SHADER_FILE, D2_FACERESHAPE_FRAG_SHADER_FILE, asset_mgr));
    //检查创建program
    if (face_program->status != 0 || my2D_program->status != 0) // status=-1时，创建program失败
    {
        LOGE("[FaceReshape()]: create_program error!");
    }
    init_FaceReshape_data();
}

FaceReshape::~FaceReshape()
{
    // *释放vao和vbo资源*
    glDeleteVertexArrays(1, &face_vao);
    glDeleteBuffers(1, &face_vbo);
    glDeleteBuffers(1, &D2_vbo);
    glDeleteFramebuffers(1, &m_FboId);
    glDeleteVertexArrays(1, &D2_vao);
    glDeleteTextures(1, &m_TextureId);
    glDeleteTextures(1, &m_FboTextureId);
}

//*********************************************************************************************
void FaceReshape::init_FaceReshape_data()
{
    //FBO顶点数组
    float vert_fbo[] = {
            -1.0, 1.0, 0.0, 1.0, // 左上
            -1.0, -1.0, 0.0, 0.0, // 左下
            1.0, -1.0, 1.0, 0.0, // 右下
            -1.0, 1.0, 0.0, 1.0,  // 左上
            1.0, 1.0, 1.0, 1.0, // 右上
            1.0, -1.0, 1.0, 0.0  // 右下
    };
    //普通2D顶点数组（显示FBO结果）
    float vert_2D[] = {
            //  ---- 位置 ----   ---- 纹理坐标 ----
            -1.0, -1.0, 0.0, 1.0, // 左下
            -1.0, 1.0, 0.0, 0.0, // 左上
            1.0, -1.0, 1.0, 1.0, // 右下
            1.0, -1.0, 1.0, 1.0,  // 右下
            -1.0, 1.0, 0.0, 0.0,  // 左上
            1.0, 1.0, 1.0, 0.0, // 右上
    };
    //FBO
    glGenVertexArrays(1, &face_vao);
    glBindVertexArray(face_vao);
    glGenBuffers(1, &face_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, face_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert_fbo), vert_fbo, GL_STATIC_DRAW);
    //绑定普通2D的顶点&缓冲
    glGenVertexArrays(1, &D2_vao);
    glBindVertexArray(D2_vao);
    glGenBuffers(1, &D2_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, D2_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert_2D), vert_2D, GL_STATIC_DRAW);
    //指定位置
    //FBO
    glBindVertexArray(face_vao);
    glBindBuffer(GL_ARRAY_BUFFER, face_vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0); // *启用数组*
    //普通2D
    glBindVertexArray(D2_vao);
    glBindBuffer(GL_ARRAY_BUFFER, D2_vbo);
    //glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    //glEnableVertexAttribArray(0); // *启用数组*
    // 1.先指定顶点坐标
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // 2.再指定纹理坐标
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //*********************************************************************************************
    //创建绑定到FBO的纹理
    glGenTextures(1, &m_FboTextureId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE); //含alpha：GL_CLAMP_TO_EDGE，避免透明边缘被插值
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时用
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    // 摄像头图像纹理
    glGenTextures(1, &m_TextureId);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //含alpha：GL_CLAMP_TO_EDGE，避免透明边缘被插值
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时用
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    //*********************************************************************************************
    //创建FBO(会渲染到上面创建的纹理，然后普通2D渲染上面的纹理就可以)
    glGenFramebuffers(1, &m_FboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)win_w, (GLsizei)win_h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE)
    {
        LOGI("PBOSample::CreateFrameBufferObj glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
    }
    CHECK_GL_ERR("CreateFrameBufferObj");
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

    // *使用当前程序*
    glUseProgram(face_program->program_id);
    // 获取变换矩阵的统一变量位置
    has_face_loc = glGetUniformLocation(face_program->program_id, "has_face");
    Vstrength_loc = glGetUniformLocation(face_program->program_id, "Vstrength");
    strength_loc = glGetUniformLocation(face_program->program_id, "strength");
    points_loc = glGetUniformLocation(face_program->program_id, "points");
    // *加载深度学习模型*
    detect_model = std::shared_ptr<UltraFace>(new UltraFace(asset_mgr, "resource/slim-320.mnn", 240, 320, 4, 0.95, 0.25));
    pts_model = std::make_shared<Pfld>(asset_mgr, "resource/pfld-lite.mnn");
}

//*********************************************************************************************
void FaceReshape::render()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    //渲染帧
    render_frame();
    //读取BaseVideoRender的数据
    readBasePixel();
    //获取关键点位置
    get_points();
    //渲染滤镜
    render_FaceReshape();
}

//*********************************************************************************************
//读取BaseVideoRender的数据
void FaceReshape::readBasePixel()
{
    if(img_w == 0 || img_h == 0) {
        return;
    }
    if(first){
        pBuffer = new uint8_t[win_w*win_h * 3];
        first = false;
    }
    glReadPixels(0, 0, (GLsizei)win_w, (GLsizei)win_h, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);
}

//*********************************************************************************************
//获取人脸关键点：人脸的两对关键点（1、31；9、23） + 脸中心的两个关键点（52；54）
void FaceReshape::get_points()
{
    if(yuv == nullptr) {
        LOGI("yuv = nullptr");
        return;
    }
    // *首先yuv->bgr*
    //pthread_mutex_lock(&frame_lock);

    //法一：慢
//    cv::Mat frame_mat;
//    ImageUtils::I420_to_Mat(yuv, img_w, img_h, img_rotation, frame_mat); //40ms
    //法二：快
    cv::Mat frame_mat0(win_h, win_w, CV_8UC3, cv::Scalar(0, 0, 0)); //优化后，18ms
    frame_mat0.data = pBuffer; //uchar*转Mat
    cv::Mat frame_mat = frame_mat0.clone();
    cv::flip(frame_mat,frame_mat,0);
    //cv::resize(frame_mat,frame_mat,cv::Size(win_w,win_h));

    //pthread_mutex_unlock(&frame_lock);
    if(frame_mat.empty()) {
        return;
    }
    //**********************************************************************
    // *人脸检测*
    vector<FaceInfo> face_info;
    detect_model->detect(frame_mat, face_info);
//    LOGI("There are %u faces", face_info.size());

    glUseProgram(face_program->program_id);
    if(face_info.empty())
    {
        has_face = 0;
        return ;
    } else{
        has_face = 1;
    }
    //TODO 不再只找一个最大脸， 改为最大可以找5个脸
    // 查找面积最大的人脸图像
    int max_area_id = 0;
    float max_area = 0;
    for(int i = 0; i < face_info.size(); ++i) {
        float area = (face_info[i].x1 - face_info[i].x2) * (face_info[i].y1 - face_info[i].y2);
        if(area > max_area) {
            max_area_id = i;
            max_area = area;
        }
    }
    if(max_area < 1000) {
        return;
    }
    //**********************************************************************
    cv::Mat resize_mat; // 裁剪人脸部分
    cv::Size croped_wh;
    cv::Point s_point;
    // *人脸关键点检测*
    std::vector<LandmarkInfo> landmarks;
    // *裁剪人脸部分*
    cv::Point pt1(face_info[max_area_id].x1, face_info[max_area_id].y1);
    cv::Point pt2(face_info[max_area_id].x2, face_info[max_area_id].y2);
    resize_mat = pts_model->Get_Resize_Croped_Img(frame_mat, pt1, pt2, s_point, croped_wh);
    // *进行关键点检测*
    pts_model->Get_Landmark_Points(resize_mat, croped_wh, s_point, landmarks);
    vector<float> pts_points = ImageUtils::LandmakrInfo_to_GLPos(landmarks, frame_mat.cols, frame_mat.rows); //98个点（x,y）

    // m_VRow 是一个 整型的 vector 实现的时每个元素的值 减 1
    for_each(pts_points.begin(), pts_points.end(), [](float& i) { i=(i+1.0f)*0.5f; });
    //8个点： 人脸的两对关键点（3->8、29->24；9->14、23->18）+ 两只眼睛（96、97） + 脸中心的两个关键点（52->53；54->85）
    points[0] = pts_points[8*2]*win_w;   points[1] = pts_points[8*2+1]*win_h;   points[2] = pts_points[24*2]*win_w;  points[3] = pts_points[24*2+1]*win_h;
    points[4] = pts_points[14*2]*win_w;  points[5] = pts_points[14*2+1]*win_h;  points[6] = pts_points[18*2]*win_w;  points[7] = pts_points[18*2+1]*win_h;
    points[8] = pts_points[96*2]*win_w;  points[9] = pts_points[96*2+1]*win_h;  points[10] = pts_points[97*2]*win_w; points[11] = pts_points[97*2+1]*win_h;
    points[12] = pts_points[53*2]*win_w; points[13] = pts_points[53*2+1]*win_h;
    points[14] = pts_points[85*2]*win_w; points[15] = pts_points[85*2+1]*win_h;
//    LOGI("pos:[%f,%f]",points[0],points[1]);
}


//*********************************************************************************************
void FaceReshape::render_FaceReshape()
{
    // 要开启混色模式
    glViewport(0.0, 0.0, win_w, win_h);
    //绑定FBO准备渲染
    glUseProgram(face_program->program_id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
    glBindVertexArray(face_vao);
    //传uniform
    glUniform1f(Vstrength_loc,sigmaA);// 脸颊拉伸强度
    glUniform1f(strength_loc,sigmaB);// 下巴拉伸强度
    glUniform1f(has_face_loc,has_face);  //有无人脸
    if(has_face == 1)
    {
        //8个点：人脸的两对关键点（3、29；9、23）+ 两只眼睛（96、97） + 脸中心的两个关键点（52；54）
        glUniform2fv(points_loc, 8, points);
    }
    //FBO离屏渲染(传入原图)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)win_w, (GLsizei)win_h, 0, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    //*********************************************************************************************
    //开始普通2D渲染，显示FBO结果
    glUseProgram(my2D_program->program_id);
    glBindVertexArray(D2_vao);  // 绑定vao和vbo
    glm::mat4 rotation = glm::mat4(1.0f);
    glUniformMatrix4fv(frame_rotation_loc, 1, GL_FALSE, glm::value_ptr(rotation));
    //激活纹理
    glActiveTexture(GL_TEXTURE0); //对应片段着色器的纹理位置
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);  //选择显示的R\G\B纹理
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) win_w, (GLsizei) win_h, 0, GL_RGBA,GL_UNSIGNED_BYTE, nullptr);
    //draw
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindVertexArray(GL_NONE);


    // *使用program，为了传uniform*
//    glUseProgram(face_program->program_id);
//    glUniform2f(x1y1_loc,pts_vec[192],pts_vec[193]);  // 左眼
//    glUniform2f(x2y2_loc,pts_vec[194],pts_vec[195]); // 右眼
//    glUniform2f(img_size_loc, win_w, win_h);
}
