# include <stdio.h>
# include <archive.h>
# include <archive_entry.h>
# include <sys/stat.h>
# include <stdlib.h>
#include <fcntl.h>
# include <string.h>

// TYPEDEFS

typedef struct password_try
{
    char* password;
    int last_char_index;
}password_try;

// PROTYPES
int verify_zip_pass(const char* archive_path, const char* dest_path, const char* password);
password_try* generate_password(password_try *pass_try, char* ascii_list, int ascii_list_size);




int main(int argc, char* argv[]){
    // Check if the user has provided the correct number of arguments
    if (argc != 3){
        printf("Usage: %s <archive_path> <max_iterations>\n", argv[0]);
        return 1;
    }
    int iterations = 0;
    char* archive_path = argv[1];
    int MAX_ITERATIONS = atoi(argv[2]);
    char *ascii_list;
    ascii_list = (char*) malloc(sizeof(char) * 93);
    int fd = open("ascii_list.txt", O_RDONLY);
    if (fd == -1){
        printf("Error opening ascii_list.txt\n");
        return 1;
    }
    password_try* pass_try = NULL;
    int result;
    // read ascii list
    read(fd, ascii_list, 93);
    if (close(fd) == -1){
        printf("Error closing ascii_list.txt\n");
        return 1;
    }
    printf("%s\n", ascii_list);

    // create temp folder 
    mkdir("temp", 0777);
    // test password
    while (iterations < MAX_ITERATIONS){
        // generate password
        pass_try = generate_password(pass_try, ascii_list, 93);
        // verify password
        printf("password%d: %s\n", iterations, pass_try->password);
        result=verify_zip_pass(archive_path, "temp", pass_try->password);
        iterations++;
        if (result == 1){
            printf("Password found: %s\n", pass_try->password);
            break;
        }
    }
    if (result == 0){
        printf("Password not found\n");
    }
    // delete temp folder with all files
    system("rm -r temp");

    return 0;
}

password_try* generate_password(password_try *pass_try, char* ascii_list, int ascii_list_size){
    if (pass_try==NULL){
        password_try* new_try = (password_try*) malloc(sizeof(password_try));
        new_try->password = (char*) malloc(sizeof(char) * 2);
        new_try->password[0] = ascii_list[0];
        new_try->password[1] = '\0';
        new_try->last_char_index = 0;
        return new_try;
    }
    char *password = pass_try->password;
    // printf("last_pass=%s\n", password);
    int last_char_index = pass_try->last_char_index;
    
    // if last char is not the last char in the ascii list
    // get the next char in the ascii list
    if (last_char_index < ascii_list_size-1){
        // printf("old_char=%c\n", password[strlen(password)-1]);
        password[strlen(password)-1] = ascii_list[last_char_index+1];
        pass_try->password = password;
        pass_try->last_char_index = last_char_index+1;
        // printf("new_char=%c\n", ascii_list[last_char_index+1]);

    }
    // if last char is the last char in the ascii list
    // add a new char to the password
    else{
        // set all previous chars to the first char in the ascii list
        // or if its already the first char, set it to the next char
        for (int i = 0; i < (int)strlen(password); i++){
            if (password[i] == ascii_list[0]){
                password[i] = ;
            }
            else{
                password[i] = ascii_list[pass_try->last_char_index+1];
                break;
            }
        }
        password = (char*) realloc(password, sizeof(char) * (strlen(password)+2));
        password[strlen(password)] = ascii_list[0];
        password[strlen(password)+1] = '\0';
        pass_try->password = password;
        pass_try->last_char_index = 0;
    }
    return pass_try;
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

