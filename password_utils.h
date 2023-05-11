#ifndef PASSWORD_UTILS_H
#define PASSWORD_UTILS_H

int verify_zip_pass(const char* archive_path, const char* dest_path, const char* password);
char* generate_next_password_0(char* password);
char* generate_next_password_1(char* password);
int get_char_index(char c);
char get_char_from_index(int index);
char *get_password_from_iteration_0(int iteration);
char *get_password_from_iteration_1(int iteration, int max_length);

#endif
