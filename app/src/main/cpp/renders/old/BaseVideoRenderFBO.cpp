//
// Created by cs on 2022/11/1.
//

#include "BaseVideoRender.h"
#include <cstdlib>
#include <cstring>
#include <jni.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <ctime>
#include <string>

#define FRAME_VERT_SHADER_FILE "shaders/frame_vert.glsl"
#define FRAME_FRAG_SHADER_FILE "shaders/frame_frag.glsl"
#define FRAME_2D_VERT_SHADER_FILE "shaders/2D_frame_vert.glsl"
#define FRAME_2D_FRAG_SHADER_FILE "shaders/2D_frame_frag.glsl"

AAssetManager* BaseVideoRender::asset_mgr = nullptr;

void BaseVideoRender::setAssetManager(AAssetManager *mgr) {
    asset_mgr = mgr;
}

BaseVideoRender::BaseVideoRender(bool use_gaussian_filter_, std::string vert_shader_file_, std::string frag_shader_file_) {
    use_gaussian_filter = use_gaussian_filter_;
    win_h = win_w = img_rotation = img_h = img_w = 0;
    frame_vao = frame_vbo = 0;
    frame_texture_id[0] = frame_texture_id[1] = frame_texture_id[2] = frame_texture_id[3] = 0;
    yuv = nullptr;
    rgb = nullptr;
//    pBuffer = new uint8_t[3648*2736 * 3];
    // 初始化锁
    pthread_mutex_init(&frame_lock, nullptr);
    // 加载帧程序
    if(vert_shader_file_.empty() || frag_shader_file_.empty()) {
        load_frame_program();
    } else {
        load_frame_program_with_file(vert_shader_file_, frag_shader_file_);
    }
    my_program = std::shared_ptr<ProgramLoader>(new ProgramLoader("FBORender", FRAME_2D_VERT_SHADER_FILE, FRAME_2D_FRAG_SHADER_FILE, asset_mgr));
    // 初始化其他信息
    init_frame_data();
}

BaseVideoRender::~BaseVideoRender() {
    // 释放 yuv 数据
    if(yuv != nullptr) {
        delete[] yuv;
        yuv = nullptr;
    }
    if(rgb != nullptr) {
        delete[] rgb;
        rgb = nullptr;
    }
    if(pBuffer != nullptr) {
        delete[] pBuffer;
        pBuffer = nullptr;
    }
    // 释放锁
    pthread_mutex_destroy(&frame_lock);
    // 释放缓冲
    glDeleteVertexArrays(1, &frame_vao);
    glDeleteBuffers(1, &frame_vbo);
    glDeleteVertexArrays(1, &frame_D2_vao);
    glDeleteBuffers(1, &frame_D2_vbo);
    glDeleteTextures(3, frame_texture_id);
}

void BaseVideoRender::setWindowSize(int w, int h) {
    win_h = h;
    win_w = w;
}

void BaseVideoRender::setYUVData(unsigned char* data, int width, int height, int rotation) { //rotation可默认0
    pthread_mutex_lock(&frame_lock);
    if(yuv != nullptr) {
        delete[] yuv;
        yuv = nullptr;
    }
    img_w = width; img_h = height; img_rotation = rotation;
    yuv = data;
    pthread_mutex_unlock(&frame_lock);
}

void BaseVideoRender::load_frame_program() {
    if(!use_gaussian_filter) {
        load_frame_program_with_file(FRAME_VERT_SHADER_FILE, FRAME_FRAG_SHADER_FILE);
    } else {
        load_frame_program_with_file(FRAME_VERT_SHADER_FILE, FRAME_FRAG_SHADER_FILE);
    }
}

void BaseVideoRender::load_frame_program_with_file(const std::string& vert_shader_file,const std::string& frag_shader_file) {
    frame_program = std::make_unique<ProgramLoader>("BaseVideoRender", vert_shader_file, frag_shader_file, asset_mgr);
    if(frame_program->status != 0) {
        LOGE("BaseVideoRender load_frame_program error");
    }
}

