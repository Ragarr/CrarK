# include <stdio.h>
# include <archive.h>
# include <archive_entry.h>
# include <sys/stat.h>
# include <stdlib.h>
# include <fcntl.h>
# include <string.h>
# include <pthread.h>
# include <math.h>
# include "password_utils.h"

// PROTOTYPES
void *brute_force_0(void *args);
void *brute_force_1(void *args);
int store_password(char* password);
void print_progress(int max_iterations, int threads, int flag);
pthread_t *start_threads(int threads, int max_iterations, char* archive_path, int flag);

// STRUCTS
typedef struct brute_force_args{
    char *starting_pass;
    char *ending_pass;
    int max_iterations;
    char *archive_path;
    int thread_id;
} brute_force_args;

// GLOBALS
pthread_mutex_t archive_mutex = PTHREAD_MUTEX_INITIALIZER;
double global_progress = 0;
int *local_progress;
char **last_pass_tried;


void debug(){
    // char* password;
    char* password2;
    for (int i=0; i< pow(26,3); i++){
        password2=get_password_from_iteration_1(i, 3);
        printf("pass%d: %s\n",i, password2);
    }
    
    exit(0);
}

int main(int argc, char* argv[]){
    if (argc != 7){
        printf("Usage: %s <archive_path> <max_len> <threads> <print_flag> <alg_flag> <debug_flag>\n", argv[0]);
        return 1;
    }
    /*
    print_flag:
    0 - global progress
    1 - global progress and local progress with last password tried
    alg_flag:
    0 - brute force
    1 - brute force, first letters then numbers then symbols 
    */
    int print_flag = atoi(argv[4]);
    int alg_flag = atoi(argv[5]);
    int debug_flag = atoi(argv[6]);

    char *archive_path = argv[1];
    int max_len = atoi(argv[2]);
    int threads = atoi(argv[3]);

    if (debug_flag==1){
        debug();
    }

    // create tmp folder
    mkdir("tmp", 0777);

    last_pass_tried = (char**) malloc(sizeof(char*) * threads);
    for (int i = 0; i < threads; i++){
        last_pass_tried[i] = NULL;
    }
    // max iteration is 93 chars in max_len positions with repetitions and order matters
    long long int max_iterations = (long long int) pow(93, max_len);
    local_progress = (int*) malloc(sizeof(int) * threads);
    for (int i = 0; i < threads; i++){
        local_progress[i] = 0;
    }
    
    pthread_t *thread_ids = start_threads(threads, max_iterations, archive_path, alg_flag);
    
    if (print_flag >1){
        printf("Invalid print_flag\n");
        return 1;
    }
    print_progress(max_iterations, threads, print_flag);

    for (int i = 0; i < threads; i++){
        pthread_join(thread_ids[i], NULL);
    }

    printf("No password found\n");
    return 0;
}
// start threads and return their ids
pthread_t *start_threads(int threads, int max_iterations, char* archive_path, int flag){
    
    pthread_t *thread_ids = (pthread_t*) malloc(sizeof(pthread_t) * threads);
    brute_force_args *args = (brute_force_args*) malloc(sizeof(brute_force_args) * threads);
    switch (flag)
    {
    case 0:
        for (int i = 0; i < threads; i++){
            args[i].starting_pass = get_password_from_iteration_0(i * (max_iterations / threads));
            args[i].ending_pass = get_password_from_iteration_0((i + 1) * (max_iterations / threads));
            args[i].max_iterations = (max_iterations / threads);
            args[i].archive_path = archive_path;
            args[i].thread_id = i;
            pthread_create(&thread_ids[i], NULL, brute_force_0, &args[i]);
        }
        break;
    case 1:
        for (int i = 0; i < threads; i++){
            args[i].starting_pass = get_password_from_iteration_1(i * (max_iterations / threads), 4);
            args[i].ending_pass = get_password_from_iteration_1((i + 1) * (max_iterations / threads), 4);
            args[i].max_iterations = (max_iterations / threads);
            args[i].archive_path = archive_path;
            args[i].thread_id = i;
            pthread_create(&thread_ids[i], NULL, brute_force_1, &args[i]);
        }
        break;
    default:
        break;
    }
    return thread_ids;
}

void* brute_force_1(void *args){
    brute_force_args *args_struct = (brute_force_args*) args;
    char *password = args_struct->starting_pass;
    int max_iterations = args_struct->max_iterations;
    char * archive_path = args_struct->archive_path;
    int r;
    for (int i=0; i<max_iterations; i++){
        password = generate_next_password_1(password);
        pthread_mutex_lock(&archive_mutex);
        r=verify_zip_pass(archive_path, "tmp", password);
        pthread_mutex_unlock(&archive_mutex);
        local_progress[args_struct->thread_id] = i;
        last_pass_tried[args_struct->thread_id] = password;
        if (r == 1){
            printf("Thread %d found password: %s\n", args_struct->thread_id, password);
            store_password(password);
            exit(0);
        }
    }
    // printf("Thread %d finished, last password tryed:%s\n", args_struct->thread_id, password);
    pthread_exit(NULL);
    
}

void print_progress(int max_iterations, int threads, int flag){
// print starting and ending passwords of each thread
    int iters_per_thread = max_iterations / threads;
    while (global_progress < 100 )
    {
        system("clear");
        global_progress = 0;
        for (int i = 0; i < threads; i++){
            global_progress += local_progress[i];
        }
        global_progress = global_progress / (double) max_iterations * 100;
        printf("Global progress: %.2f%%\n", global_progress);
        if (flag==1) {
            for (int i = 0; i < threads; i++){
                printf("Thread %d progress: %.2f%%\n", i, (double) local_progress[i] / (double) (iters_per_thread) * 100);
                printf("Last password tried: %s\n", last_pass_tried[i]);
            }
        }
        sleep(1);
    }
}

void *brute_force_0(void *args){
    brute_force_args *args_struct = (brute_force_args*) args;
    char *password = args_struct->starting_pass;
    int max_iterations = args_struct->max_iterations;
    char * archive_path = args_struct->archive_path;
    int r;
    for (int i=0; i<max_iterations; i++){
        password = generate_next_password_0(password);
        pthread_mutex_lock(&archive_mutex);
        r=verify_zip_pass(archive_path, "tmp", password);
        pthread_mutex_unlock(&archive_mutex);
        local_progress[args_struct->thread_id] = i;
        last_pass_tried[args_struct->thread_id] = password;
        if (r == 1){
            printf("Thread %d found password: %s\n", args_struct->thread_id, password);
            store_password(password);
            exit(0);
        }
    }
    // printf("Thread %d finished, last password tryed:%s\n", args_struct->thread_id, password);
    pthread_exit(NULL);
}

int store_password(char* password){
    // create file output
    int fd;
    fd = open("password.txt", O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd < 0){
        printf("Error creating file\n");
        return 1;
    }
    write(fd, password, strlen(password));
    close(fd);
    // remove tmp folder with all content
    // system("rm -rf -f tmp"); // cannot remove dir
    return 0;
}