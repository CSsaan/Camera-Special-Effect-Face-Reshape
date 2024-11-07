//
// Created by chenshuai on 2022/11/10.
//
#include "FaceMask.h"
#include "../faces/ImageUtils.h"
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include<numeric>

#define STB_IMAGE_IMPLEMENTATION
#include "../faces/stb_image.h"

//float *cpoints = nullptr;
//FBO显示关键点
#define FACEMASK_VERY_SHADER_FILE "shaders/facemask/fbo_facemask_vert.glsl"
#define FACEMASK_FRAG_SHADER_FILE "shaders/facemask/fbo_facemask_frag.glsl"
//普通2D显示的着色器（显示FBO结果）
#define D2_FACEMASK_VERT_SHADER_FILE "shaders/facemask/2D_facemask_vert.glsl"
#define D2_FACEMASK_FRAG_SHADER_FILE "shaders/facemask/2D_facemask_frag.glsl"

#define FACEMASK_PNG_PATH "resource/facemask.jpg" //skin_lookup.png
#define POINTS_NUM 6 //使用的关键点数

FaceMask::FaceMask(): BaseVideoRender(false,"","")
{
    // *创建程序*
    face_program = std::shared_ptr<ProgramLoader>(new ProgramLoader("FaceMask", FACEMASK_VERY_SHADER_FILE, FACEMASK_FRAG_SHADER_FILE, asset_mgr));
    my2D_program = std::shared_ptr<ProgramLoader>(new ProgramLoader("FaceMask_2D", D2_FACEMASK_VERT_SHADER_FILE, D2_FACEMASK_FRAG_SHADER_FILE, asset_mgr));
    //检查创建program
    if (face_program->status != 0 || my2D_program->status != 0) // status=-1时，创建program失败
    {
        LOGE("[FaceMask()]: create_program error!");
    }
    init_FaceMask_data();
}

FaceMask::~FaceMask()
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
    glDeleteTextures(1, &m_FboTextureId);
    glDeleteTextures(1, &m_MASK_TextureId);
}

