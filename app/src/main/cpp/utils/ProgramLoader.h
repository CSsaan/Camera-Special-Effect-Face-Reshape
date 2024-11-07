/*
 * Created by cs on 2022/11/1.
 *
 * 该文件所完成的OpenGL创建program内容:
 *  * loadShaderFile：
 *      * 读取GLSL着色器代码;shader创建'glCreateShader';链接'glShaderSource';
 *        编译'glCompileShader'.
 *      * @return glCreateShader的句柄.
 *  * ProgramLoader:
 *      * 构造函数完成program创建.'glCreateProgram';着色器与program链接'glAttachShader';
 *        program链接'glLinkProgram'.
 *
 * 使用方法：
 * 1.首先创建对象
 * @param logTag: 用于LOGI显示当前program的名称提示;
 * @param vert_file_name: 顶点着色器shader文件路径;
 * @param frag_file_name: 片段着色器shader文件路径;
 * @param asset_mgr: 从Java得到的AssetManager;
 * @return frame_program: 当前创建的类对象.
 * frame_program = std::make_unique<ProgramLoader>("logTag", vert_file_name, frag_file_name, mgr);
 *
 * 2.检测创建成功
   if(frame_program->status != 0) {
        LOGE("BaseVideoRender load_frame_program error");
    }
 *
 * 3.使用program
 * @param program_id: 当前创建的program的ID;
 * glUseProgram(frame_program->program_id).
 */

#ifndef CAMERASPECIALEFFECT_PROGRAMLOADER_H
#define CAMERASPECIALEFFECT_PROGRAMLOADER_H

#include "Log.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <GLES3/gl32.h>
#include <string>

// 用于程序的加载
class ProgramLoader
{
private:
    GLuint vert_shader;
    GLuint frag_shader;

    // 加载着色器
    GLuint loadShaderFile(AAssetManager *mgr, const std::string& file_name, int shader_type);

    // 创建程序
    std::string log_tag;

public:
    GLuint program_id;
    int status;
    ProgramLoader(const std::string& logTag, const std::string& vert_file_name, const std::string& frag_file_name, AAssetManager *mgr);
    ~ProgramLoader();
};


#endif //CAMERASPECIALEFFECT_PROGRAMLOADER_H
