# include "password_utils.h"
# include <stdio.h>
# include <archive.h>
# include <archive_entry.h>
# include <sys/stat.h>
# include <stdlib.h>
# include <fcntl.h>
# include <string.h>
# include <pthread.h>
# include <math.h>

char *get_password_from_iteration(int n){
    int base = 93;
    int i = 0;
    int n2 = n;
    if (n==0){
        return NULL;
    }
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

char* generate_next_password(char* password){
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