//*********************************************************************************************
void FaceMask::init_FaceMask_data()
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

    //TODO 改为index的画法，dralElements，脸部三角剖分算法找到每个三角形的index
    // gewwgwse
    // gewwgwse
    // gewwgwse
    float mask_textureCoords[] {
//            //共79个点
//            //脸（0-32）
            0.0f, 0.0f, 0.150f, 0.508f,
//            0.0f, 0.0f, 0.143f, 0.535f,
//            0.0f, 0.0f, 0.146f, 0.570f,
//            0.0f, 0.0f, 0.146f, 0.603f,
//            0.0f, 0.0f, 0.159f, 0.645f,
//            0.0f, 0.0f, 0.165f, 0.689f,
//            0.0f, 0.0f, 0.181f, 0.723f,
//            0.0f, 0.0f, 0.196f, 0.756f,
//            0.0f, 0.0f, 0.225f, 0.781f,
//            0.0f, 0.0f, 0.256f, 0.809f,
//            0.0f, 0.0f, 0.271f, 0.835f,
//            0.0f, 0.0f, 0.303f, 0.860f,
//            0.0f, 0.0f, 0.343f, 0.888f,
//            0.0f, 0.0f, 0.378f, 0.911f,
//            0.0f, 0.0f, 0.418f, 0.935f,
            0.0f, 0.0f, 0.468f, 0.946f,
            //0.0f, 0.0f, 0.515f, 0.953f,
            0.0f, 0.0f, 0.559f, 0.941f,     0.0f, 0.0f, 0.150f, 0.508f,    0.0f, 0.0f, 0.559f, 0.941f,
//            0.0f, 0.0f, 0.593f, 0.932f,
//            0.0f, 0.0f, 0.625f, 0.918f,
//            0.0f, 0.0f, 0.662f, 0.895f,
//            0.0f, 0.0f, 0.696f, 0.872f,
//            0.0f, 0.0f, 0.728f, 0.842f,
//            0.0f, 0.0f, 0.762f, 0.577f,
//            0.0f, 0.0f, 0.781f, 0.774f,
//            0.0f, 0.0f, 0.806f, 0.737f,
//            0.0f, 0.0f, 0.818f, 0.705f,
//            0.0f, 0.0f, 0.834f, 0.672f,
//            0.0f, 0.0f, 0.843f, 0.638f,
//            0.0f, 0.0f, 0.853f, 0.605f,
//            0.0f, 0.0f, 0.856f, 0.577f,
//            0.0f, 0.0f, 0.859f, 0.540f,
            0.0f, 0.0f, 0.856f, 0.496f,
//            //左眉（33-41）
//            0.0f, 0.0f, 0.218f, 0.445f,
//            0.0f, 0.0f, 0.265f, 0.424f,
//            0.0f, 0.0f, 0.318f, 0.422f,
//            0.0f, 0.0f, 0.362f, 0.426f,
//            0.0f, 0.0f, 0.381f, 0.431f,
//            0.0f, 0.0f, 0.396f, 0.450f,
//            0.0f, 0.0f, 0.359f, 0.452f,
//            0.0f, 0.0f, 0.318f, 0.450f,
//            0.0f, 0.0f, 0.262f, 0.447f,
//            //右眉（42-50）
//            0.0f, 0.0f, 0.587f, 0.440f,
//            0.0f, 0.0f, 0.637f, 0.426f,
//            0.0f, 0.0f, 0.684f, 0.422f,
//            0.0f, 0.0f, 0.737f, 0.424f,
//            0.0f, 0.0f, 0.775f, 0.443f,
//            0.0f, 0.0f, 0.740f, 0.450f,
//            0.0f, 0.0f, 0.690f, 0.447f,
//            0.0f, 0.0f, 0.643f, 0.452f,
//            0.0f, 0.0f, 0.587f, 0.452f,
//            //鼻子（51-59）
//            0.0f, 0.0f, 0.493f, 0.556f,
//            0.0f, 0.0f, 0.493f, 0.587f,
//            0.0f, 0.0f, 0.493f, 0.617f,
//            0.0f, 0.0f, 0.493f, 0.658f,
//            0.0f, 0.0f, 0.387f, 0.712f,
//            0.0f, 0.0f, 0.450f, 0.712f,
//            0.0f, 0.0f, 0.512f, 0.712f,
//            0.0f, 0.0f, 0.575f, 0.712f,
//            0.0f, 0.0f, 0.625f, 0.712f,
//            //左眼（60-67）
//            0.0f, 0.0f, 0.262f, 0.519f,
//            0.0f, 0.0f, 0.287f, 0.501f,
//            0.0f, 0.0f, 0.325f, 0.496f,
//            0.0f, 0.0f, 0.359f, 0.508f,
//            0.0f, 0.0f, 0.378f, 0.533f,
//            0.0f, 0.0f, 0.350f, 0.542f,
//            0.0f, 0.0f, 0.325f, 0.540f,
//            0.0f, 0.0f, 0.284f, 0.533f,
//            //右眼（68-75）
//            0.0f, 0.0f, 0.606f, 0.533f,
//            0.0f, 0.0f, 0.621f, 0.515f,
//            0.0f, 0.0f, 0.656f, 0.503f,
//            0.0f, 0.0f, 0.693f, 0.503f,
//            0.0f, 0.0f, 0.725f, 0.522f,
//            0.0f, 0.0f, 0.700f, 0.538f,
//            0.0f, 0.0f, 0.662f, 0.542f,
//            0.0f, 0.0f, 0.621f, 0.542f,
//            //嘴（76-87）
//            0.0f, 0.0f, 0.384f, 0.786f,
//            0.0f, 0.0f, 0.431f, 0.770f,
//            0.0f, 0.0f, 0.465f, 0.765f,
//            0.0f, 0.0f, 0.490f, 0.770f,
//            0.0f, 0.0f, 0.518f, 0.765f,
//            0.0f, 0.0f, 0.556f, 0.772f,
//            0.0f, 0.0f, 0.603f, 0.788f,
//            0.0f, 0.0f, 0.587f, 0.812f,
//            0.0f, 0.0f, 0.550f, 0.837f,
//            0.0f, 0.0f, 0.496f, 0.844f,
//            0.0f, 0.0f, 0.446f, 0.835f,
//            0.0f, 0.0f, 0.412f, 0.816f,
//            //口缝隙（88-95）
//            0.0f, 0.0f, 0.400f, 0.793f,
//            0.0f, 0.0f, 0.468f, 0.793f,
//            0.0f, 0.0f, 0.493f, 0.793f,
//            0.0f, 0.0f, 0.518f, 0.788f,
//            0.0f, 0.0f, 0.581f, 0.793f,
//            0.0f, 0.0f, 0.518f, 0.816f,
//            0.0f, 0.0f, 0.490f, 0.821f,
//            0.0f, 0.0f, 0.465f, 0.812f,
//            //瞳孔（96-97）
//            0.0f, 0.0f, 0.321f, 0.519f,
//            0.0f, 0.0f, 0.668f, 0.519f,


    };
    for(int n=0;n<4*POINTS_NUM;n++)
    {
        mask_Coords[n] = mask_textureCoords[n];
    }

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
    // 加载图片1  (NDK 中是 AssetManager)
    // 1.打开 Asset 文件夹下的文件
    AAsset *pathAsset = AAssetManager_open(asset_mgr, FACEMASK_PNG_PATH, AASSET_MODE_UNKNOWN);
    // 2.得到文件的长度
    off_t assetLength = AAsset_getLength(pathAsset);
    // 3.得到文件对应的 Buffer
    auto *fileData = (unsigned char *) AAsset_getBuffer(pathAsset);
    // 4.stb_image 的方法，从内存中加载图片
    MASK_png_data = stbi_load_from_memory(fileData, (int)assetLength, &width, &height, &nrChannels, 0);

    //*********************************************************************************************
    //创建绑定到FBO的纹理
    glGenTextures(1, &m_FboTextureId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE); //含alpha：GL_CLAMP_TO_EDGE，避免透明边缘被插值
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小时用
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    // faceMASK纹理
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
void FaceMask::render()
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
    render_FaceMask();
}

