//
// Created by chenshuai on 2022/11/23.
// FBO离屏渲染直方图 + 普通2D显示。
//
#include "FBOHist.h"
#include "../faces/stb_image.h"

//FBO进行离屏渲染的着色器（离屏渲染）
#define FBO_HIST_VERT_SHADER_FILE "shaders/fbo_hist_vert.glsl"
#define FBO_HIST_FRAG_SHADER_FILE "shaders/fbo_hist_frag.glsl"
//普通2D显示的着色器（显示FBO结果）
#define FRAME_2D_VERT_SHADER_FILE "shaders/2D_frame_vert.glsl"
#define FRAME_2D_FRAG_SHADER_FILE "shaders/2D_frame_frag.glsl"
#define PNG_NAME "resource/ocean.png"

//原始的frame_program为普通2D渲染
FBOHist::FBOHist() : BaseVideoRender(false, "", "")
{
    //为申请内存
    m_pVerticesCoords = new glm::vec3[m_RenderDataSize * 6]; //(x,y,z) * 6 points
//    m_pTextureCoords = new glm::vec2[m_RenderDataSize * 6]; //(x,y) * 6 points
    m_pCombile = new glm::vec4[m_RenderDataSize * 6]; //(x,y, x,y) * 6 points
    m_HistData = new short[m_RenderDataSize];

    //原始相机program:   [frame_program]
    //普通2D的program为: [my_program]
    //创建FBO离屏渲染program: [fbo_program]
    my_program = std::shared_ptr<ProgramLoader>(new ProgramLoader("MyRender", FRAME_2D_VERT_SHADER_FILE, FRAME_2D_FRAG_SHADER_FILE, asset_mgr));
    fbo_program = std::shared_ptr<ProgramLoader>(new ProgramLoader("FBORender", FBO_HIST_VERT_SHADER_FILE, FBO_HIST_FRAG_SHADER_FILE, asset_mgr));
    //检查创建program
    if (fbo_program->status != 0 || my_program->status != 0) // status=-1时，创建program失败
    {
        LOGE("[init_FBOHist_data] load_frame_program error!");
    }

    //初始化相关数据
    init_FBOHist_data();
}

//析构释放
FBOHist::~FBOHist()
{
    // 释放 yuv 数据
    if(yuv != nullptr) {
        delete[] yuv;
        yuv = nullptr;
    }
//    if (m_pTextureCoords != nullptr) {
//        delete [] m_pTextureCoords;
//        m_pTextureCoords = nullptr;
//    }
    if (m_pVerticesCoords != nullptr) {
        delete [] m_pVerticesCoords;
        m_pVerticesCoords = nullptr;
    }
    if (m_pCombile != nullptr) {
        delete [] m_pCombile;
        m_pCombile = nullptr;
    }
    if (m_HistData != nullptr) {
        delete [] m_HistData;
        m_HistData = nullptr;
    }
    if(pBuffer != nullptr) {
        delete[] pBuffer;
        pBuffer = nullptr;
    }

    // 释放缓冲
    glDeleteBuffers(1, &fbo_vbo);
    glDeleteVertexArrays(1, &fbo_vao);
    glDeleteBuffers(1, &D2_vbo);
    glDeleteBuffers(1, &m_FboId);
    glDeleteVertexArrays(1, &D2_vao);
    glDeleteTextures(1, &m_FboTextureId); //销毁普通2D显示结果纹理
    glDeleteTextures(1, &tex_id); //销毁FBO离屏渲染纹理（直方图中没用到纹理）
    glDeleteTextures(1, &texture1); //销毁原图纹理
//    png_data = nullptr;
}

