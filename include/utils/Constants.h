/**
 * @file Constants.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * 
 */

#pragma once

#define MAX_FRAMES_IN_FLIGHT 1
#define MAX_VIEWS 64
#define MAX_SBOS 1024
#define MAX_BINDLESS_RESOURCES 16536
#define MAX_RESOLUTION_LINEAR (NOVEL_VIEW_WIDTH * NOVEL_VIEW_HEIGHT)
#define MAX_OFFSCREEN_RESOLUTION_LINEAR (POINT_CLOUD_WIDTH * POINT_CLOUD_HEIGHT)
#define MIN_RAY_SAMPLES 0
#define MAX_RAY_SAMPLES 256
#define MAX_POINT_CLOUD_DIM 1920 * 2

#define VIEW_MATRIX_WIDTH  (1920.f * 4.f)
#define VIEW_MATRIX_HEIGHT (1080.f * 4.f)
#define WINDOW_WIDTH (1920.f  / 2.f)
#define WINDOW_HEIGHT (1080.f / 2.f)
#define NOVEL_VIEW_WIDTH  (1920.f / 2.f) 
#define NOVEL_VIEW_HEIGHT (1080.f / 2.f)
#define POINT_CLOUD_HEIGHT (1080.f * 2.f)
#define POINT_CLOUD_WIDTH  (1920.f / 2.f)

#define MAX_EVAL_FRAMES 3
#define EVAL_SAMPLES_STEP 16
#define MAX_MSE_STEPS 15
#define MSE_MOVE_STEP 3
#define MSE_FW_STEP 0.1f
#define MAX_MSE_SIDESTEPS 17
#define MSE_SIDESTEP_DEVIATION 2.f
#define MSE_SIDESTEP 0.2f
#define MAX_MSE_ROTATES 8
#define MSE_ROTATION_ANGLE 0.2f

#define RET_ID_NOT_FOUND -1

#ifndef COMPILED_SHADER_LOC
#define COMPILED_SHADER_LOC "../build/compiled_shaders/"
#endif

#ifndef CONFIG_FILES_LOC
#define CONFIG_FILES_LOC "../res/configs/"
#endif

#ifndef SCREENSHOT_FILES_LOC
#define SCREENSHOT_FILES_LOC "../screenshots/"
#endif

#ifndef MODELS_FILES_LOC
#define MODELS_FILES_LOC "../res/models/"
#endif

#define DRAW_LIGHT false

