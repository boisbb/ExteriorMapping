#include "constants.glsl"
#include "structs.glsl"

#define WRITE_TO_IMAGE(origPixId, novelImage, color) \
    for (int x = 0; x < INTERPOLATE_PIXELS_X; x++) \
    { \
        for (int y = 0; y < INTERPOLATE_PIXELS_Y; y++) \
        { \
            imageStore(novelImage, ivec2(origPixId + vec2(x, y)), color); \
        } \
    }

#define WRITE_IMAGE_PIXEL_NEIGHBOURS(image, pixel, offset, color) \
    ivec2 start = pixel - offset; \
    for (int x = 0; x < offset * 2 + 1; x++) \
    { \
        for (int y = 0; y < offset * 2 + 1; y++) \
        { \
            ivec2 pixIndex = start + ivec2(x, y); \
            WRITE_TO_IMAGE(pixIndex, image, color); \
        } \
    }

#define CALCULATE_PIX_ID(t, view, proj, res, offset, pixId) \
    vec4 ct = proj * view * vec4(t, 1.f); \
    ct /= ct.w; \
    \
    pixId = (((ct.xy + 1) / 2) * res) + offset;

#define CALCULATE_PIX_ID_D(t, view, proj, res, offset, pixId, d) \
    vec4 ct = proj * view * vec4(t, 1.f); \
    ct /= ct.w; \
    \
    d = ct.xy; \
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
    valid = true; \
    for (int k = 0; k < 6 && valid; k++) \
    { \
        valid = (dot(frustumPlanes[k].xyz, intersect) + frustumPlanes[k].w >= 0.f); \
        valid = k == viewId || valid; \
    }

#define FIND_INTERSECTS(frustumHitsIn, frustumHitsOut, ubo, cssbo, org, dir) \
    for (int i = 0; i < ubo.viewCnt; i++) \
    { \
        ViewDataEvalCompute currentView = cssbo.objects[i]; \
        \
        float intersects[2]; \
        int foundIntersects = 0; \
        \
        bool valid = true; \
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
        intersects[idIn] = int(intersects[idIn] >= 0) * intersects[idIn]; \
        \
        frustumHitsIn[intersectCount].viewId = i; \
        frustumHitsOut[intersectCount].viewId = i; \
        frustumHitsIn[intersectCount].t = intersects[idIn]; \
        frustumHitsOut[intersectCount].t = intersects[idOut]; \
        \
        intersectCount += int(foundIntersects == 2); \
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

#define EVALUATE_AND_SAMPLE_COLOR(org, dir, maxInterval, avg, rayPixSamples, maxViewsUsed) \
    float dist = 20; \
    float sampleDist = (maxInterval.t.y - maxInterval.t.x) / rayPixSamples; \
    float segmentStart = maxInterval.t.x; \
    \
    for (int j = 0; j < rayPixSamples; j++) \
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
                \
                if (numOfViews > maxViewsUsed) \
                { \
                    break; \
                } \
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

#define EVALUATE_AND_SAMPLE_DEPTH_DIST(org, dir, maxInterval, finalColor, samplingType, rayPixSamples, maxViewsUsed) \
    float sampleDist = (maxInterval.t.y - maxInterval.t.x) / rayPixSamples; \
    float segmentStart = maxInterval.t.x; \
    \
    vec3 startP = org + dir * maxInterval.t.x; \
    vec3 endP = org + dir * maxInterval.t.y; \
    \
    float minDist = 1.0 / 0.0; \
    for (int j = 0; j < rayPixSamples; j++) \
    { \
        vec3 p = org + dir * (segmentStart + j * sampleDist); \
        \
        vec4 colorAcc = vec4(0.0); \
        float pointDistAcc = 0; \
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
                vec2 pixId = vec2(0.0); \
                vec2 dView = vec2(0.0); \
                CALCULATE_PIX_ID_D(p, currentView.view,  currentView.proj, \
                    currentView.resOffset.xy, currentView.resOffset.zw, \
                    pixId, dView); \
                \
                vec2 uvView = pixId / ubo.viewsTotalRes; \
                \
                float n = currentView.nearFar.x; \
                float f = currentView.nearFar.y; \
                float z = texture(viewImagesDepthSampler, uvView).r; \
                \
                vec4 clipPoint = vec4(dView, z, 1.0); \
                vec4 viewPoint = currentView.invProj * clipPoint; \
                viewPoint /= viewPoint.w; \
                \
                vec3 worldPoint = (currentView.invView * viewPoint).xyz; \
                \
                float pointDistance = 1.0 / 0.0; \
                if (samplingType == SAMPLE_DEPTH_NORMAL) \
                { \
                    POINT_TO_LINE_DIST(worldPoint, startP, endP, pointDistance); \
                } \
                else if (samplingType == SAMPLE_DEPTH_ANGLE) \
                { \
                    LINE_TO_LINE_ANGLE_DIST(org, worldPoint, startP, endP, pointDistance); \
                } \
                \
                pointDistAcc += pointDistance; \
                colorAcc += texture(viewImagesSampler, uvView); \
                \
                numOfViews++; \
                if (numOfViews > maxViewsUsed) \
                { \
                    break; \
                } \
            } \
        } \
        \
        if (pointDistAcc < minDist) \
        { \
            minDist = pointDistAcc; \
            finalColor = colorAcc / numOfViews; \
        } \
    }

