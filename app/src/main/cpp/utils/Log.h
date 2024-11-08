//
// Created by ldh on 2022/4/18.
//

#ifndef CAMERASPECIALEFFECT_LOG_H
#define CAMERASPECIALEFFECT_LOG_H

#include <android/log.h>
#define LOG_TAG "[LOG]"

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG,__VA_ARGS__)


#endif //CAMERASPECIALEFFECT_LOG_H