void BaseVideoRender::init_frame_data() {
    // 清理颜色缓冲区
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    // 禁用深度测试
    glDisable(GL_DEPTH_TEST);
    glUseProgram(frame_program->program_id);
    // FBO顶点坐标数据
    float vert_buf[] = {
            -1.0, 1.0, 0.0, 0.0, // 左上
            -1.0, -1.0, 0.0, 1.0, // 左下
            1.0, -1.0, 1.0, 1.0, // 右下
            -1.0, 1.0, 0.0, 0.0,  // 左上
            1.0, 1.0, 1.0, 0.0, // 右上
            1.0, -1.0, 1.0, 1.0  // 右下
    };
    //普通2D顶点数组
    float vert_2D[] = {
            -1.0, 1.0, 0.0, 0.0, // 左上
            -1.0, -1.0, 0.0, 1.0, // 左下
            1.0, -1.0, 1.0, 1.0, // 右下
            -1.0, 1.0, 0.0, 0.0,  // 左上
            1.0, 1.0, 1.0, 0.0, // 右上
            1.0, -1.0, 1.0, 1.0  // 右下
    };
    //绑定FBO的顶点&缓冲
    //FBO
    glGenVertexArrays(1, &frame_vao);
    glBindVertexArray(frame_vao);
    glGenBuffers(1, &frame_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, frame_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert_buf), vert_buf, GL_STATIC_DRAW);
    //绑定普通2D的顶点&缓冲
    glGenVertexArrays(1, &frame_D2_vao);
    glBindVertexArray(frame_D2_vao);
    glGenBuffers(1, &frame_D2_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, frame_D2_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert_2D), vert_2D, GL_STATIC_DRAW);
    CHECK_GL_ERR("vao_vbo_bind_init");
    //指定位置
    //FBO
    glBindVertexArray(frame_vao);
    glBindBuffer(GL_ARRAY_BUFFER, frame_vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);  // *启用数组*
    //普通2D
    glBindVertexArray(frame_D2_vao);
    glBindBuffer(GL_ARRAY_BUFFER, frame_D2_vbo);
    // 1.先指定顶点坐标
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // 2.再指定纹理坐标
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    CHECK_GL_ERR("vao_vbo_enable_init");

    // 创建3个用于连接FBO的颜色附着(传入Y\U\V纹理图片)
    // 发送普通2D纹理数据
    unsigned char *y_plane = yuv,
            *u_plane = yuv + img_w * img_h,
            *v_plane = yuv + img_w * img_h + (img_w * img_h) / 4;
