#include "constants.glsl"
#include "structs.glsl"

#define CALCULATE_PIX_ID(t, view, proj, res, offset, pixId) \
    vec4 ct = proj * view * vec4(t, 1.f); \
    ct /= ct.w; \
    \
    pixId = (((ct.xy + 1) / 2) * res) + offset;

// vec2 calculatePixId(vec3 t, mat4 view, mat4 proj, vec2 res, vec2 offset)
// {
//     vec4 ct = proj * view * vec4(t, 1.f);
//     ct /= ct.w;
// 
//     return (((ct.xy + 1) / 2) * res) + offset;
// }

// id mask helpers
#define GET_MASK_ID(id, result) \
    int maskId = int(floor(id / 32)); \
    int innermaskId = int(id) - (maskId * 32); \
    \
    result = vec2(maskId, innermaskId);

vec2 calculateMaskId(float id)
{
    int maskId = int(floor(id / 32));
    int innermaskId = int(id) - (maskId * 32);

    return vec2(maskId, innermaskId);
}

#define MASK_EMPTY(mask, empty) \
    bool ret = true; \
    for (int i = 0; i < INTS_FOR_ENCODING; i++) \
    { \
        ret = ret && (mask[i] == 0); \
    } \
    empty = ret;

bool idMaskEmpty(uint mask[INTS_FOR_ENCODING])
{
    bool ret = true;
    for (int i = 0; i < INTS_FOR_ENCODING; i++)
    {
        ret = ret && (mask[i] == 0);
    }

    return ret;
}

#define IS_IN_MASK(id, mask, result) \
    vec2 maskId = calculateMaskId(id); \
    int outer = int(maskId.x); \
    int inner = int(maskId.y); \
    \
    uint idNum = (1 << inner); \
    uint check = mask[outer] & idNum; \
    \
    result = check == idNum;

bool isInMask(int id, uint mask[INTS_FOR_ENCODING])
{
    vec2 maskId = calculateMaskId(id);
    int outer = int(maskId.x);
    int inner = int(maskId.y);

    uint idNum = (1 << inner);
    uint check = mask[outer] & idNum;

    return check == idNum;
}

#define ADD_TO_MASK(mask, id) \
    vec2 maskId = calculateMaskId(id); \
    int outer = int(maskId.x); \
    int inner = int(maskId.y); \
    \
    mask[outer] = mask[outer] | (1 << inner);

void addToMask(inout uint mask[INTS_FOR_ENCODING], int id)
{
    vec2 maskId = calculateMaskId(id);
    int outer = int(maskId.x);
    int inner = int(maskId.y);

    mask[outer] = mask[outer] | (1 << inner);
}

#define REMOVE_FROM_MASK(mask, id) \
    vec2 maskId = calculateMaskId(id); \
    int outer = int(maskId.x); \
    int inner = int(maskId.y); \
    \
    mask[outer] = mask[outer] & (~(1 << inner));

void removeFromMask(inout uint mask[INTS_FOR_ENCODING], int id)
{
    vec2 maskId = calculateMaskId(id);
    int outer = int(maskId.x);
    int inner = int(maskId.y);

    mask[outer] = mask[outer] & (~(1 << inner));
}

#define IS_POINT_IN_FRUSTUM(t, intersect, frustumPlanes, viewId, valid) \
    valid = t >= 0; \
    \
    for (int k = 0; k < 6 && valid; k++) \
    { \
        valid = (dot(frustumPlanes[k].xyz, intersect) + frustumPlanes[k].w >= 0.f); \
        valid = k == viewId || valid; \
    }

bool isPointInFrustum(float t, vec3 intersect, vec4 frustumPlanes[6], int viewId)
{
    bool valid = t >= 0;

    for (int k = 0; k < 6 && valid; k++)
    {
        valid = (dot(frustumPlanes[k].xyz, intersect) + frustumPlanes[k].w >= 0.f);
        valid = k == viewId || valid;
    }

    return valid;
}

#define INSERT_SORT(hits, intersectCount) \
    for (int i = 1; i < intersectCount; i++) \
    { \
        FrustumHit key = hits[i]; \
        \
        int j = i - 1; \
        while (j >= 0 && hits[j].t > key.t) \
        { \
            hits[j + 1] = hits[j]; \
            j = j - 1; \
        } \
        \
        hits[j + 1] = key; \
    }

// Others
void insertSort(inout FrustumHit hits[MAX_HITS], int intersectCount)
{
    for (int i = 1; i < intersectCount; i++)
    {
        // hits out
        FrustumHit key = hits[i];

        int j = i - 1;
        while (j >= 0 && hits[j].t > key.t)
        {
            hits[j + 1] = hits[j];
            j = j - 1;
        }

        hits[j + 1] = key;
    }
}

#define FIND_MAX_INTERVAL(maxInterval, frustumHitsIn, frustumHitsOut, intersectCount) \
    maxInterval.count = 0; \
    bool maxFound = false; \
    int maxInInterval = 0; \
    int currentlyInInterval = 0; \
    uint cameraIndexMask[INTS_FOR_ENCODING]; \
    for (int i = 0; i < INTS_FOR_ENCODING; i++) \
    { \
        cameraIndexMask[i] = 0; \
    } \
    float currentStartT = -1; \
    \
    FrustumHit hitIn; \
    FrustumHit hitOut; \
    int inId = 0; \
    int outId = 0; \
    for (int i = 0; i < intersectCount * 2; i++) \
    { \
        hitIn = frustumHitsIn[min(inId, (intersectCount - 1))]; \
        hitOut = frustumHitsOut[outId]; \
        \
        if (hitIn.t <= hitOut.t && inId < intersectCount) \
        { \
            currentlyInInterval++; \
            \
            addToMask(cameraIndexMask, hitIn.viewId); \
            \
            currentStartT = hitIn.t; \
            \
            inId++; \
        } \
        else if (hitIn.t > hitOut.t || inId >= intersectCount) \
        { \
            if (currentlyInInterval >= MIN_INTERVAL_VIEWS && \
                currentlyInInterval > maxInInterval) \
            { \
                maxInterval.t = vec2(currentStartT, hitOut.t); \
                maxInterval.idBits = cameraIndexMask; \
                maxInterval.count = currentlyInInterval; \
                maxFound = true; \
                maxInInterval = currentlyInInterval; \
            } \
            \
            currentlyInInterval--; \
            \
            removeFromMask(cameraIndexMask, hitOut.viewId); \
            \
            if (frustumHitsIn[inId - 1].viewId == hitOut.viewId) \
            { \
                for (int j = inId - 2; j >= 0; j--) \
                { \
                    if (isInMask(j, cameraIndexMask)) \
                    { \
                        currentStartT = frustumHitsIn[j].t; \
                        break; \
                    } \
                } \
            } \
            \
            outId++; \
        } \
    }