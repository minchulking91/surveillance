#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <mem.h>
#include <limits.h>

typedef struct CAMERA {
    int start;
    int end;
} camera_t;

typedef struct SURVEILLANCE {
    int n;
    int k;
    camera_t **cameras;
} surveillance_t;

typedef struct SEED {
    camera_t **cameras;
    int seedSize;
} seed_t;

int inputData(surveillance_t *surveillance);

void trimNullPointer(camera_t **list, int *n);

void removeInnerCamera(camera_t **list, int n);

bool checkSurveillance(camera_t *camera, int n, int current_wall_num);

int compare(const void *a, const void *b) {
    return (*(camera_t **) a)->start - (*(camera_t **) b)->start;
}

int surveillance(surveillance_t *surv, camera_t *camera);

int findSeedCameras(surveillance_t *surv, seed_t *seed);

int getWallNumber(int wallNumber, int n);

int main() {
    surveillance_t surv;
    memset(&surv, 0, sizeof(surveillance_t));

    inputData(&surv);

    seed_t seed;
    memset(&seed, 0, sizeof(seed_t));
    findSeedCameras(&surv, &seed);

    int minCount = INT_MAX;
    for (int i = 0; i < seed.seedSize; i++) {
        int cnt;
        camera_t *seedCamera = seed.cameras[i];
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

    free(seed.cameras);
    for(int i = 0; i < surv.k; i++){
        free(surv.cameras[i]);
    }
    free(surv.cameras);
    return 0;
}

int surveillance(surveillance_t *surv, camera_t *seed) {
    int seedIndex = 0;
    int start, end;
    start = getWallNumber(seed->end + 1, surv->n);
    end = getWallNumber(seed->start - 1, surv->n);
    for (int i = 0; i < surv->k; i++) {
        if (seed == surv->cameras[i]) {
            seedIndex = i;
            break;
        }
    }
    int running_flag = 1;
    int cnt = 1; //contain cameras camera
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
            camera_t *searchCamera = surv->cameras[searchIndex];
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

bool checkSurveillance(camera_t *camera, int n, int current_wall_num) {
    if (camera->start <= camera->end) {
        return (camera->start <= current_wall_num && current_wall_num <= camera->end) ? true : false;
    } else {
        return (camera->start <= current_wall_num && current_wall_num <= n) ||
               (1 <= current_wall_num && current_wall_num <= camera->end) ? true : false;
    }
}

int inputData(surveillance_t *s) {

    int n, k, max_end = 0, min_start = INT_MAX;
    camera_t **group_a = NULL, **group_b = NULL;
    int size_a = 0, size_b = 0;
    //surveillance-23-start-with-wrap-3
    //surveillance-25b-walter

    scanf("%d %d", &n, &k);

    group_a = malloc(sizeof(camera_t *) * k);
    memset(group_a, 0, sizeof(camera_t *) * k);
    group_b = malloc(sizeof(camera_t *) * k);
    memset(group_b, 0, sizeof(camera_t *) * k);

    for (int i = 0; i < k; i++) {
        camera_t *camera;
        camera = malloc(sizeof(camera_t));
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
            camera_t *camera = *(group_a + i);
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
        qsort(group_a, (size_t) size_a, sizeof(camera_t *), compare);
        removeInnerCamera(group_a, size_a);
        trimNullPointer(group_a, &size_a);
    }

    if (size_b > 0) {
        qsort(group_b, (size_t) size_b, sizeof(camera_t *), compare);
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
    s->cameras = malloc(sizeof(camera_t *) * size_a);
    for (int i = 0; i < size_a; i++) {
        s->cameras[i] = group_a[i];
    }
    free(group_a);

    return 0;
}

int findSeedCameras(surveillance_t *surv, seed_t *seed) {
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
    seed->cameras = malloc(sizeof(camera_t *) * seed->seedSize);
    int index = 0;
    for (int i = 0; i < surv->k; i++) {
        if (checkSurveillance(surv->cameras[i], surv->n, minWallNumber)) {
            seed->cameras[index] = surv->cameras[i];
            index++;
        }
    }
    if (seed->seedSize == 0) {
        free(seed->cameras);
    }

    free(camera_count);
    return seed->seedSize;

}

void removeInnerCamera(camera_t **list, int n) {//remove unnecessary camera.
    if (n <= 0) {
        return;
    }
    for (int i = n - 1; i >= 1; i--) {
        if (list[i]->end <= list[i - 1]->end) {
            free(list[i]);
            list[i] = NULL;
        }
    }
}

void trimNullPointer(camera_t **list, int *n) {
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