#define EVALUATE_AND_SAMPLE_DEPTH_DIST_TEST_PIXEL(org, dir, maxInterval, finalColor, samplingType, testPixelImage, rayPixSamples, maxViewsUsed) \
    float sampleDist = (maxInterval.t.y - maxInterval.t.x) / rayPixSamples; \
    float segmentStart = maxInterval.t.x; \
    \
    vec3 startP = org + dir * maxInterval.t.x; \
    vec3 endP = org + dir * maxInterval.t.y; \
    \
    float minDist = 1.0 / 0.0; \
    \
    int intervalViewCnt = 0; \
    ivec2 sampledPixels[MAX_VIEWS]; \
    ivec2 startPixels[MAX_VIEWS]; \
    ivec2 endPixels[MAX_VIEWS]; \
    for (int j = 0; j < rayPixSamples; j++) \
    { \
        vec3 p = org + dir * (segmentStart + j * sampleDist); \
        \
        vec4 colorAcc = vec4(0.0); \
        float pointDistAcc = 0; \
        \
        intervalViewCnt = 0; \
        ivec2 localSampledPixels[MAX_VIEWS]; \
        for (int k = 0; k < ubo.viewCnt; k++) \
        { \
            bool result = false; \
            IS_IN_MASK(k, maxInterval.idBits, result); \
            if (result) \
            { \
                ViewDataEvalCompute currentView = cssbo.objects[k]; \
                \
                vec2 pixId = vec2(0.0); \
                vec2 dView = vec2(0.0); \
                CALCULATE_PIX_ID_D(p, currentView.view,  currentView.proj, \
                    currentView.resOffset.xy, currentView.resOffset.zw, \
                    pixId, dView); \
                \
                vec2 uvView = pixId / ubo.viewsTotalRes; \
                localSampledPixels[intervalViewCnt] = ivec2(pixId); \
                \
                if (j == 0) \
                { \
                    startPixels[intervalViewCnt] = ivec2(pixId); \
                } \
                else if (j == rayPixSamples - 1) \
                { \
                    endPixels[intervalViewCnt] = ivec2(pixId); \
                } \
                \
                float n = currentView.nearFar.x; \
                float f = currentView.nearFar.y; \
                float z = texture(viewImagesDepthSampler, uvView).r; \
                \
                vec4 clipPoint = vec4(dView, z, 1.0); \
                vec4 viewPoint = currentView.invProj * clipPoint; \
                viewPoint /= viewPoint.w; \
                \
                vec3 worldPoint = (currentView.invView * viewPoint).xyz; \
                \
                float pointDistance = 1.0 / 0.0; \
                if (samplingType == SAMPLE_DEPTH_NORMAL) \
                { \
                    POINT_TO_LINE_DIST(worldPoint, startP, endP, pointDistance); \
                } \
                else if (samplingType == SAMPLE_DEPTH_ANGLE) \
                { \
                    LINE_TO_LINE_ANGLE_DIST(org, worldPoint, startP, endP, pointDistance); \
                } \
                \
                pointDistAcc += pointDistance; \
                colorAcc += texture(viewImagesSampler, uvView); \
                intervalViewCnt++; \
                WRITE_IMAGE_PIXEL_NEIGHBOURS(testPixelImage, ivec2(pixId), 15, vec4(0, 0, 1, 1)); \
                if (intervalViewCnt > maxViewsUsed) \
                { \
                    break; \
                } \
            } \
        } \
        \
        if (pointDistAcc < minDist) \
        { \
            minDist = pointDistAcc; \
            finalColor = colorAcc / intervalViewCnt; \
            sampledPixels = localSampledPixels; \
        } \
    } \
    \
    for (int l = 0; l < intervalViewCnt; l++) \
    { \
        ivec2 pixel = sampledPixels[l]; \
        int offset = 20; \
        ivec2 start = pixel - offset; \
        ivec2 startStart = startPixels[l] - offset; \
        ivec2 endStart = endPixels[l] - offset; \
        for (int x = 0; x < offset * 2 + 1; x++) \
        { \
            for (int y = 0; y < offset * 2 + 1; y++) \
            { \
                ivec2 pixIndex = start + ivec2(x, y); \
                WRITE_IMAGE_PIXEL_NEIGHBOURS(testPixelImage, pixel, offset, vec4(1.f, 1.f, 0.f, 1.f)); \
            } \
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

// https://stackoverflow.com/questions/43749543/c-calculate-angle-0-to-360-between-two-3d-vectors
#define LINE_TO_LINE_ANGLE_DIST(c, d, a, b, dist) \
    vec3 ab = b - a; \
    vec3 cd = d - c; \
    \
    dist = acos(dot(ab, cd) / (length(ab) * length(cd)));

#define VEC_TO_VEC_ANGLE_DIST(a, b, dist) \
    dist = acos(dot(a, b) / (length(a) * length(b)));

#define CHOOSE_SAMPLE_COUNT(ubo, cssbo, org, dir, maxInterval, sampleCount) \
    vec3 rayStart = org + dir * maxInterval.t.x; \
    vec3 rayEnd = org + dir * maxInterval.t.y; \
    vec3 ray = rayEnd - rayStart; \
    float maxDist = 0.f; \
    \
    for (int i = 0; i < ubo.viewCnt; i++) \
    { \
        bool result = false; \
        IS_IN_MASK(i, maxInterval.idBits, result); \
        if (result) \
        { \
            vec3 viewDir = cssbo.objects[i].viewDir.xyz; \
            float localDist = 0.f; \
            VEC_TO_VEC_ANGLE_DIST(viewDir, ray, localDist); \
            \
            if (localDist > maxDist) \
            { \
                maxDist = localDist; \
            } \
        } \
    } \
    \
    float ratio = maxDist / MAX_ANGLE; \
    ratio = (ratio > 1) ? 1.f : ratio; \
    sampleCount = MIN_PIX_SAMPLES + int(ratio * MAX_PIX_SAMPLES);
    