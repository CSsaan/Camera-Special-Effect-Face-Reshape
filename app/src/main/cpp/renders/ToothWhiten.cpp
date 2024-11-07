//
// Created by chenshuai on 2022/11/10.
//
#include "ToothWhiten.h"
#include "../faces/ImageUtils.h"
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include<numeric>

#include "../faces/stb_image.h"

//float *cpoints = nullptr;
//FBO显示关键点
#define TOOTHWHITEN_VERY_SHADER_FILE "shaders/ToothWhiten/fbo_ToothWhiten_vert.glsl"
#define TOOTHWHITEN_FRAG_SHADER_FILE "shaders/ToothWhiten/fbo_ToothWhiten_frag.glsl"
//普通2D显示的着色器（显示FBO结果）
#define D2_TOOTHWHITEN_VERT_SHADER_FILE "shaders/ToothWhiten/2D_ToothWhiten_vert.glsl"
#define D2_TOOTHWHITEN_FRAG_SHADER_FILE "shaders/ToothWhiten/2D_ToothWhiten_frag.glsl"

#define TOOTHWHITEN_PNG_PATH "resource/teeth_mask.jpg" //skin_lookup.png
#define TOOTHLUT_PNG_PATH "resource/teeth_beauty_lookup.png" //skin_lookup.png
//#define POINTS_NUM 8 //使用的关键点数

ToothWhiten::ToothWhiten(): BaseVideoRender(false,"","")
{
    // *创建程序*
    face_program = std::shared_ptr<ProgramLoader>(new ProgramLoader("ToothWhiten", TOOTHWHITEN_VERY_SHADER_FILE, TOOTHWHITEN_FRAG_SHADER_FILE, asset_mgr));
    my2D_program = std::shared_ptr<ProgramLoader>(new ProgramLoader("ToothWhiten_2D", D2_TOOTHWHITEN_VERT_SHADER_FILE, D2_TOOTHWHITEN_FRAG_SHADER_FILE, asset_mgr));
    //检查创建program
    if (face_program->status != 0 || my2D_program->status != 0) // status=-1时，创建program失败
    {
        LOGE("[ToothWhiten()]: create_program error!");
    }
    init_ToothWhiten_data();
}

ToothWhiten::~ToothWhiten()
{
    // *释放vao和vbo资源*
    glDeleteVertexArrays(1, &face_vao);
    glDeleteBuffers(1, &face_vbo);
    glDeleteVertexArrays(1, &MASK_vao);
    glDeleteBuffers(1, &MASK_vbo);
    glDeleteVertexArrays(1, &D2_vao);
    glDeleteBuffers(1, &D2_vbo);
    glDeleteFramebuffers(1, &m_FboId);
    glDeleteTextures(1, &m_TextureId);
    glDeleteTextures(1, &m_TextureId2);
    glDeleteTextures(1, &m_FboTextureId);
    glDeleteTextures(1, &m_MASK_TextureId);
}

