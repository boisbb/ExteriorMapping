#pragma once

#define FRAMEBUFFER_WIDTH (1920.f * 4)
#define FRAMEBUFFER_HEIGHT (1016.f * 4)

#define WINDOW_WIDTH (1920.f / 2.f)
#define WINDOW_HEIGHT (1016.f / 2.f)

#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_VIEWS 128
#define MAX_SBOS 1024
#define MAX_BINDLESS_RESOURCES 16536
#define MAX_RESOLUTION_LINEAR (FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT)


#define RET_ID_NOT_FOUND -1