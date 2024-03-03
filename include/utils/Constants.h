/**
 * @file Constants.h
 * @author Boris Burkalo (xburka00)
 * @brief 
 * @date 2024-03-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#define FRAMEBUFFER_WIDTH  (1920.f * 4.f)
#define FRAMEBUFFER_HEIGHT (1016.f * 4.f)

#define WINDOW_WIDTH (1920.f  / 2.f)
#define WINDOW_HEIGHT (1016.f / 2.f)

#define NOVEL_VIEW_WIDTH  (1920.f / 2.f)
#define NOVEL_VIEW_HEIGHT (1016.f / 2.f)

#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_VIEWS 64
#define MAX_SBOS 1024
#define MAX_BINDLESS_RESOURCES 16536
#define MAX_RESOLUTION_LINEAR (NOVEL_VIEW_WIDTH * NOVEL_VIEW_HEIGHT)

#define RET_ID_NOT_FOUND -1

#define COMPILED_SHADER_LOC "../build/compiled_shaders/"

#define DRAW_LIGHT false