//*********************************************************************************************
void ToothWhiten::init_ToothWhiten_data()
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

    //使用索引绘制三角形的方法，不再使用把每个三角形的顶点都写出来的方法，dralElements？
    float mask_textureCoords[] {
            //口缝隙（牙齿）（88-95）
            0.0f, 0.0f, 0.154639f, 0.378788f,
            0.0f, 0.0f, 0.398625f, 0.196970f,
            0.0f, 0.0f, 0.512027f, 0.287879f,
            0.0f, 0.0f, 0.611684f, 0.212121f,
            0.0f, 0.0f, 0.872852f, 0.378788f,
            0.0f, 0.0f, 0.639176f, 0.848485f,
            0.0f, 0.0f, 0.522337f, 0.636364f,
            0.0f, 0.0f, 0.398625f, 0.833333f,
    };
    for(int n=0;n<4*POINTS_NUM;n++)
    {
        mask_Coords[n] = mask_textureCoords[n];
    }

    //*********************************************************************************************
    //指定位置
    //FBO
    glBindVertexArray(face_vao);
    glBindBuffer(GL_ARRAY_BUFFER, face_vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0); // *启用数组*
    //普通2D
    glBindVertexArray(D2_vao);
    glBindBuffer(GL_ARRAY_BUFFER, D2_vbo);
    // 1.先指定顶点坐标
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // 2.再指定纹理坐标
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //*********************************************************************************************
    // 加载图片1  (NDK 中是 AssetManager)
    AAsset *pathAsset = AAssetManager_open(asset_mgr, TOOTHWHITEN_PNG_PATH, AASSET_MODE_UNKNOWN);
    off_t assetLength = AAsset_getLength(pathAsset);
    auto *fileData = (unsigned char *) AAsset_getBuffer(pathAsset);
    MASK_png_data = stbi_load_from_memory(fileData, (int)assetLength, &width, &height, &nrChannels, 0);
    // 加载LUT图片
    AAsset *pathAsset2 = AAssetManager_open(asset_mgr, TOOTHLUT_PNG_PATH, AASSET_MODE_UNKNOWN);
    off_t assetLength2 = AAsset_getLength(pathAsset2);
    auto *fileData2 = (unsigned char *) AAsset_getBuffer(pathAsset2);
    MASK_png_data2 = stbi_load_from_memory(fileData2, (int)assetLength2, &width2, &height2, &nrChannels2, 0);

    //*********************************************************************************************
    //创建绑定到FBO的纹理
    glGenTextures(1, &m_FboTextureId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE); //含alpha：GL_CLAMP_TO_EDGE，避免透明边缘被插值
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时用
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    // ToothMASK纹理
    glGenTextures(1, &m_MASK_TextureId);
    glBindTexture(GL_TEXTURE_2D, m_MASK_TextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //含alpha：GL_CLAMP_TO_EDGE，避免透明边缘被插值
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时用
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, MASK_png_data);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    // 摄像头图像纹理
    glGenTextures(1, &m_TextureId);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //含alpha：GL_CLAMP_TO_EDGE，避免透明边缘被插值
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时用
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    // LUT纹理
    glGenTextures(1, &m_TextureId2);
    glBindTexture(GL_TEXTURE_2D, m_TextureId2);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //含alpha：GL_CLAMP_TO_EDGE，避免透明边缘被插值
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时用
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width2, (GLsizei)height2, 0, GL_RGBA, GL_UNSIGNED_BYTE,MASK_png_data2);
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
    strength_loc = glGetUniformLocation(my2D_program->program_id, "strength");
    // *加载深度学习模型*
    detect_model = std::shared_ptr<UltraFace>(new UltraFace(asset_mgr, "resource/slim-320.mnn", 240, 320, 4, 0.95, 0.25));
    pts_model = std::make_shared<Pfld>(asset_mgr, "resource/pfld-lite.mnn");
}

//*********************************************************************************************
void ToothWhiten::render()
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
    //更新顶点数组
    Cal_Mesh();
    //渲染滤镜
    render_ToothWhiten();
}

//*********************************************************************************************
//读取BaseVideoRender的数据
void ToothWhiten::readBasePixel()
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
void ToothWhiten::get_points()
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
//    for_each(pts_points.begin(), pts_points.end(), [](float& i) { i=(i+1.0f)*0.5f; });
//    //8个点： 人脸的两对关键点（3->8、29->24；9->14、23->18）+ 两只眼睛（96、97） + 脸中心的两个关键点（52->53；54）
//    points[0] = pts_points[8*2]*win_w;   points[1] = pts_points[8*2+1]*win_h;   points[2] = pts_points[24*2]*win_w;  points[3] = pts_points[24*2+1]*win_h;
//    points[4] = pts_points[14*2]*win_w;  points[5] = pts_points[14*2+1]*win_h;  points[6] = pts_points[18*2]*win_w;  points[7] = pts_points[18*2+1]*win_h;
//    points[8] = pts_points[96*2]*win_w;  points[9] = pts_points[96*2+1]*win_h;  points[10] = pts_points[97*2]*win_w; points[11] = pts_points[97*2+1]*win_h;
//    points[12] = pts_points[53*2]*win_w; points[13] = pts_points[53*2+1]*win_h;
//    points[14] = pts_points[54*2]*win_w; points[15] = pts_points[54*2+1]*win_h;