//初始化加载图片,创建顶点、纹理、获取_loc
void FBOHist::init_FBOHist_data()
{
    //*********************************************************************************************
    // 创建一个用来普通2D显示的纹理(显示FBO渲染结果的直方图)
    glGenTextures(1, &m_FboTextureId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, png_data);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    // 创建一个FBO纹理的颜色附着（直方图中，FBO没有传纹理，只传了坐标）
//    glGenTextures(1, &tex_id);
//    glBindTexture(GL_TEXTURE_2D, tex_id);
//    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, png_data);
//    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    //*********************************************************************************************
    // 初始化FBO
    glGenFramebuffers(1, &m_FboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE)
    {
        LOGI("PBOSample::CreateFrameBufferObj glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
    }
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    CHECK_GL_ERR("CreateFrameBufferObj");
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

    //*********************************************************************************************
    //获取uniform_location
    m_Loc = glGetUniformLocation(my_program->program_id, "s_TextureMap");
    frame_rotation_loc = glGetUniformLocation(my_program->program_id, "frame_rotation");
    m_MVPMatLoc = glGetUniformLocation(fbo_program->program_id, "u_MVPMatrix");
}

//循环计算直方图顶点
void FBOHist::render_Mesh()
{
    if(frame_program == nullptr || yuv == nullptr || frame_program->status == -1) {
        return;
    }

    //先激活FBO program
    glUseProgram(fbo_program->program_id);

    //*********************************************************************************************
//    // 加载图片2  (NDK 中是 AssetManager)
//    // 1.打开 Asset 文件夹下的文件
//    AAsset *pathAsset = AAssetManager_open(asset_mgr, PNG_NAME, AASSET_MODE_UNKNOWN);
//    // 2.得到文件的长度
//    off_t assetLength = AAsset_getLength(pathAsset);
//    // 3.得到文件对应的 Buffer
//    auto *fileData = (unsigned char *) AAsset_getBuffer(pathAsset);
//    // 4.stb_image 的方法，从内存中加载图片
//    png_data = stbi_load_from_memory(fileData, (int)assetLength, &width, &height, &nrChannels, 0);
//    //加载图片1
//    PNGData png_data;
//    PngTool::loadPngFromAssets(asset_mgr, PNG_NAME, &png_data);
//
//    cv::Mat view(width, height, CV_8UC3, cv::Scalar(0, 0, 0));
//    view.data = png_data; //uchar*转Mat


    //*********************************************************************************************
    //不进行YUV2RGB了，直接读纹理出来
    glUseProgram(frame_program->program_id);
//    cv::Mat view2(win_w, win_h, CV_8UC3, cv::Scalar(0, 0, 0));
//    //指针pBuffer数据还原到Mat数组
//    if(pBuffer!= nullptr)
//    {
//        view2.data = pBuffer;
//    }

    //*********************************************************************************************
    //计算直方图
//    CalHistgram(view2);
    CalHistgramPointer(pBuffer); //pBuffer大小：sizeof(uint8_t) * img_w * img_h * 3

    //*********************************************************************************************
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

    //*********************************************************************************************
    // 申请锁
    pthread_mutex_lock(&frame_lock);
    //更新顶点、纹理坐标 vec4 ： m_pCombile
    UpdateMesh();
    // 释放锁
    pthread_mutex_unlock(&frame_lock);

    //*********************************************************************************************
    //打印显示一下（第二个点）
    // LOGI("[Combile]:[%f,%f,%f,%f]",m_pCombile[100 * 6 + 5].x, m_pCombile[100 * 6 + 5].y, m_pCombile[100 * 6 + 5].z, m_pCombile[100 * 6 + 5].w); //打印显示

    //绑定FBO的vbo
    if(fbo_vbo == 0)
    {
        glGenBuffers(1, &fbo_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, fbo_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_RenderDataSize * 6 * 4, m_pCombile, GL_STATIC_DRAW);
    }else
    {
        glBindBuffer(GL_ARRAY_BUFFER, fbo_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * m_RenderDataSize * 6 * 4, m_pCombile);
    }
    //绑定FBO的vao
    if(fbo_vao == GL_NONE)
    {
        glGenVertexArrays(1, &fbo_vao);
        glBindVertexArray(fbo_vao);
        //FBO顶点缓冲
        glBindBuffer(GL_ARRAY_BUFFER, fbo_vbo);
        // 一次性指定顶点坐标&纹理坐标
        // glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        // glEnableVertexAttribArray(0); // *启用数组*
        // 1.先指定顶点坐标
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // 2.再指定纹理坐标
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
//    else
//    {
//        glBindVertexArray(fbo_vao);
//        //FBO顶点缓冲
//        glBindBuffer(GL_ARRAY_BUFFER, fbo_vbo);
//        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
//        glEnableVertexAttribArray(0); // *启用数组*
//    }
    //****************************************
    //绑定顶点&缓冲
    if(D2_vbo == 0)
    {
        glGenBuffers(1, &D2_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, D2_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vert_2D), vert_2D, GL_STATIC_DRAW);
    }else
    {
        glBindBuffer(GL_ARRAY_BUFFER, D2_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vert_2D), vert_2D);
    }
    if(D2_vao == GL_NONE)
    {
        glGenVertexArrays(1, &D2_vao);
        glBindVertexArray(D2_vao);
        //普通2D
        glBindBuffer(GL_ARRAY_BUFFER, D2_vbo);
        // 一次性指定顶点坐标&纹理坐标
        // glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        // glEnableVertexAttribArray(0); // *启用数组*
        // 1.先指定顶点坐标
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // 2.再指定纹理坐标
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        CHECK_GL_ERR("vao_vbo_enable_init");
    }
//    else
//    {
//        glBindVertexArray(D2_vao);
//        //普通2D
//        glBindBuffer(GL_ARRAY_BUFFER, D2_vbo);
//        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
//        glEnableVertexAttribArray(0); // *启用数组*
//    }
    CHECK_GL_ERR("vao_vbo_bind_init");

    //*********************************************************************************************

}

