#-------------------------------------------------
#
# Project created by QtCreator 2016-12-08T00:00:30
#
#-------------------------------------------------

QT       += core gui network xml androidextras multimedia sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = o2oAttendance
TEMPLATE = app


SOURCES += main.cpp\
        maindialog.cpp \
    inputkeydialog.cpp \
    native.cpp \
    fingerprint.cpp \
    facial.c \
    uart.c \
    tcpforclient.cpp \
    clientthread.cpp \
    json/json_scanner.cpp \
    json/parser.cpp \
    json/parserrunnable.cpp \
    json/qobjecthelper.cpp \
    json/serializer.cpp \
    json/serializerrunnable.cpp \
    JQChecksum.cpp \
    json/json_parser.cc \
    json/json_scanner.cc \
    stm32comm.cpp \
    healthtestthread.cpp \
    alcoholtestthread.cpp \
    alcoholadjustdialog.cpp \
    facialthread.cpp \
    devicemanager.cpp \
    camera.c \
    uploadthread.cpp \
    dbmanager.cpp

HEADERS  += maindialog.h \
    inputkeydialog.h \
    libusb.h \
    fingerprint.h \
    facial.h \
    uart.h \
    tcpforclient.h \
    clientthread.h \
    json/FlexLexer.h \
    json/json_parser.hh \
    json/json_scanner.h \
    json/location.hh \
    json/parser.h \
    json/parser_p.h \
    json/parserrunnable.h \
    json/qjson_debug.h \
    json/qjson_export.h \
    json/qobjecthelper.h \
    json/serializer.h \
    json/serializerrunnable.h \
    JQChecksum.h \
    common.h \
    json/json_parser.yy \
    json/json_scanner.yy \
    json/position.hh \
    json/stack.hh \
    stm32comm.h \
    healthtestthread.h \
    alcoholtestthread.h \
    alcoholadjustdialog.h \
    facialthread.h \
    devicemanager.h \
    libusbi.h \
    include/opencv/cv.h \
    include/opencv/cv.hpp \
    include/opencv/cvaux.h \
    include/opencv/cvaux.hpp \
    include/opencv/cvwimage.h \
    include/opencv/cxcore.h \
    include/opencv/cxcore.hpp \
    include/opencv/cxeigen.hpp \
    include/opencv/cxmisc.h \
    include/opencv/highgui.h \
    include/opencv/ml.h \
    include/opencv2/calib3d/calib3d.hpp \
    include/opencv2/calib3d/calib3d_c.h \
    include/opencv2/core/cuda/detail/color_detail.hpp \
    include/opencv2/core/cuda/detail/reduce.hpp \
    include/opencv2/core/cuda/detail/reduce_key_val.hpp \
    include/opencv2/core/cuda/detail/transform_detail.hpp \
    include/opencv2/core/cuda/detail/type_traits_detail.hpp \
    include/opencv2/core/cuda/detail/vec_distance_detail.hpp \
    include/opencv2/core/cuda/block.hpp \
    include/opencv2/core/cuda/border_interpolate.hpp \
    include/opencv2/core/cuda/color.hpp \
    include/opencv2/core/cuda/common.hpp \
    include/opencv2/core/cuda/datamov_utils.hpp \
    include/opencv2/core/cuda/dynamic_smem.hpp \
    include/opencv2/core/cuda/emulation.hpp \
    include/opencv2/core/cuda/filters.hpp \
    include/opencv2/core/cuda/funcattrib.hpp \
    include/opencv2/core/cuda/functional.hpp \
    include/opencv2/core/cuda/limits.hpp \
    include/opencv2/core/cuda/reduce.hpp \
    include/opencv2/core/cuda/saturate_cast.hpp \
    include/opencv2/core/cuda/scan.hpp \
    include/opencv2/core/cuda/simd_functions.hpp \
    include/opencv2/core/cuda/transform.hpp \
    include/opencv2/core/cuda/type_traits.hpp \
    include/opencv2/core/cuda/utility.hpp \
    include/opencv2/core/cuda/vec_distance.hpp \
    include/opencv2/core/cuda/vec_math.hpp \
    include/opencv2/core/cuda/vec_traits.hpp \
    include/opencv2/core/cuda/warp.hpp \
    include/opencv2/core/cuda/warp_reduce.hpp \
    include/opencv2/core/cuda/warp_shuffle.hpp \
    include/opencv2/core/hal/hal.hpp \
    include/opencv2/core/hal/interface.h \
    include/opencv2/core/hal/intrin.hpp \
    include/opencv2/core/hal/intrin_cpp.hpp \
    include/opencv2/core/hal/intrin_neon.hpp \
    include/opencv2/core/hal/intrin_sse.hpp \
    include/opencv2/core/affine.hpp \
    include/opencv2/core/base.hpp \
    include/opencv2/core/bufferpool.hpp \
    include/opencv2/core/core.hpp \
    include/opencv2/core/core_c.h \
    include/opencv2/core/cuda.hpp \
    include/opencv2/core/cuda.inl.hpp \
    include/opencv2/core/cuda_stream_accessor.hpp \
    include/opencv2/core/cuda_types.hpp \
    include/opencv2/core/cvdef.h \
    include/opencv2/core/cvstd.hpp \
    include/opencv2/core/cvstd.inl.hpp \
    include/opencv2/core/directx.hpp \
    include/opencv2/core/eigen.hpp \
    include/opencv2/core/fast_math.hpp \
    include/opencv2/core/ippasync.hpp \
    include/opencv2/core/mat.hpp \
    include/opencv2/core/mat.inl.hpp \
    include/opencv2/core/matx.hpp \
    include/opencv2/core/neon_utils.hpp \
    include/opencv2/core/ocl.hpp \
    include/opencv2/core/ocl_genbase.hpp \
    include/opencv2/core/opengl.hpp \
    include/opencv2/core/operations.hpp \
    include/opencv2/core/optim.hpp \
    include/opencv2/core/ovx.hpp \
    include/opencv2/core/persistence.hpp \
    include/opencv2/core/private.cuda.hpp \
    include/opencv2/core/private.hpp \
    include/opencv2/core/ptr.inl.hpp \
    include/opencv2/core/saturate.hpp \
    include/opencv2/core/sse_utils.hpp \
    include/opencv2/core/traits.hpp \
    include/opencv2/core/types.hpp \
    include/opencv2/core/types_c.h \
    include/opencv2/core/utility.hpp \
    include/opencv2/core/va_intel.hpp \
    include/opencv2/core/version.hpp \
    include/opencv2/core/wimage.hpp \
    include/opencv2/features2d/features2d.hpp \
    include/opencv2/flann/all_indices.h \
    include/opencv2/flann/allocator.h \
    include/opencv2/flann/any.h \
    include/opencv2/flann/autotuned_index.h \
    include/opencv2/flann/composite_index.h \
    include/opencv2/flann/config.h \
    include/opencv2/flann/defines.h \
    include/opencv2/flann/dist.h \
    include/opencv2/flann/dummy.h \
    include/opencv2/flann/dynamic_bitset.h \
    include/opencv2/flann/flann.hpp \
    include/opencv2/flann/flann_base.hpp \
    include/opencv2/flann/general.h \
    include/opencv2/flann/ground_truth.h \
    include/opencv2/flann/hdf5.h \
    include/opencv2/flann/heap.h \
    include/opencv2/flann/hierarchical_clustering_index.h \
    include/opencv2/flann/index_testing.h \
    include/opencv2/flann/kdtree_index.h \
    include/opencv2/flann/kdtree_single_index.h \
    include/opencv2/flann/kmeans_index.h \
    include/opencv2/flann/linear_index.h \
    include/opencv2/flann/logger.h \
    include/opencv2/flann/lsh_index.h \
    include/opencv2/flann/lsh_table.h \
    include/opencv2/flann/matrix.h \
    include/opencv2/flann/miniflann.hpp \
    include/opencv2/flann/nn_index.h \
    include/opencv2/flann/object_factory.h \
    include/opencv2/flann/params.h \
    include/opencv2/flann/random.h \
    include/opencv2/flann/result_set.h \
    include/opencv2/flann/sampling.h \
    include/opencv2/flann/saving.h \
    include/opencv2/flann/simplex_downhill.h \
    include/opencv2/flann/timer.h \
    include/opencv2/highgui/highgui.hpp \
    include/opencv2/highgui/highgui_c.h \
    include/opencv2/imgcodecs/imgcodecs.hpp \
    include/opencv2/imgcodecs/imgcodecs_c.h \
    include/opencv2/imgcodecs/ios.h \
    include/opencv2/imgproc/detail/distortion_model.hpp \
    include/opencv2/imgproc/hal/hal.hpp \
    include/opencv2/imgproc/hal/interface.h \
    include/opencv2/imgproc/imgproc.hpp \
    include/opencv2/imgproc/imgproc_c.h \
    include/opencv2/imgproc/types_c.h \
    include/opencv2/ml/ml.hpp \
    include/opencv2/objdetect/detection_based_tracker.hpp \
    include/opencv2/objdetect/objdetect.hpp \
    include/opencv2/objdetect/objdetect_c.h \
    include/opencv2/photo/cuda.hpp \
    include/opencv2/photo/photo.hpp \
    include/opencv2/photo/photo_c.h \
    include/opencv2/shape/emdL1.hpp \
    include/opencv2/shape/hist_cost.hpp \
    include/opencv2/shape/shape.hpp \
    include/opencv2/shape/shape_distance.hpp \
    include/opencv2/shape/shape_transformer.hpp \
    include/opencv2/stitching/detail/autocalib.hpp \
    include/opencv2/stitching/detail/blenders.hpp \
    include/opencv2/stitching/detail/camera.hpp \
    include/opencv2/stitching/detail/exposure_compensate.hpp \
    include/opencv2/stitching/detail/matchers.hpp \
    include/opencv2/stitching/detail/motion_estimators.hpp \
    include/opencv2/stitching/detail/seam_finders.hpp \
    include/opencv2/stitching/detail/timelapsers.hpp \
    include/opencv2/stitching/detail/util.hpp \
    include/opencv2/stitching/detail/util_inl.hpp \
    include/opencv2/stitching/detail/warpers.hpp \
    include/opencv2/stitching/detail/warpers_inl.hpp \
    include/opencv2/stitching/warpers.hpp \
    include/opencv2/superres/optical_flow.hpp \
    include/opencv2/video/background_segm.hpp \
    include/opencv2/video/tracking.hpp \
    include/opencv2/video/tracking_c.h \
    include/opencv2/video/video.hpp \
    include/opencv2/videoio/cap_ios.h \
    include/opencv2/videoio/videoio.hpp \
    include/opencv2/videoio/videoio_c.h \
    include/opencv2/videostab/deblurring.hpp \
    include/opencv2/videostab/fast_marching.hpp \
    include/opencv2/videostab/fast_marching_inl.hpp \
    include/opencv2/videostab/frame_source.hpp \
    include/opencv2/videostab/global_motion.hpp \
    include/opencv2/videostab/inpainting.hpp \
    include/opencv2/videostab/log.hpp \
    include/opencv2/videostab/motion_core.hpp \
    include/opencv2/videostab/motion_stabilizing.hpp \
    include/opencv2/videostab/optical_flow.hpp \
    include/opencv2/videostab/outlier_rejection.hpp \
    include/opencv2/videostab/ring_buffer.hpp \
    include/opencv2/videostab/stabilizer.hpp \
    include/opencv2/videostab/wobble_suppression.hpp \
    include/opencv2/calib3d.hpp \
    include/opencv2/core.hpp \
    include/opencv2/cvconfig.h \
    include/opencv2/features2d.hpp \
    include/opencv2/flann.hpp \
    include/opencv2/highgui.hpp \
    include/opencv2/imgcodecs.hpp \
    include/opencv2/imgproc.hpp \
    include/opencv2/ml.hpp \
    include/opencv2/objdetect.hpp \
    include/opencv2/opencv.hpp \
    include/opencv2/opencv_modules.hpp \
    include/opencv2/photo.hpp \
    include/opencv2/shape.hpp \
    include/opencv2/stitching.hpp \
    include/opencv2/superres.hpp \
    include/opencv2/video.hpp \
    include/opencv2/videoio.hpp \
    include/opencv2/videostab.hpp \
    uploadthread.h \
    dbmanager.h