//    for(int n=0; n<98*2; n++)
//    {
//        if(n%2==0)
//        {
//            pts_points[n]=(pts_points[n]+1.0f)*0.5f*win_w;
//        }else
//        {
//            pts_points[n]=(pts_points[n]+1.0f)*0.5f*win_h;
//        }
//    }
//    for(int n=0; n<POINTS_NUM; n++)
//    {
//        mask_Coords[n*4] = pts_points[n*2]; mask_Coords[n*4+1] = pts_points[n*2+1];
//    }

    mask_Coords[0*4] = pts_points[88*2]; mask_Coords[0*4+1] = pts_points[88*2+1];
    mask_Coords[1*4] = pts_points[89*2]; mask_Coords[1*4+1] = pts_points[89*2+1];
    mask_Coords[2*4] = pts_points[90*2]; mask_Coords[2*4+1] = pts_points[90*2+1];
    mask_Coords[3*4] = pts_points[91*2]; mask_Coords[3*4+1] = pts_points[91*2+1];
    mask_Coords[4*4] = pts_points[92*2]; mask_Coords[4*4+1] = pts_points[92*2+1];
    mask_Coords[5*4] = pts_points[93*2]; mask_Coords[5*4+1] = pts_points[93*2+1];
    mask_Coords[6*4] = pts_points[94*2]; mask_Coords[6*4+1] = pts_points[94*2+1];
    mask_Coords[7*4] = pts_points[95*2]; mask_Coords[7*4+1] = pts_points[95*2+1];
    //LOGI("pos:[%f,%f]",points[0],points[1]);
}

//*********************************************************************************************
//循环计算直方图顶点
void ToothWhiten::Cal_Mesh()
{
    if(frame_program == nullptr || yuv == nullptr || frame_program->status == -1) {
        return;
    }
    //*********************************************************************************************
    //绑定FBO和普通2D的VAO、VBO
    //绑定FBO的顶点&缓冲(0:R 1:G 2:B)
    if(MASK_vbo == 0)
    {
        glGenBuffers(1, &MASK_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, MASK_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(mask_Coords), mask_Coords, GL_STATIC_DRAW);
    }else
    {
        glBindBuffer(GL_ARRAY_BUFFER, MASK_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mask_Coords), mask_Coords); //更新坐标
        //LOGI("%lu : %f",sizeof(mask_Coords),mask_Coords[1]);
    }
    if(MASK_vao == GL_NONE)
    {
        glGenVertexArrays(1, &MASK_vao);
        glBindVertexArray(MASK_vao);
        //FBO顶点缓冲
        glBindBuffer(GL_ARRAY_BUFFER, MASK_vbo);
        // 一次性指定顶点坐标&纹理坐标
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0); // *启用数组*
    }
}

//*********************************************************************************************
void ToothWhiten::render_ToothWhiten()
{
//    if(MASK_vbo == GL_NONE || MASK_vao == GL_NONE)
//    {
//        return;
//    }
    // 要开启混色模式
    glViewport(0.0, 0.0, win_w, win_h);
    //绑定FBO准备渲染
    glUseProgram(face_program->program_id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
    //清除FBO导致重影(防止重影、不消失)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glBindVertexArray(face_vao);
    glBindVertexArray(MASK_vao);
    //传uniform
    glUniform1f(has_face_loc,has_face);  //有无人脸

    //顶点索引
    short indices[] = {
            0, 1, 7,
            7, 1, 6,
            1, 6, 2,
            2, 6, 3,
            5, 6, 3,
            5, 3, 4,
    };
    //传MASK图
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_MASK_TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_SHORT, indices);
    glBindVertexArray(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    //*********************************************************************************************
    //开始普通2D渲染，显示FBO结果
    glUseProgram(my2D_program->program_id);
    glBindVertexArray(D2_vao);  // 绑定vao和vbo
    glm::mat4 rotation = glm::mat4(1.0f);
    glUniformMatrix4fv(frame_rotation_loc, 1, GL_FALSE, glm::value_ptr(rotation));
    glUniform1f(strength_loc,sigmaA);// 亮牙强度
    //离屏渲染MASK结果
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) win_w, (GLsizei) win_h, 0, GL_RGBA,GL_UNSIGNED_BYTE, nullptr);
    //(传入原图)
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)win_w, (GLsizei)win_h, 0, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);
    //传入LUT
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_TextureId2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width2, (GLsizei)height2, 0, GL_RGBA, GL_UNSIGNED_BYTE,nullptr);
    //draw
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindVertexArray(GL_NONE);
}