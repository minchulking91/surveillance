#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <time.h>
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
    camera_t **seed;
    int seedSize;
} surveillance_t;

int inputData(surveillance_t* surveillance);

bool checkSurveillance(camera_t *camera, int n, int current_wall_num);

int compare(const void *a, const void *b) {
    return (*(camera_t **) a)->start - (*(camera_t **) b)->start;
}

int surveillance(surveillance_t *surveillance);

void trimNullPointer(camera_t **list, int *n);

void removeInnerCamera(camera_t **list, int n);

int findSeedCameras(int n, camera_t **list, int k, camera_t ***seed);

static void printTime(char* string){
    static bool init = false;
    static clock_t c1;
    clock_t c2;
    if(!init){
        c1 = clock();
        init = true;
    }else{
        c2 = clock();
        printf("%s %.lf\n", string, (double) c2 - c1);
        c1 = c2;
    }
}

int main() {

    int minCount = INT_MAX;
    surveillance_t s;
    memset(&s, 0, sizeof(surveillance_t));

    printTime("init");
    if (inputData(&s) != 0) {
        printf("cannot open file!\n");
        return -1;
    }
    printTime("after input data");

    printf("%d\n", s.seedSize);
    for(int i = 0; i < s.seedSize; i++){
        printf("camera %d %d\n", s.seed[i]->start, s.seed[i]->end);
    }

    //minCount = surveillance(&s);
    printf("%d\n", minCount);


    return 0;
}

int surveillance(surveillance_t *s) {
    return 0;
}

bool checkSurveillance(camera_t *camera, int n, int current_wall_num) {
    if (camera->start <= camera->end) {
        return (camera->start <= current_wall_num && current_wall_num <= camera->end) ? true : false;
    } else {
        return (camera->start <= current_wall_num && current_wall_num <= n) ||
               (1 <= current_wall_num && current_wall_num <= camera->end) ? true : false;
    }
}

/** 파일로부터 데이터를 입력받아
 * 1. 다른 카메라에 포함되는 카메라 제거
 * 2. seed pillar 에 대한 camera list 구하기.
 *
 * @param s
 * @return 파일 읽기에 실패할 경우 -1
 */
int inputData(surveillance_t* s) {

    int n, k, max_end = 0, min_start = INT_MAX;
    camera_t **group_a = NULL,  **group_b = NULL;
    int size_a = 0, size_b = 0;

    FILE *file = fopen("secret/surveillance-25b-walter.in", "r");
    if (file == NULL) {
        return -1;
    }

    fscanf(file, "%d %d", &n, &k);

    group_a = (camera_t **) malloc(sizeof(camera_t *) * k);
    memset(group_a, 0, sizeof(camera_t*) * k);
    group_b = (camera_t **) malloc(sizeof(camera_t *) * k);
    memset(group_b, 0, sizeof(camera_t*) * k);

    for (int i = 0; i < k; i++) {
        camera_t *camera;
        camera = (camera_t *) malloc(sizeof(camera_t));
        fscanf(file, "%d %d", &(camera->start), &(camera->end));
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

    if(size_b > 0) {
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

    if(size_a > 0) {
        qsort(group_a, (size_t) size_a, sizeof(camera_t *), compare);
        removeInnerCamera(group_a, size_a);
        trimNullPointer(group_a, &size_a);
    }

    if(size_b > 0) {
        qsort(group_b, (size_t) size_b, sizeof(camera_t *), compare);
        removeInnerCamera(group_b, size_b);
        trimNullPointer(group_b, &size_b);
    }

    //merge
    for(int i = 0; i < size_b; i++){
        group_a[size_a + i] = group_b[i];
    }
    free(group_b);
    size_a += size_b;

    printTime("after trim camera");

    s->n = n;
    s->k = size_a;
    s->cameras = (camera_t**) malloc(sizeof(camera_t*) * size_a);
    for(int i = 0; i < size_a; i++){
        s->cameras[i] = group_a[i];
    }
    s->seedSize = findSeedCameras(n, group_a, size_a, &(s->seed));
    free(group_a);
    printTime("after find seed");
    return 0;
}

/**
 * 기둥 I를 감시하는 카메라의 갯수 J를 구해 기둥 1~n 에 대한 최소 J를 구하고 해당 카메라 목록 M을 만든다.
 * J가 0일 경우 감시할 수 없는 기둥이 생기므로 impossible
 * J가 1일 경우 한대의 카메라는 꼭 포함해야 하고, 확정적으로 문제의 해답을 구할 수 있다.
 * J가 2이상일 경우 M에 있는 카메라 중 한대 이상을 꼭 포함해야 하므로 J번의 탐색을 통해 문제의 해답을 구할 수 있다.
 *
 * @param list : 카메라 목록. start 를 기준으로 오름차순 정렬되어 있음.
 *                서로 다른 카메라 i, j에 대해서 i, j 모두 start < end 이거나 start > end 일때 i.start > j.start 면 i.end > j.end 를 만족함.
 * @param k : list의 크기
 * @param seed : 카메라 목록 M
 * @return 카메라의 갯수 J
 */
int findSeedCameras(int n, camera_t **list, int k, camera_t ***seed) {
    int min = INT_MAX;
    int minPillar = 0;
    printTime("start camera counter");
    int* camera_counter = (int*) malloc(sizeof(int) * (n + 1));
    memset(camera_counter, 0, sizeof(int) * n);
    for(int i = 0; i < k; i++){
        if (list[i]->start <= list[i]->end) {
            for(int j = list[i]->start; j <= list[i]->end; j++){
                camera_counter[j] ++;
            }
        }else{
            for(int j = 1; j <= list[i]->end; j++){
                camera_counter[j] ++;
            }
            for(int j = list[i]->start; j <= n; j++){
                camera_counter[j] ++;
            }
        }
    }
    printTime("after camera counter");
    for(int i = 1; i <=n; i++){
        if(min > camera_counter[i]){
            min = camera_counter[i];
            minPillar = i;
        }
    }
    if(min == 0){
        //impossible
        *seed = NULL;
        return 0;
    }else{
        int index = 0;
        *seed = (camera_t**) malloc(sizeof(camera_t*) * min);
        for(int i = 0; i < k; i++) {
            if(checkSurveillance(list[i], n, minPillar)){
                (*seed)[index] = list[i];
                index++;
            }
        }
        return index;
    }

}

void removeInnerCamera(camera_t **list, int n) {//remove unnecessary camera.
    if(n <= 0){
        return;
    }
    for (int i = n - 1; i >= 1; i--) {
        if (list[i]->end <= list[i-1]->end) {
            free(list[i]);
            list[i] = NULL;
        }
    }
}

void trimNullPointer(camera_t **list, int *n) {
    int i = 0, offset = 0;
    while(i + offset < *n){
        if(list[i + offset] == NULL){
            offset++;
        }else{
            list[i] = list[i + offset];
            i++;
        }
    }
    *n = i;
}