//*********************************************************************************************
//读取BaseVideoRender的数据
void FaceMask::readBasePixel()
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
void FaceMask::get_points()
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

    mask_Coords[0*4] = pts_points[0*2]; mask_Coords[0*4+1] = pts_points[0*2+1];
    mask_Coords[1*4] = pts_points[15*2]; mask_Coords[1*4+1] = pts_points[15*2+1];
    mask_Coords[2*4] = pts_points[17*2]; mask_Coords[2*4+1] = pts_points[17*2+1];
    mask_Coords[3*4] = pts_points[0*2]; mask_Coords[3*4+1] = pts_points[0*2+1];
    mask_Coords[4*4] = pts_points[17*2]; mask_Coords[4*4+1] = pts_points[17*2+1];
    mask_Coords[5*4] = pts_points[32*2]; mask_Coords[5*4+1] = pts_points[32*2+1];
    //LOGI("pos:[%f,%f,%f,%f,%f]",mask_Coords[0*4],mask_Coords[1*4],mask_Coords[2*4],mask_Coords[3*4],mask_Coords[4*4]);
    //LOGI("pos:[%f,%f,%f,%f,%f]",mask_Coords[0*4+2],mask_Coords[1*4+2],mask_Coords[2*4+2],mask_Coords[3*4+2],mask_Coords[4*4+2]);

//    LOGI("pos:[%f,%f]",points[0],points[1]);
}

//*********************************************************************************************
//循环计算直方图顶点
void FaceMask::Cal_Mesh()
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
void FaceMask::render_FaceMask()
{
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
    glUniform1f(Vstrength_loc,sigmaA);// 脸颊拉伸强度
    glUniform1f(strength_loc,sigmaB);// 下巴拉伸强度
    glUniform1f(has_face_loc,has_face);  //有无人脸
    if(has_face == 1)
    {
        //8个点：人脸的两对关键点（3、29；9、23）+ 两只眼睛（96、97） + 脸中心的两个关键点（52；54）
        //glUniform2fv(points_loc, 8, points);
    }
    //FBO离屏渲染(传入原图)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)win_w, (GLsizei)win_h, 0, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);
    //传MASK图
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_MASK_TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
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