//循环渲染部分
void FBOHist::render()
{
    // 清理颜色缓冲区
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // 要开启混色模式
    glViewport(0.0, 0.0, (GLsizei)win_w, (GLsizei)win_h);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA); //混合模式会导致两个纹理显示效果
//    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    //渲染视频帧
    render_frame();
    long long t0 = GetSysCurrentTime();
    //读取BaseVideoRender的数据
    readBasePixel();
    //计算直方图的顶点数组
    render_Mesh();
    //渲染滤镜
    render_FBOHist();
    long long t1 = GetSysCurrentTime();
    LOGI("TIME[renderFBO]:%lld ms",(t1-t0));
    //保存照片
//    save_picture();
}

//读取BaseVideoRender的数据
void FBOHist::readBasePixel()
{
    if(img_w == 0 || img_h == 0) {
        return;
    }
    if(first){
        pBuffer = new uint8_t[img_w*img_h * 3];
        first = false;
    }
    glReadPixels(0, 0, img_w, img_h, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);
}

//渲染本文件部分
void FBOHist::render_FBOHist()
{

    //****************************************
    //使用fbo程序
    glUseProgram(fbo_program->program_id);
    //转换矩阵
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(0.5f, 0.5f, 0.0f)); //缩放
    // Model = glm::translate(Model, glm::vec3(0.9f, 0.0f, 0.0f));          //平移
    // Model = glm::rotate(Model, glm::radians(270.0f), glm::vec3(0.0f, 0.0f, 1.0f)); //旋转
    // glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)win_w / (float)win_h, 0.1f, 100.0f); //参数（视野角、宽高比、平截头体近平面1.0f、远平面100.0f）
    glm::mat4 m_MVPMatrix = Model; //矩阵左乘 Projection * View *
    glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]); //uniform传矩阵
    //绑定FBO准备渲染
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
    //清除FBO导致重影(防止重影、不消失)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(fbo_vao);
    glBindBuffer(GL_ARRAY_BUFFER, fbo_vbo);
    //FBO离屏渲染
    //直方图只用到了坐标，没有传纹理
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, tex_id);
    // glUniform1i(m_FboSamplerLoc, 0);
    CHECK_GL_ERR("FBO_draw");
    //***========***
    glDrawArrays(GL_TRIANGLES, 0, m_RenderDataSize*6);
    //***========***
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);


    //****************************************
    //普通2D渲染（显示FBO结果的直方图）
//    glViewport(0.0, 0.0, win_w, win_h);
    glUseProgram(my_program->program_id);
    glBindVertexArray(D2_vao);  // 绑定vao和vbo
    glBindBuffer(GL_ARRAY_BUFFER, D2_vbo);
    glm::mat4 rotation = glm::mat4(1.0f);
    // glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians((float )(-1 * img_rotation)), glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(frame_rotation_loc, 1, GL_FALSE, glm::value_ptr(rotation));
    //激活纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glUniform1i(m_Loc, 0);
    CHECK_GL_ERR("2D_draw");
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindVertexArray(GL_NONE);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

