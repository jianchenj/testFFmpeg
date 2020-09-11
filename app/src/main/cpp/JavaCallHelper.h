//
// Created by Administrator on 2020/9/1.
//

#ifndef MY_APPLICATION_JAVACALLHELPER_H
#define MY_APPLICATION_JAVACALLHELPER_H

#include "jni.h"
extern "C"
{
#include "libavcodec/avcodec.h"
}

class JavaCallHelper {

public:
    //构造函数
    JavaCallHelper(JavaVM * _javaVM , JNIEnv* _env , jobject &_obj);
    ~JavaCallHelper();

    void onPrepare(int thread);

    void onProgress(int thread, int progress);

    void onError(int thread, int code);

private:
    JavaVM* javaVm;//jvm
    JNIEnv* env;//jni环境
    jobject jobj;//java层传过来的对象,回调的就是他的方法
    jmethodID jmid_prepare;//方法id
    jmethodID jmid_error;
    jmethodID jmid_progress;
};


#endif //MY_APPLICATION_JAVACALLHELPER_H