//    LOGI("[Y:%d]",y_plane[1]);
    glGenTextures(3, frame_texture_id);
    {
        glBindTexture(GL_TEXTURE_2D, frame_texture_id[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, img_w, img_h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }
    {
        glBindTexture(GL_TEXTURE_2D, frame_texture_id[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, img_w / 2, img_h / 2, 0, GL_RED, GL_UNSIGNED_BYTE,nullptr);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }
    {
        glBindTexture(GL_TEXTURE_2D, frame_texture_id[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, img_w / 2, img_h / 2, 0, GL_RED, GL_UNSIGNED_BYTE,nullptr);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }
    // 创建一个用来普通2D显示的纹理
    glGenTextures(1, &frame_2DFboTextureId);
    glBindTexture(GL_TEXTURE_2D, frame_2DFboTextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    // 创建一个用来普通2D显示的纹理
    glGenTextures(1, &tttt);
    glBindTexture(GL_TEXTURE_2D, tttt);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    // 初始化FBO
    glGenFramebuffers(1, &frame_FboId);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_FboId);
    // FBO绑定到纹理
    glBindTexture(GL_TEXTURE_2D, frame_2DFboTextureId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_2DFboTextureId, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE)
    {
        LOGI("PBOSample::CreateFrameBufferObj glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
    }
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    CHECK_GL_ERR("CreateFrameBufferObj");
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    //*****************************************************************************************
    //  获取 rotation 矩阵
    frame_rotation_loc = glGetUniformLocation(frame_program->program_id, "frame_rotation");
    img_size_loc = glGetUniformLocation(frame_program->program_id, "img_size");
//    LOGI("[BaseVideoRender] frame_rotation_loc: %d", frame_rotation_loc);
}


void BaseVideoRender::render() {
    // 渲染帧
    render_frame();
    // 保存当前的图片
    //save_picture(); //不调用第三方库保存了
}

void BaseVideoRender::render_frame() {
    if(frame_program == nullptr || yuv == nullptr || frame_program->status == -1) {
        return;
    }
    //*****************************************************************************************
    //FBO渲染
    // 使用当前程序
    glUseProgram(frame_program->program_id);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_FboId);
    // 设置视口
    glViewport(0.0, 0.0, this->win_w, this->win_h);
    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    // 绑定vao和vbo
    glBindVertexArray(frame_vao);
    // 申请锁
    pthread_mutex_lock(&frame_lock);
    // 发送统一变量
    glUniform2f(img_size_loc, win_w, win_h);
//    glUniform2f(img_size_loc, img_h, img_w);
//    LOGI("[BaseVIdeoRender] tex_size_loc:%d, %d, %d",img_size_loc, win_w, win_h);
    // 计算旋转矩阵并发送
//    glm::mat4 pers = glm::perspective(1.0472f, (float)win_w / ((float)win_h + 0.001f), 0.1f, 1000.0f);  // 透视投影
//    glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));    // 平移
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians((float )(img_rotation)), glm::vec3(0.0f, 0.0f, 1.0f));
//    rotation = pers * trans * rotation;
    glUniformMatrix4fv(frame_rotation_loc, 1, GL_FALSE, glm::value_ptr(rotation));

    // 发送Y\U\V纹理数据
    unsigned char *y_plane = yuv,
            *u_plane = yuv + img_w * img_h,
            *v_plane = yuv + img_w * img_h + (img_w * img_h) / 4;
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, frame_texture_id[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, img_w, img_h, 0, GL_RED, GL_UNSIGNED_BYTE, y_plane);
    }
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, frame_texture_id[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, img_w / 2, img_h / 2, 0, GL_RED, GL_UNSIGNED_BYTE, u_plane);
    }
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, frame_texture_id[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, img_w / 2, img_h / 2, 0, GL_RED, GL_UNSIGNED_BYTE, v_plane);
    }
    // 释放锁
    pthread_mutex_unlock(&frame_lock);
    // 进行绘制
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    //*****************************************************************************************
    //普通2D渲染
    glViewport(0.0, 0.0, win_w, win_h);
    glUseProgram(my_program->program_id);
    glBindVertexArray(frame_D2_vao);  // 绑定vao和vbo
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frame_2DFboTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    CHECK_GL_ERR("2D_draw");
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindVertexArray(GL_NONE);
//    //用来传给FBOHist，计算直方图,glReadPixels
//    pBuffer = new uint8_t[win_w*win_h * 4];
//    glReadPixels(0, 0, win_w, win_h, GL_RGBA, GL_UNSIGNED_BYTE, pBuffer);
//    LOGI("[pBuffer] pBuffer: %d",pBuffer[100]);
}

void BaseVideoRender::save_next_frame(const char *path) {
    save_dir = std::string(path);
    need_save_next = true;
}
//两个滑动条
void BaseVideoRender::setSigmaA(int numberA)
{
    sigmaA = (float)numberA;
    green_strenth = (float)numberA/100;
}
void BaseVideoRender::setSigmaB(int numberB)
{
    sigmaB = (float)numberB/100;
    green_pow = (float)numberB/60 + (float)0.5;
}
void BaseVideoRender::CHECK_GL_ERR(const char *msg)
{
    GLenum err = glGetError();
    if (err)
        LOGI("%s:glErr=%d",msg,err);
}

