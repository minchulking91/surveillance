#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <mem.h>
#include <limits.h>

typedef struct _CAMERA {
    int start;
    int end;
} CAMERA, *P_CAMERA;

typedef struct _SURVEILLANCE {
    int n;
    int k;
    P_CAMERA *cameras;
} SURVEILLANCE, *P_SURVEILLANCE;

typedef struct _SEED {
    P_CAMERA *cameras;
    int seedSize;
} SEED, *P_SEED;

int inputData(P_SURVEILLANCE surveillance);

void trimNullPointer(P_CAMERA *list, int *n);

void removeInnerCamera(P_CAMERA *list, int n);

bool checkSurveillance(P_CAMERA camera, int n, int current_wall_num);

int compare(const void *a, const void *b) {
    int diff = (*(CAMERA **) a)->start - (*(CAMERA **) b)->start;
    if(diff == 0){
        return (*(CAMERA **) b)->end - (*(CAMERA **) a)->end;
    }
    return diff;
}

int surveillance(SURVEILLANCE *surv, CAMERA *camera);

int findSeedCameras(SURVEILLANCE *surv, P_SEED seed);

int getWallNumber(int wallNumber, int n);

int main() {
    SURVEILLANCE surv;
    surv.cameras = NULL;
    surv.n = 0;
    surv.k = 0;
    inputData(&surv);

    SEED seed;
    seed.cameras = NULL;
    seed.seedSize = 0;

    findSeedCameras(&surv, &seed);

    int minCount = INT_MAX;
    for (int i = 0; i < seed.seedSize; i++) {
        int cnt;
        CAMERA *seedCamera = seed.cameras[i];
        cnt = surveillance(&surv, seedCamera);
        if (minCount > cnt) {
            minCount = cnt;
        }
    }

    if (minCount == INT_MAX) {
        printf("impossible\n");
    } else {
        printf("%d\n", minCount);
    }

    if(seed.cameras != NULL) {
        free(seed.cameras);
        seed.cameras = NULL;
    }
    for(int i = 0; i < surv.k; i++) {
        if (surv.cameras[i] != NULL) {
            free(surv.cameras[i]);
            surv.cameras[i] = NULL;
        }
    }
    if(surv.cameras != NULL) {
        free(surv.cameras);
        surv.cameras = NULL;
    }
    return 0;
}

int surveillance(P_SURVEILLANCE surv, P_CAMERA seed) {
    int seedIndex = 0;
    int start, end;
    start = getWallNumber(seed->end + 1, surv->n);
    end = getWallNumber(seed->start - 1, surv->n);
    if(checkSurveillance(seed, surv->n, end)){
        return 1;
    }
    for (int i = 0; i < surv->k; i++) {
        if (seed == surv->cameras[i]) {
            seedIndex = i;
            break;
        }
    }
    int running_flag = 1;
    int cnt = 1; //contain init camera
    while (running_flag) {
        int searchIndex;
        searchIndex = -1;
        for (int i = 0; i < surv->k; i++) {
            int index;
            index = (i + seedIndex + 1) % surv->k;
            if (checkSurveillance(surv->cameras[index], surv->n, start)) {
                searchIndex = index;
            } else {
                break;
            }
        }
        if (searchIndex != -1) {
            CAMERA *searchCamera = surv->cameras[searchIndex];
            if (checkSurveillance(searchCamera, surv->n, end)) {
                running_flag = 0;
            }
            start = getWallNumber(searchCamera->end + 1, surv->n);
            seedIndex = searchIndex;
            cnt++;
        } else {
            return INT_MAX;
        }
    }
    return cnt;
}

int getWallNumber(int wallNumber, int n) {
    if(wallNumber < 1){
        return wallNumber + n;
    }else if (wallNumber > n){
        return wallNumber - n;
    }
    return wallNumber;
}

bool checkSurveillance(P_CAMERA camera, int n, int current_wall_num) {
    if (camera->start <= camera->end) {
        return (camera->start <= current_wall_num && current_wall_num <= camera->end) ? true : false;
    } else {
        return (camera->start <= current_wall_num && current_wall_num <= n) ||
               (1 <= current_wall_num && current_wall_num <= camera->end) ? true : false;
    }
}

