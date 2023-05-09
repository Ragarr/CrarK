# include <stdio.h>
# include <archive.h>
# include <archive_entry.h>

// PROTYPES
int extract_file(const char* archive_path, const char* dest_path);





int main(int argc, char* argv[]){
    // Check if the user has provided the correct number of arguments
    if (argc != 3){
        printf("Usage: %s <archive_path> <dest_path>\n", argv[0]);
        return 1;
    }
    // Extract the zip file
    extract_file(argv[1], argv[2]);
    return 0;
}

int extract_file(const char* archive_path, const char* dest_path){
    struct archive *zip;
    struct archive_entry *entry;
    int result;

    // Create a new archive struct
    zip = archive_read_new();

    // Support all known compression types
    archive_read_support_filter_all(zip);
    archive_read_support_format_all(zip);
    
    if((result = archive_read_open_filename(zip, archive_path, 10240))){
        // Error opening file
        printf("Error: %s\n", archive_error_string(zip));
        return 1;
    }

    // Iterate through each file in the archive
    const char* entry_pathname; // Pathname of the current file (inside the archive)
    char dest_pathname[1024]; // Pathname of the current file (outside the archive)
    
    while ((result = archive_read_next_header(zip, &entry)) == ARCHIVE_OK){
        // we store the current file in the archive in the entry variable
        // Get the name of the file
        entry_pathname = archive_entry_pathname(entry);
        // Create the destination pathname
        sprintf(dest_pathname, "%s/%s", dest_path, entry_pathname);
        // Set the pathname of the entry to the destination pathname
        archive_entry_set_pathname(entry, dest_pathname);
        // Extract the file
        archive_read_extract(zip, entry, ARCHIVE_EXTRACT_TIME);
    } // end while
    // we have finished extracting the archive
    // Close the archive
    archive_read_close(zip);
    // Free the archive
    archive_read_free(zip);
    return 0;
}