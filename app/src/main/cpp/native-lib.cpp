#include <jni.h>
#include <string>
#include <GLES3/gl3.h>
#include "renders/BaseVideoRender.h"
#include "renders/FaceReshape.h"
#include "renders/FaceReshape2.h"
#include "renders/FaceMask.h"
#include "renders/ToothWhiten.h"
#include "renders/BigEyes.h"

#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include "pthread.h"

// *定义窗口的宽高*
size_t g_width = 0, g_height = 0;
int typeJNI = 0;
// *全局的AssetManager*
AAssetManager *g_asset_mgr = nullptr;

// *渲染器类型*
int render_type = 0;

// 全局的渲染器
BaseVideoRender *g_render = nullptr;
//OpenCL *c_render = nullptr;
pthread_mutex_t render_mutex;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameraspecialeffectFaceReshape_GLRenderJNI_setSizeJNI(JNIEnv *env, jclass clazz, jint width,
                                                            jint height) {
    g_height = height;
    g_width = width;
    if(g_render) {
        g_render->setWindowSize(width, height);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameraspecialeffectFaceReshape_GLRenderJNI_initRenderJNI(JNIEnv *env, jclass clazz,
                                                               jobject mgr, jobject surface) {
    if(env && mgr) {
        g_asset_mgr = AAssetManager_fromJava(env, mgr);
        if(g_asset_mgr == nullptr) {
            LOGE("[JNI-Interface] AAssetManager_fromJava() is NULL!");
        }
        // 设置 asset
        BaseVideoRender::setAssetManager(g_asset_mgr);
    } else {
        LOGE("[JNI-Interface] env or mgr is NULL!");
    }
    g_render = new BaseVideoRender();
    g_render->setWindowSize(g_width, g_height);
    pthread_mutex_init(&render_mutex, nullptr);
    LOGI("Render load succeed");
}

extern "C"
JNIEXPORT void JNICALL
//选择N种滤镜
Java_com_example_cameraspecialeffectFaceReshape_GLRenderJNI_setRenderTypeJNI(JNIEnv *env, jclass clazz, jint type) {
    if(type == render_type) {
        return;
    }
    render_type = type;
    delete g_render;
    g_render = nullptr;
    if(type == 0) { // 原始
        g_render = new BaseVideoRender(false);
    } else if(type == 1) {
        g_render = new FaceReshape();
    } else if(type == 2) {
        g_render = new FaceReshape2();
    } else if(type == 3) {
        g_render = new FaceMask();
    } else if(type == 4) {
        g_render = new ToothWhiten();
    } else if(type == 5) {
        g_render = new BigEyes();
    }


    if(g_render) {
        g_render->setWindowSize(g_width, g_height);
        LOGI("[setWindowSize] g_width:%zu, g_height:%zu", g_width,g_height);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameraspecialeffectFaceReshape_GLRenderJNI_releaseRenderJNI(JNIEnv *env, jclass clazz) {
    if(g_render) {
        delete g_render;
//        delete c_render;
        g_render = nullptr;
//        c_render = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameraspecialeffectFaceReshape_GLRenderJNI_sendFrameDataJNI(JNIEnv *env, jclass clazz,
                                                                  jbyteArray image, jint width, jint height,
                                                               jint orientation) {
    if(g_render == nullptr) {
        return;
    }
    // 读取数据
    size_t sz = env->GetArrayLength(image);
    auto* buffer = new unsigned char[sz];
    env->GetByteArrayRegion(image, 0, sz, reinterpret_cast<jbyte *>(buffer));
    // 设置获取camera图像数据
    if(g_render)
        g_render->setYUVData(buffer, width, height, orientation);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameraspecialeffectFaceReshape_GLRenderJNI_renderJNI(JNIEnv *env, jclass clazz) {
    if(g_render != nullptr) {
        g_render->render();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameraspecialeffectFaceReshape_GLRenderJNI_savePngJNI(JNIEnv *env, jclass clazz,
                                                            jstring dir) {
    const char* tmp = env->GetStringUTFChars(dir, nullptr);
    if(g_render) {
        g_render->save_next_frame(tmp);
    }
    env->ReleaseStringUTFChars(dir, tmp);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameraspecialeffectFaceReshape_GLRenderJNI_setSigmaAJNI(JNIEnv *env, jclass clazz,
                                                              jint number_a) {
    if(g_render) { //typeJNI == 31 &&
        g_render->setSigmaA(number_a);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_cameraspecialeffectFaceReshape_GLRenderJNI_setSigmaBJNI(JNIEnv *env, jclass clazz,
                                                              jint number_b) {
    if(g_render) { //typeJNI == 31 &&
        g_render->setSigmaB(number_b);
    }
}

//extern "C"
//JNIEXPORT void JNICALL
//Java_com_example_cameraspecialeffectBeauty_CLRenderJNI_renderOpenCLJNI(JNIEnv *env, jclass clazz) {
//    c_render->runCL();
//}

//extern "C"
//JNIEXPORT void JNICALL
//Java_com_example_cameraspecialeffectBeauty_GLRenderJNI_useFrontJNI(JNIEnv *env, jclass clazz,
//                                                             jint use_front) {
//    g_render->setUseFront(use_front);
//}