int inputData(P_SURVEILLANCE s) {

    int n, k, max_end = 0, min_start = INT_MAX;
    P_CAMERA *group_a = NULL, *group_b = NULL;
    int size_a = 0, size_b = 0;

    scanf("%d %d", &n, &k);

    group_a = malloc(sizeof(P_CAMERA) * k);
    memset(group_a, 0, sizeof(P_CAMERA) * k);
    group_b = malloc(sizeof(P_CAMERA) * k);
    memset(group_b, 0, sizeof(P_CAMERA) * k);

    for (int i = 0; i < k; i++) {
        P_CAMERA camera;
        camera = malloc(sizeof(CAMERA));
        scanf("%d %d", &(camera->start), &(camera->end));
        if (camera->start <= camera->end) {
            group_a[size_a] = camera;
            size_a++;
        } else {
            max_end = max_end > camera->end ? max_end : camera->end;
            min_start = min_start > camera->start ? camera->start : min_start;
            group_b[size_b] = camera;
            size_b++;
        }
    }

    if (size_b > 0) {
        for (int i = 0; i < size_a; i++) {
            CAMERA *camera = *(group_a + i);
            if (camera->start > min_start) {
                free(camera);
                *(group_a + i) = NULL;
            } else if (camera->end < max_end) {
                free(camera);
                *(group_a + i) = NULL;
            }
        }
        trimNullPointer(group_a, &size_a);
    }

    if (size_a > 0) {
        qsort(group_a, (size_t) size_a, sizeof(CAMERA *), compare);
        removeInnerCamera(group_a, size_a);
        trimNullPointer(group_a, &size_a);
    }

    if (size_b > 0) {
        qsort(group_b, (size_t) size_b, sizeof(CAMERA *), compare);
        removeInnerCamera(group_b, size_b);
        trimNullPointer(group_b, &size_b);
    }

    //merge
    for (int i = 0; i < size_b; i++) {
        group_a[size_a + i] = group_b[i];
    }
    free(group_b);
    size_a += size_b;

    s->n = n;
    s->k = size_a;
    s->cameras = malloc(sizeof(CAMERA *) * size_a);
    for (int i = 0; i < size_a; i++) {
        s->cameras[i] = group_a[i];
    }
    free(group_a);

    return 0;
}

int findSeedCameras(P_SURVEILLANCE surv, P_SEED seed) {
    int min = INT_MAX;
    int minWallNumber = 0;

    int *camera_count = NULL;
    camera_count = malloc(sizeof(int) * (surv->n + 1));
    memset(camera_count, 0, sizeof(int) * (surv->n + 1));

    for (int i = 0; i < surv->k; i++) {
        camera_count[getWallNumber(surv->cameras[i]->start, surv->n)]++;
        camera_count[getWallNumber(surv->cameras[i]->end + 1, surv->n)]--;
    }
    for (int i = 1; i <= surv->n; i++) {
        camera_count[i] += camera_count[i - 1];
    }

    for (int i = 1; i <= surv->n; i++) {
        if (min > camera_count[i]) {
            min = camera_count[i];
            minWallNumber = i;
        }
    }

    seed->seedSize = 0;
    for (int i = 0; i < surv->k; i++) {
        if (checkSurveillance(surv->cameras[i], surv->n, minWallNumber)) {
            seed->seedSize++;
        }
    }
    seed->cameras = malloc(sizeof(P_CAMERA) * seed->seedSize);
    int index = 0;
    for (int i = 0; i < surv->k; i++) {
        if (checkSurveillance(surv->cameras[i], surv->n, minWallNumber)) {
            seed->cameras[index] = surv->cameras[i];
            index++;
        }
    }
    if (seed->seedSize == 0) {
        free(seed->cameras);
        seed->cameras = NULL;
    }

    free(camera_count);
    return seed->seedSize;

}

void removeInnerCamera(P_CAMERA *list, int n) {//remove unnecessary camera.
    if (n <= 0) {
        return;
    }
    int temp = list[0]->end;
    for (int i = 1; i < n; i++) {
        if (temp >= list[i]->end) {
            free(list[i]);
            list[i] = NULL;
        }else{
            temp = list[i]->end;
        }
    }
}

void trimNullPointer(P_CAMERA *list, int *n) {
    int i = 0, offset = 0;
    while (i + offset < *n) {
        if (list[i + offset] == NULL) {
            offset++;
        } else {
            list[i] = list[i + offset];
            i++;
        }
    }
    *n = i;
}
