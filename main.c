#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <time.h>
#include <mem.h>

typedef struct CAMERA {
    int start;
    int end;
} camera_t;

void inputData(int *n, int *k, camera_t** cameraList);
bool checkSurveillance(camera_t *camera, int n, int current_wall_num);

int compare (const void *a, const void *b) {
    return  ((camera_t*)a)->start - ((camera_t*)b)->start;
}

void surveillance(int n, int k, camera_t *camera_list, camera_t **stack);


int main() {
    int n, k;
    camera_t *camera_list = NULL;
    camera_t **camera_stack = NULL;
    clock_t c1 = clock();

    inputData(&n, &k, &camera_list);
    //init stack
    camera_stack = (camera_t**) malloc(sizeof(camera_t*) * k);
    memset(camera_stack, 0, sizeof(camera_t*) * k);

    surveillance(n, k, camera_list, camera_stack);

    free(camera_list);
    free(camera_stack);

    clock_t c2 = clock();
    printf("%.lf\n", (double)c2 - c1);
    return 0;
}


void surveillance(int n, int k, camera_t *camera_list, camera_t **stack) {
    for(int i = 1; i <= n; i++){
        camera_t* max_camera = NULL;
        for(int j = 0; j < k; j++){
            if( checkSurveillance(camera_list + j, n, i) ){

            }
        }
    }
}

bool checkSurveillance(camera_t* camera, int n, int current_wall_num) {
    if (camera->start < camera->end) {
        if (camera->start <= current_wall_num && current_wall_num <= camera->end) {
            return true;
        }
        return false;
    } else {
        if ((camera->start <= current_wall_num && current_wall_num <= n) ||
            (1 <= current_wall_num && current_wall_num <= camera->end)) {
            return true;
        }
        return false;
    }
}

void inputData(int *n, int *k, camera_t **pList) {
    FILE *file = fopen("secret/surveillance-25b-walter.in", "r");
    if(file == NULL){
        return;
    }
    fscanf(file, "%d %d", n, k);
    *pList = (camera_t*) malloc(sizeof(camera_t) * (*k));

    camera_t* cameras = *pList;

    for (int i = 0; i < *k; i++) {
        fscanf(file, "%d %d",  &(cameras[i].start), &(cameras[i].end));
    }

    qsort(*pList, (size_t)*k, sizeof(camera_t), compare);

}
