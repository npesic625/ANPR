################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../IMG.cpp \
../ImageProcessor.cpp \
../Logger.cpp \
../TextDetection.cpp \
../main.cpp 

OBJS += \
./IMG.o \
./ImageProcessor.o \
./Logger.o \
./TextDetection.o \
./main.o 

CPP_DEPS += \
./IMG.d \
./ImageProcessor.d \
./Logger.d \
./TextDetection.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/tesseract -I/usr/local/include/GraphicsMagick -I/usr/local/include/opencv -I/media/sf_Development/opencv-3.1.0/modules/highgui/include/opencv2 -I/media/sf_Development/opencv-3.1.0/modules/imgcodecs/include/opencv2 -I/media/sf_Development/opencv-3.1.0/modules/imgproc/include/opencv2 -I/media/sf_Development/opencv-3.1.0/modules/core/include/opencv2 -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -fpermissive -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


