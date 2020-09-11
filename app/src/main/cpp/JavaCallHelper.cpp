//
// Created by Administrator on 2020/9/1.
//

#include "JavaCallHelper.h"
#include "macro.h"

JavaCallHelper::JavaCallHelper(JavaVM *_javaVm, JNIEnv *_env, jobject &_obj): javaVm(_javaVm),
                                                                              env(_env){
//    javaVm = _javaVm;
//    env = _env;
    jobj = env->NewGlobalRef(_obj);//建立全局的引用对象jobj防止方法执行结束，内存被回收.
    jclass jclassz = env->GetObjectClass(jobj);//获取对象类
    //找到对应方法
    // "(I)V" ()内是参数类型，()后是返回类型
    jmid_error = env->GetMethodID(jclassz, "onError", "(I)V");
    jmid_progress = env->GetMethodID(jclassz, "onProgress", "(I)V");
    jmid_prepare = env->GetMethodID(jclassz, "onPrepare", "()V");
}

JavaCallHelper::~JavaCallHelper() {

}

void JavaCallHelper::onPrepare(int thread) {
    if (thread == THREAD_CHILD) {
        JNIEnv *jniEnv;
        if (javaVm->AttachCurrentThread(&jniEnv, 0 ) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_prepare);
        //调用完成之后需要解绑.
        javaVm->DetachCurrentThread();
    } else {
        env->CallVoidMethod(jobj, jmid_prepare);
    }
}

void JavaCallHelper::onProgress(int thread, int progress) {

}

void JavaCallHelper::onError(int thread, int code) {

}
