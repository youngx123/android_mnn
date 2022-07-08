## MNN Android 部署测试

利用 MNN 推理框架在安卓端进行部署，测试模型推理时间以及结果 

![](https://github.com/youngx123/android_mnn/blob/master/YOLOV5/result.jpg?raw=true)

VS中相同图像处理方法在安卓调用时不起作用，导致结果不一致
```C++
cv::resize(inputImg, image, cv::Size(INPUT_SIZE, INPUT_SIZE));
image.convertTo(image, CV_32FC3);
image = image / 255.0f;
```
改为用MNN 中的图像预处理函数(MNN 中推理精度设置也会导致安卓端和vs中结果不一致)
```c++
MNN::Tensor *input_tensor = model->getSessionInput(session, nullptr);
// pass image to model(rgb format, pixels minus by 0 and then divided by 255.0)
const float mean_vals[3] = { 0.0, 0.0, 0.0 };
const float norm_vals[3] = {1.0 / 255.0, 1.0 / 255.0, 1.0 / 255.0 };
std::shared_ptr<MNN::CV::ImageProcess> pretreat(MNN::CV::ImageProcess::create(MNN::CV::BGR, MNN::CV::RGB, mean_vals, 3, norm_vals, 3));
pretreat->convert(image.data, INPUT_SIZE, INPUT_SIZE,INPUT_SIZE*3, input_tensor);
```

```c++
enum PrecisionMode {
    Precision_Normal = 0,
    Precision_High,
    Precision_Low
};
int precision = 0;
int precision = 2; //结果不一致
MNN::BackendConfig backendConfig;
backendConfig.precision = (MNN::BackendConfig::PrecisionMode)precision;
```
### Note
安卓部署主要修改文件 ： 

`activaty_main.xml` app 显示界面设置 

`MainActivity.java` 主函数

`native-lib.cpp`  JNI调用c++ 

`CMakeLists.txt`  cmake添加所需的库和头文件

`build.gradle(:app)` 设置sdk, ndk版本，以及其他属性

.cpp/.h  c++调用模型进行图像处理文件

### java与c++数据转换
```java
jobjectArray result;
jclass intArrCls = env->FindClass("[I");
result = env->NewObjectArray(size, intArrCls, NULL);
```

是创建一个`jclass`的引用，因为 `result`的元素是一维int数组的引用，所以`intArrCls`
必须是一维`int`数组的引用，这一点是如何保证,注意`FindClass`的参数 `"[I"`，JNI就是
通过它来`确定引用的类型的`，`I`表示是`int`类型，`[标识是数组`。对于其他的类型，都有
相应的表示方法

```java
[I         // 代表一维整型数组，I 表示整型
[[I        // 代表二维整型数组
[Ljava/lang/String;      // 代表一维字符串数组，

Z boolean
B byte
C char
S short
I int
J long
F float
D double

String 是通过 Ljava/lang/String 表示的，那相应的， String 数组就应该是 [Ljava/lang/String;

result = env->NewObjectArray(size, intArrCls, NULL); //的作用是为result分配空间

jintArray iarr = env->NewIntArray(size);      //是为一维int数组iarr分配空间。

env->SetIntArrayRegion(iarr, 0, size, tmp);   //是为iarr赋值。

env->SetObjectArrayElement(result, i, iarr);  //是为result的第i个元素赋值。
```

__一维数组__
```java
int callbackMethod(int num,vector<int> vArray)
{
    int ret = 0;
    int needsDetach;
    JNIEnv *env = getJNIEnv(&needsDetach);
    jintArray jArray = env->NewIntArray(num);
    jint *jnum = new jint[num];
    for(int i = 0; i < num; ++i)
    {
        *(jnum + i) = vArray[i];
    }
    env->SetIntArrayRegion(jArray, 0, num, jnum);
	//现在得到了我们要的jArray，也就是回调函数中返回到Java端的int数组,
	//回调中的参数为(int a,int[] b);JNI中的类型为"I[I"
    env->CallVoidMethod(mobj, mid, num,jArray);

    jthrowable exception = env->ExceptionOccurred();
    return jArray;
}
```

__二维数组__
```JAVA
JNIEXPORT jobjectArray JNICALL Java_ObjectArrayTest_initInt2DArray(JNIEnv *env, jclass cls, int size)
{
    jobjectArray result;
    jclass intArrCls = env->FindClass("[I");
    result = env->NewObjectArray(size, intArrCls, NULL);
    for (int i = 0; i < size; i++)
	 {
        jint tmp[256];
        jintArray iarr = env->NewIntArray(size);
        for(int j = 0; j < size; j++) 
		  {
            tmp[j] = i + j;
        }
        env->SetIntArrayRegion(iarr, 0, size, tmp);
        env->SetObjectArrayElement(result, i, iarr);
        env->DeleteLocalRef(iarr);
    }
    return result; 
}
```

>参考

>https://www.jianshu.com/p/9ad1a7868e11

>https://github.com/nihui/ncnn-android-yolov5