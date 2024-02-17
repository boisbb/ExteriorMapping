#include "constants.glsl"
#include "structs.glsl"

#define CALCULATE_PIX_ID(t, view, proj, res, offset, pixId) \
    vec4 ct = proj * view * vec4(t, 1.f); \
    ct /= ct.w; \
    \
    pixId = (((ct.xy + 1) / 2) * res) + offset;

// id mask helpers
#define GET_MASK_ID(id, result) \
    int outerMask = int(floor(id / 32)); \
    int innermaskId = int(id) - (outerMask * 32); \
    \
    result = vec2(outerMask, innermaskId);

#define MASK_EMPTY(mask, empty) \
    bool ret = true; \
    for (int i = 0; i < INTS_FOR_ENCODING; i++) \
    { \
        ret = ret && (mask[i] == 0); \
    } \
    empty = ret;

#define IS_IN_MASK(id, mask, result) \
    vec2 maskId; \
    GET_MASK_ID(id, maskId); \
    \
    int outer = int(maskId.x); \
    int inner = int(maskId.y); \
    \
    uint idNum = (1 << inner); \
    uint check = mask[outer] & idNum; \
    \
    result = check == idNum;

#define ADD_TO_MASK(mask, id) \
    vec2 maskId; \
    GET_MASK_ID(id, maskId); \
    int outer = int(maskId.x); \
    int inner = int(maskId.y); \
    \
    mask[outer] = mask[outer] | (1 << inner);

#define REMOVE_FROM_MASK(mask, id) \
    vec2 maskId; \
    GET_MASK_ID(id, maskId); \
    int outer = int(maskId.x); \
    int inner = int(maskId.y); \
    \
    mask[outer] = mask[outer] & (~(1 << inner));

#define IS_POINT_IN_FRUSTUM(t, intersect, frustumPlanes, viewId, valid) \
    valid = t >= 0; \
    \
    for (int k = 0; k < 6 && valid; k++) \
    { \
        valid = (dot(frustumPlanes[k].xyz, intersect) + frustumPlanes[k].w >= 0.f); \
        valid = k == viewId || valid; \
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

#define FIND_MAX_INTERVAL(maxInterval, frustumHitsIn, frustumHitsOut, intersectCount) \
    maxInterval.count = 0; \
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
            ADD_TO_MASK(cameraIndexMask, hitIn.viewId) \
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
                maxInInterval = currentlyInInterval; \
            } \
            \
            currentlyInInterval--; \
            \
            REMOVE_FROM_MASK(cameraIndexMask, hitOut.viewId); \
            \
            if (frustumHitsIn[inId - 1].viewId == hitOut.viewId) \
            { \
                for (int j = inId - 2; j >= 0; j--) \
                { \
                    bool result = false; \
                    IS_IN_MASK(j, cameraIndexMask, result); \
                    if (result) \
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


#define FIND_INTERSECTS(frustumHitsIn, frustumHitsOut, ubo, cssbo, org, dir) \
    for (int i = 0; i < ubo.viewCnt; i++) \
    { \
        ViewDataEvalCompute currentView = cssbo.objects[i]; \
        \
        float intersects[2]; \
        int foundIntersects = 0; \
        \
        bool valid = false; \
        for (int j = 0; j < 6; j++) \
        { \
            vec4 currentPlane = currentView.frustumPlanes[j]; \
            \
            vec3 frustumNormal = currentPlane.xyz; \
            float frustumDistance = currentPlane.w; \
            \
            if (abs(dot(frustumNormal, dir)) > 1e-6) \
            { \
                float t = -(dot(frustumNormal, org) + frustumDistance) / dot(frustumNormal, dir); \
                vec3 intersect = org + t * dir; \
                \
                intersects[foundIntersects] = t; \
                \
                IS_POINT_IN_FRUSTUM(t, intersect, currentView.frustumPlanes, j, valid); \
                \
                foundIntersects += int(valid && foundIntersects < 2) * 1; \
            } \
        } \
        \
        int idIn = int(intersects[0] >= intersects[1]); \
        int idOut = int(intersects[0] < intersects[1]); \
        \
        frustumHitsIn[intersectCount].viewId = i; \
        frustumHitsOut[intersectCount].viewId = i; \
        frustumHitsIn[intersectCount].t = intersects[idIn]; \
        frustumHitsOut[intersectCount].t = intersects[idOut]; \
        \
        intersectCount += int(foundIntersects == 2); \
    }


#define EVALUATE_AND_SAMPLE_COLOR(maxInterval, avg) \
    float dist = 20; \
    float sampleDist = (maxInterval.t.y - maxInterval.t.x) / RAY_PIX_SAMPLES; \
    float segmentStart = maxInterval.t.x; \
    \
    for (int j = 0; j < RAY_PIX_SAMPLES; j++) \
    { \
        vec3 p = org + dir * (segmentStart + j * sampleDist); \
        \
        vec4 localMin = vec4(2); \
        vec4 localMax = vec4(-1); \
        vec4 localAvg = vec4(0); \
        \
        int numOfViews = 0; \
        \
        for (int k = 0; k < ubo.viewCnt; k++) \
        { \
            bool result = false; \
            IS_IN_MASK(k, maxInterval.idBits, result); \
            if (result) \
            { \
                ViewDataEvalCompute currentView = cssbo.objects[k]; \
                \
                vec2 pixId; \
                CALCULATE_PIX_ID(p, currentView.view,  currentView.proj, \
                    currentView.resOffset.xy, currentView.resOffset.zw, \
                    pixId); \
                \
                vec2 pixIdNorm = pixId / ubo.viewsTotalRes; \
                \
                vec4 pixVal = texture(viewImagesSampler, pixIdNorm); \
                \
                localMin = min(localMin, pixVal); \
                localMax = max(localMax, pixVal); \
                \
                localAvg += pixVal; \
                \
                numOfViews++; \
            } \
        } \
        \
        localAvg /= float(numOfViews); \
        \
        vec4 localVecDist = localMax - localMin; \
        float localDist = localVecDist.x + localVecDist.y + localVecDist.z + localVecDist.w; \
        \
        if (localDist < dist) \
        { \
            dist = localDist; \
            avg = localAvg; \
        } \
    }

#define WRITE_TO_IMAGE(origPixId, novelImageSampler, color) \
    for (int x = 0; x < INTERPOLATE_PIXELS_X; x++) \
    { \
        for (int y = 0; y < INTERPOLATE_PIXELS_Y; y++) \
        { \
            imageStore(novelImageSampler, ivec2(origPixId + vec2(x, y)), color); \
        } \
    }

// https://stackoverflow.com/questions/4858264/find-the-distance-from-a-3d-point-to-a-line-segment
#define POINT_TO_LINE_DIST(v, a, b, dist) \
    vec3 ab = b - a; \
    vec3 av = v - a; \
    vec3 bv = v - b; \
    \
    if (dot(av, ab) <= 0.0) \
    { \
        dist = length(av); \
    } \
    else if (dot(bv, ab) >= 0.0) \
    { \
        dist = length(bv); \
    } \
    else \
    { \
        dist = length(cross(ab, av)) / length(ab); \
    } 
    