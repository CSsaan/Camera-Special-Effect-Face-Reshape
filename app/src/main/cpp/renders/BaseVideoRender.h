//
// Created by ldh on 2022/4/18.
//

#ifndef CS_BASEVIDEORENDER_H
#define CS_BASEVIDEORENDER_H
#include <GLES3/gl32.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <pthread.h>
#include "../utils/ProgramLoader.h"

class BaseVideoRender
{

protected:
    // 窗口的宽和高
    size_t win_w = 0;
    size_t win_h = 0;
    // 图像的数据
    unsigned char *yuv = nullptr;
    size_t img_w = 0;
    size_t img_h = 0;
    int img_rotation = 0; // 旋转角度
    // 是否使用滤波核进行美颜
    bool use_gaussian_filter = false;
    // 锁用于锁定 yuv 数据
    pthread_mutex_t frame_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    // 加载帧渲染程序
    void load_frame_program();
    void load_frame_program_with_file(const std::string &vert_shader_file, const std::string &frag_shader_file);
    std::unique_ptr<ProgramLoader> frame_program;
    GLuint frame_vbo;
    GLuint frame_vao;
    GLint frame_rotation_loc{}; // frame_rotation 在程序中的位置
    GLuint frame_texture_id[3]{};
    // 和保存相关的信息
    std::string save_dir;
    bool need_save_next = false;
    GLint img_size_loc{};
    GLint use_front_loc{};
    // 初始化gl信息
    void init_frame_data();
    static AAssetManager *asset_mgr;

    // 渲染视频帧
    void render_frame();

public:
    // 设置 asset_mgr
    static void setAssetManager(AAssetManager *mgr);
    BaseVideoRender(bool use_gaussian_filter_ = false, std::string vert_shader_file = "", std::string frag_shader_file = "");
    // 设置窗口的宽和高
    void setWindowSize(int w, int h);
    // 设置YUV数据
    void setYUVData(unsigned char *data, int width, int height, int rotation);
    // 析构函数
    virtual ~BaseVideoRender();
    // 进行渲染
    virtual void render();
    // 设置进行保存
    void save_next_frame(const char *path);
    // 检查错误
    static void CHECK_GL_ERR(const char *msg);

    // 设置双边滤波sigmaA
    float sigmaA = 0.15;
    float sigmaB = 0.05;
    //    float green_strenth = 0.9;
    //    float green_pow = 0.5;
    void setSigmaA(int);
    void setSigmaB(int);
    // 相机是否朝前
    bool use_front = false;
};

#endif // CS_BASEVIDEORENDER_H
