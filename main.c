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
void *brute_force_from_to(void *args);

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

int main(int argc, char* argv[]){
    if (argc != 4){
        printf("Usage: %s <archive_path> <max_len> <threads>\n", argv[0]);
        return 1;
    }
    char *archive_path = argv[1];
    int max_len = atoi(argv[2]);
    int threads = atoi(argv[3]);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);



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
    
    pthread_t *thread_ids = (pthread_t*) malloc(sizeof(pthread_t) * threads);
    brute_force_args *args = (brute_force_args*) malloc(sizeof(brute_force_args) * threads);
    for (int i = 0; i < threads; i++){
        args[i].starting_pass = get_password_from_iteration(i * (max_iterations / threads));
        args[i].ending_pass = get_password_from_iteration((i + 1) * (max_iterations / threads));
        args[i].max_iterations = (max_iterations / threads);
        args[i].archive_path = archive_path;
        args[i].thread_id = i;
        pthread_create(&thread_ids[i], &attr, brute_force_from_to, &args[i]);
    }
 
    // print starting and ending passwords of each thread
    while (global_progress < 100 )
    {
        system("clear");
        global_progress = 0;
        for (int i = 0; i < threads; i++){
            global_progress += local_progress[i];
        }
        global_progress = global_progress / (double) max_iterations * 100;
        printf("Global progress: %.2f%%\n", global_progress);
        for (int i = 0; i < threads; i++){
            printf("Thread %d progress: %.2f%%\n", i, (double) local_progress[i] / (double) (args[i].max_iterations) * 100);
            printf("Last password tried: %s\n", last_pass_tried[i]);
            // store password bla bla
        }
        sleep(2);
    }
    for (int i = 0; i < threads; i++){
        pthread_join(thread_ids[i], NULL);
    }
    printf("No password found\n");
    return 0;
}

void *brute_force_from_to(void *args){
    brute_force_args *args_struct = (brute_force_args*) args;
    char *password = args_struct->starting_pass;
    int max_iterations = args_struct->max_iterations;
    char * archive_path = args_struct->archive_path;
    int r;
    for (int i=0; i<max_iterations; i++){
        password = generate_next_password(password);
        pthread_mutex_lock(&archive_mutex);
        r=verify_zip_pass(archive_path, "tmp", password);
        pthread_mutex_unlock(&archive_mutex);
        local_progress[args_struct->thread_id] = i;
        last_pass_tried[args_struct->thread_id] = password;
        if (r == 1){
            printf("Thread %d found password: %s\n", args_struct->thread_id, password);
            exit(0);
        }
    }
    printf("Thread %d finished, last password tryed:%s\n", args_struct->thread_id, password);
    pthread_exit(NULL);
}

