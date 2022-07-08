#include <jni.h>
#include <string>
#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include <opencv2/opencv.hpp>
#include "yoloInference.h"
#include "stdio.h"
#include "stdlib.h"
#include <time.h>

extern "C" JNIEXPORT jobjectArray  JNICALL
Java_com_example_yolov5_MainActivity_resultJNI(
        JNIEnv* env,
        jobject obj,
        jobject bitmap,
        jstring jstr) {
        AndroidBitmapInfo info;
        void *pixels;
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat temp(info.height, info.width, CV_8UC4, pixels);
        cv::Mat temp2 = temp.clone();
        // 将jstring类型转换成C++里的const char*类型
        const char *path = env->GetStringUTFChars(jstr, 0);

         std::vector<std::vector<float>> results2;

         detection(path, temp2, results2);

        int nms_size = results2.size();
        int bboxInfo_size = results2[0].size();

        jclass FArrCls = (*env).FindClass("[F");
        jobjectArray jOArray = env->NewObjectArray(nms_size, FArrCls, 0);

        // 二维数组长度为2，循环赋值
        for (int i = 0; i < nms_size; i++)
        {
            jfloat ftmp[bboxInfo_size];
            jfloatArray jFarr = env->NewFloatArray(bboxInfo_size);
            for(int j = 0; j < bboxInfo_size; j++)
            {
                float value = results2[i][j];
                ftmp[j] = value;
            }
            env->SetFloatArrayRegion(jFarr, 0, bboxInfo_size, ftmp);
            env->SetObjectArrayElement(jOArray, i, jFarr);

//            jboolean jb = JNI_TRUE;
//            jfloat *a = env->GetFloatArrayElements(jiarr, &jb);
//            for(int j = 0; j < bboxInfo_size; j++)
//            {
//               value = jiarr[i];
//                float v;
//                v = tmp[j] - a[j];
//            }
            env->DeleteLocalRef(jFarr);
        }

        return jOArray;
    }
}