//自动计算hist顶点坐标
void FBOHist::UpdateMesh()
{
    float dy = 0.5f / (float)MAX_VALUE_LEVEL;  //0.5f控制渲染时纵坐标高度
    float dx = 1.0f / (float)m_RenderDataSize; //1.0f控制渲染时横坐标宽度
//    memset(m_pCombile, 0, sizeof(glm::vec4)*m_RenderDataSize*6);
//    memset(m_pVerticesCoords, 0, sizeof(glm::vec3)*m_RenderDataSize*6);
    //循环每个正方形(两个三角形)
    for (int i = 0; i < m_RenderDataSize; ++i)
    {
        float y = (float)m_HistData[i] * dy * -1;
        //LOGI("[MAX_VALUE_LEVEL]:%d, [dy]:%f, [y]: %f",MAX_VALUE_LEVEL,dy,y);
        y = y < 0 ? y : -y;
        glm::vec2 p1((float)i * dx, 0 + 1.0f);
        glm::vec2 p2((float)i * dx, y + 1.0f);
        glm::vec2 p3(((float)i + 1.0f) * dx, y + 1.0f);
        glm::vec2 p4(((float)i + 1.0f) * dx, 0 + 1.0f);
        m_pVerticesCoords[i * 6 + 0] = texCoordToVertexCoord(p1);
        m_pVerticesCoords[i * 6 + 1] = texCoordToVertexCoord(p2);
        m_pVerticesCoords[i * 6 + 2] = texCoordToVertexCoord(p4);
        m_pVerticesCoords[i * 6 + 3] = texCoordToVertexCoord(p4);
        m_pVerticesCoords[i * 6 + 4] = texCoordToVertexCoord(p2);
        m_pVerticesCoords[i * 6 + 5] = texCoordToVertexCoord(p3);
        glm::vec2 LD(0.0, 1.0);
        glm::vec2 LT(0.0, 0.0);
        glm::vec2 RD(1.0, 1.0);
        glm::vec2 RT(1.0, 0.0);
        m_pCombile[i * 6 + 0] = Combile(m_pVerticesCoords[i * 6 + 0],LD);
        m_pCombile[i * 6 + 1] = Combile(m_pVerticesCoords[i * 6 + 1],LT);
        m_pCombile[i * 6 + 2] = Combile(m_pVerticesCoords[i * 6 + 2],RD);
        m_pCombile[i * 6 + 3] = Combile(m_pVerticesCoords[i * 6 + 3],RD);
        m_pCombile[i * 6 + 4] = Combile(m_pVerticesCoords[i * 6 + 4],LT);
        m_pCombile[i * 6 + 5] = Combile(m_pVerticesCoords[i * 6 + 5],RT);
    }
}

glm::vec3 FBOHist::texCoordToVertexCoord(const glm::vec2 &texCoord)
{
    return {2 * texCoord.x - 1, 1 - 2 * texCoord.y, 0};
}

glm::vec4 FBOHist::Combile(const glm::vec3 &VertCoord, glm::vec2 &texCoord)
{
    return {VertCoord.x, VertCoord.y, texCoord.x, texCoord.y};
}

void FBOHist::CalHistgramPointer(const uint8_t *imgDataPtr)
{
    vector<vector<int>>Histogram(256, {0}); //初始化为0
    for (int i = 0; i < Histogram.size(); ++i)
        Histogram[i].resize(4);
    for (int row = 0; row < win_w; row+=8)
    {
        for (int col = 0; col < win_h; col+=8)
        {
            Histogram[imgDataPtr[3 * (row * win_h + col) + 2]][0]++; //当前位置像素R值
            Histogram[imgDataPtr[3 * (row * win_h + col) + 1]][1]++; //当前位置像素G值
            Histogram[imgDataPtr[3 * (row * win_h + col) + 0]][2]++; //当前位置像素B值
            Histogram[(unsigned long)(imgDataPtr[3*(row * win_h + col)+2]*0.299 + imgDataPtr[3*(row * win_h + col)+1]*0.587 + imgDataPtr[3*(row*win_h+col)+0]*0.114)][3]++;
//            LOGI("[R,G,B]:[%d,%d,%d]", Histogram[imgDataPtr[3 * (row * win_h + col) + 2]][0],Histogram[imgDataPtr[3 * (row * win_h + col) + 1]][1],Histogram[imgDataPtr[3 * (row * win_h + col) + 0]][2]);
        }
    }
    MAX_VALUE_LEVEL = 0;
    for(int n = 0;n<255;n++)
    {
        m_HistData[n] = (short)Histogram[n][0]; //R通道
//        m_HistData[n] = (short)Histogram[n][1]; //G通道
//        m_HistData[n] = (short)Histogram[n][2]; //B通道
//        m_HistData[n] = (short)Histogram[n][3]; //灰度直方图
        //求直方图的最值
        if(m_HistData[n]>MAX_VALUE_LEVEL)
        {
            MAX_VALUE_LEVEL = m_HistData[n];
        }
        // LOGI("[R]:%d  max:%d", Histogram[n][0],MAX_VALUE_LEVEL);
    }
}


