#include "constants.glsl"

struct ViewDataEvalCompute
{
    vec4 frustumPlanes[6];
    mat4 view;
    mat4 proj;
    mat4 invView;
    mat4 invProj;
    vec4 resOffset;
    vec2 nearFar;
};

struct ViewEvalDebugCompute {
    vec4 frustumPlanes[6];
    int numOfIntersections;
    int numOfFoundIntervals;
    vec2 viewRes;
    vec4 pointInWSpace;
};

struct IntervalHit {
    vec2 t;
    uint idBits[INTS_FOR_ENCODING];
    uint count;
};

struct FrustumHit
{
    float t;
    int viewId;
};