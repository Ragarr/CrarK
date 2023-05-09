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
char* generate_password(char* password, char* ascii_list, int ascii_list_size);
int get_char_index(char* ascii_list, char c);




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
    int result;
    char* password= NULL;
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
        password = generate_password(password, ascii_list, 93);
        // verify password
        printf("password%d: %s\n", iterations, password);
        result=verify_zip_pass(archive_path, "temp", password);
        iterations++;
        if (result == 1){
            printf("Password found: %s\n", password);
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

char* generate_password(char* password, char* ascii_list, int ascii_list_size){
    if (password == NULL){
        password = (char*) malloc(sizeof(char) * 2);
        password[0] = ascii_list[0];
        password[1] = '\0';
        return password;
    }
    int password_size = strlen(password);
    int last_char_index = get_char_index(ascii_list, password[password_size - 1]);
    if (last_char_index == ascii_list_size - 1){
        // last char is the last char in the ascii list
        // set the previous char to the next in the ascii list
        for (int i = password_size; i>=0; i--){
            if (get_char_index(ascii_list, password[i]) == ascii_list_size){
                // si el caracter anterior es el ultimo de la lista ascii
                // lo ponemos como el primero y seguimos con el anterior
                password[i] = ascii_list[0];
            }
            else{
                // si el caracter anterior no es el ultimo de la lista ascii
                // incrementamos el caracter anterior
                password[i] = ascii_list[get_char_index(ascii_list, password[i]) + 1];
                break;
            }
        }
        // add a new char to the password
        password = (char*) realloc(password, sizeof(char) * (password_size + 2));
        password[password_size] = ascii_list[0];
        password[password_size + 1] = '\0';
        return password;
    }
    else{
        // last char is not the last char in the ascii list
        // increment the last char
        password[password_size - 1] = ascii_list[last_char_index + 1];
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

int get_char_index(char* ascii_list, char c){
    for (int i = 0; i < (int)strlen(ascii_list); i++){
        if (ascii_list[i] == c){
            return i;
        }
    }
    return -1;
}