FORMS    += maindialog.ui \
    inputkeydialog.ui \
    alcoholadjustdialog.ui

CONFIG += mobility
MOBILITY = 

OTHER_FILES += \
    android/AndroidManifest.xml \
    android/src/o2oAttendance/QtFullscreenActivity.java \
    android/src/com/zkteco/zkfinger/FingerprintService.java

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

CONFIG += resources_big

RESOURCES += \
    o2oResource.qrc


LIBS += -L$$PWD/libs/armeabi -lzkfinger10

LIBS +=  -L$$PWD/libs/opencv \
    -lIlmImf \
    -llibjasper \
    -llibjpeg \
    -llibpng \
    -llibtiff \
    -llibwebp \
    -lopencv_calib3d \
    -lopencv_core \
    -lopencv_features2d \
    -lopencv_flann \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_imgproc \
    -lopencv_ml \
    -lopencv_objdetect \
    -lopencv_photo \
    -lopencv_shape \
    -lopencv_stitching \
    -lopencv_superres \
    -lopencv_video \
    -lopencv_videoio \
    -lopencv_videostab \
    -ltbb \
    -ltegra_hal


INCLUDEPATH += $$PWD/include $$PWD/json

DEPENDPATH += $$PWD/libs



unix:!win32: LIBS += -L$$PWD/libs/armeabi/ -lzkfinger10 -lusb1.0 -lopencv_java3

DEPENDPATH += $$PWD/libs/armeabi


contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        D:/workplace/QT/o2oAttendance/libs/armeabi/libzkfinger10.so \
        D:/workplace/QT/o2oAttendance/libs/armeabi/libusb1.0.so \
        $$PWD/libs/opencv/libopencv_java3.so
}

DISTFILES += \
    android/libs/zkandroidcore.jar \
    android/libs/zkandroidfpsensor.jar \
    android/libs/sqlite-jdbc-3.16.1.jar \
    android/src/o2oAttendance/NativeFunctions.java \
    android/src/o2oAttendance/AndroidTTS.java \
    android/assets/msyh.ttf \
    libs/armeabi/libusb1.0.so \
    libs/armeabi/libzkfinger10.so





