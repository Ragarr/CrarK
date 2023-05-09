# include <stdio.h>
# include <archive.h>
# include <archive_entry.h>
# include <sys/stat.h>
# include <stdlib.h>
#include <fcntl.h>
# include <string.h>
# include <pthread.h>
# include <math.h>

// GLOBALS
pthread_mutex_t archive_mutex = PTHREAD_MUTEX_INITIALIZER;
double global_progress = 0;
int *local_progress;

// TYPEDEFS
typedef struct brute_force_args{
    char *starting_pass;
    int max_iterations;
    char *archive_path;
    int thread_id;
} brute_force_args;

// PROTYPES
int verify_zip_pass(const char* archive_path, const char* dest_path, const char* password);
char* generate_password(char* password);
int get_char_index(char c);
char get_char_from_index(int index);
char *get_password_from_iteration(int iteration);
void *brute_force_from_to(void *args);


int main(int argc, char* argv[]){
    if (argc != 4){
        printf("Usage: %s <archive_path> <max_len> <threads>\n", argv[0]);
        return 1;
    }
    pthread_t threads[atoi(argv[3])];
    int max_iterations = pow(93, atoi(argv[2]));
    int iterations_per_thread = max_iterations / atoi(argv[3]);
    char **threads_starting_passwords;
    threads_starting_passwords = (char**) malloc(sizeof(char*) * atoi(argv[3]));

    local_progress = (int*) malloc(sizeof(int) * atoi(argv[3]));
    for (int i = 0; i < atoi(argv[3]); i++){
        threads_starting_passwords[i] = get_password_from_iteration(iterations_per_thread * i);
    }
    // print starting passwords
    for (int i = 0; i < atoi(argv[3]); i++){
        printf("Thread %d starting password: %s\n", i, threads_starting_passwords[i]);
    }
    brute_force_args *args;
    args = (brute_force_args*) malloc(sizeof(brute_force_args) * atoi(argv[3]));
    for (int i = 0; i < atoi(argv[3]); i++){
        args[i].starting_pass = threads_starting_passwords[i];
        args[i].max_iterations = iterations_per_thread;
        args[i].archive_path = argv[1];
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, brute_force_from_to, (void*) &args[i]);
    }
    while (1)
    {
        system("clear");
        for (int i = 0; i < atoi(argv[3]); i++){
            printf("Thread %d progress: %f\n", i, local_progress[i]/ (double) iterations_per_thread);
            global_progress += local_progress[i]/ (double) iterations_per_thread * 100;
        };
        printf("Global progress: %f\n", global_progress);
        sleep(2);
    }
    

    for (int i = 0; i < atoi(argv[3]); i++){
        pthread_join(threads[i], NULL);
    }
    printf("Contraseña no encontrada\n");
    return 0;
}

void *brute_force_from_to(void *args){
    brute_force_args *args_struct = (brute_force_args*) args;
    char *password = args_struct->starting_pass;
    int max_iterations = args_struct->max_iterations;
    char * archive_path = args_struct->archive_path;
    int i = 0;
    int result;
    printf("Thread %d starting password: '%s'\n", args_struct->thread_id, password);
    while (i<max_iterations){
        password = generate_password(password);
        i++;
        // printf("Thread %d testing: %s\n", args_struct->thread_id, password);
        pthread_mutex_lock(&archive_mutex);
        result = verify_zip_pass(archive_path, "temp", password);
        pthread_mutex_unlock(&archive_mutex);
        local_progress[args_struct->thread_id] += 1;
        if (result == 1){
            printf("Contraseña encontrada: %s", password);
            exit(0);
        }
    }
    pthread_exit(NULL);
}

char *get_password_from_iteration(int n){
    int base = 93;
    int i = 0;
    int n2 = n;
    while (n2 > 0){
        n2 = n2 / base;
        i++;
    }
    char *password = (char*) malloc(sizeof(char) * (i + 1));
    password[i] = '\0';
    i--;
    while (n>0){
        int remainder = n % base;
        password[i] = get_char_from_index(remainder);
        i--;
        n = n / base;
    }
    if (password[0] == '\0'){
        password[0] = get_char_from_index(0);
    }
    return password;
    
}

char* generate_password(char* password){
    if (password == NULL){
        password = (char*) malloc(sizeof(char) * 2);
        password[0] = get_char_from_index(0);
        password[1] = '\0';
        return password;
    }
    int password_size = strlen(password);
    int last_char_index = get_char_index(password[password_size - 1]);
    if (last_char_index == 93){
        // last char is the last char in the ascii list
        // increment the char before the last char
        password = (char*) realloc(password, sizeof(char) * (password_size + 1));
        // printf("Incrementing %dº char\n", password_size);
        password[password_size] = '\0';
        for (int i = password_size-1; i >= 0; i--){
            last_char_index = get_char_index(password[i]);
            if (last_char_index < 93){
                password[i] = get_char_from_index(last_char_index + 1);
                return password;
            }
            else{
                password[i] = get_char_from_index(0);
            }
        }
        // if we reach this point, we need to add a new char to the password
        // printf("Adding %dº char\n", password_size + 1);
        password = (char*) realloc(password, sizeof(char) * (password_size + 2));
        // printf("Incrementing %dº char\n", password_size + 1);
        password[password_size + 1] = '\0';
        password[0] = get_char_from_index(0);
        for (int i = 1; i < password_size + 1; i++){
            password[i] = get_char_from_index(0);
        }
        return password;
    }
    else{
        // last char is not the last char in the ascii list
        // increment the last char
        password[password_size - 1] = get_char_from_index(last_char_index + 1);
        return password;
    }
}


int verify_zip_pass(const char* archive_path, const char *dest_path, const char* password){
    // Create a new archive struct
    struct archive *zip;
    struct archive_entry *entry;
    int result;

    // Create a new archive struct
    zip = archive_read_new();

    // Support all known compression types
    archive_read_support_filter_all(zip);
    archive_read_support_format_all(zip);
    
    if ((result = archive_read_add_passphrase(zip, password)) == ARCHIVE_FATAL){
        // Error setting password
        printf("Error: %s\n", archive_error_string(zip));
        archive_read_close(zip);
        archive_read_free(zip);
        return result;
    }

    if((result = archive_read_open_filename(zip, archive_path, 10240))){
        // Error opening file
        printf("Error: %s\n", archive_error_string(zip));
        archive_read_close(zip);
        archive_read_free(zip);
        return result;
    }

    // Iterate through each file in the archive
    const char* entry_pathname; // Pathname of the current file (inside the archive)
    char dest_pathname[1024]; // Pathname of the current file (outside the archive)

    while ((result = archive_read_next_header(zip, &entry)) == ARCHIVE_OK){
        // Get the pathname of the current file
        entry_pathname = archive_entry_pathname(entry);
        // Create the full pathname of the current file
        snprintf(dest_pathname, sizeof(dest_pathname), "%s/%s", dest_path, entry_pathname);
        // Extract the current file
        archive_entry_set_pathname(entry, dest_pathname);
        result = archive_read_extract(zip, entry, ARCHIVE_EXTRACT_TIME);
        if (result != ARCHIVE_OK){
            // Error extracting file
            archive_read_close(zip);
            archive_read_free(zip);
            return 0;
        }
    }
    
    archive_read_close(zip);
    // Free the archive
    archive_read_free(zip);
    return 1; 
}

int get_char_index(char c){
    return (int) (c - 33);
}

char get_char_from_index(int index){
    return (char) (index + 33);
}