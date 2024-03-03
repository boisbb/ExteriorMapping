#define INTERPOLATE_PIXELS_X 1
#define INTERPOLATE_PIXELS_Y 1

#define M_PI 3.1415926535897932384626433832795

#define MAX_VIEWS 64
#define MAX_HITS MAX_VIEWS
#define MIN_INTERVAL_VIEWS 4
#define MAX_INTERVALS (MAX_HITS / MIN_INTERVAL_VIEWS)
#define INTS_FOR_ENCODING (MAX_HITS / 32)
#define MAX_ANGLE (M_PI / 2.f)
#define MIN_PIX_SAMPLES 16
#define MAX_PIX_SAMPLES (256 - MIN_PIX_SAMPLES)

// Sample types
#define SAMPLE_COLOR 0x00000001u
#define SAMPLE_DEPTH_NORMAL 0x00000002u
#define SAMPLE_DEPTH_ANGLE 0x00000004u
