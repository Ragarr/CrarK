# include <stdio.h>
# include <archive.h>
# include <archive_entry.h>
# include <sys/stat.h>
# include <stdlib.h>
#include <fcntl.h>
# include <string.h>

// TYPEDEFS

// PROTYPES
int verify_zip_pass(const char* archive_path, const char* dest_path, const char* password);
char* generate_password(char* password);
int get_char_index(char c);
char get_char_from_index(int index);




int main(int argc, char* argv[]){
    // Check if the user has provided the correct number of arguments
    if (argc != 3){
        printf("Usage: %s <archive_path> <max_iterations>\n", argv[0]);
        return 1;
    }
    int iterations = 0;
    char* archive_path = argv[1];
    int MAX_ITERATIONS = atoi(argv[2]);
    
    int result;
    char* password= NULL;
    // el ascii va desde el 33 al 126
    // create temp folder 
    mkdir("temp", 0777);
    // test password
    while (iterations < MAX_ITERATIONS){
        // generate password
        password = generate_password(password);
        // verify password
        // printf("password%d: %s\n", iterations, password);
        result=verify_zip_pass(archive_path, "temp", password);
        iterations++;
        if (result == 1){
            printf("Password found: %s\n", password);
            break;
        }
        if (iterations % 10000 == 0){
            // printf("Iteration: %d\n", iterations);
            system("clear");
            printf("%% complete: %f\n", (float) iterations / MAX_ITERATIONS * 100);
        }
    }
    if (result == 0){
        printf("Password not found\n");
    }
    // delete temp folder with all files
    system("rm -r temp");

    return 0;
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
        printf("Adding %dº char\n", password_size + 1);
        password = (char*) realloc(password, sizeof(char) * (password_size + 2));
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
    return (int) c - 33;
}

char get_char_from_index(int index){
    return (char) index + 33;
}