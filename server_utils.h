// server_utils.h

#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <errno.h>
#include <sys/stat.h>
// User roles
#define ADMIN 1
#define FACULTY 2
#define STUDENT 3
// Structures
typedef struct
{
    int id;
    char username[50];
    char password[50];
    int role;
    int active; // 1 for active, 0 for inactive
} User;

typedef struct
{
    int id;
    char name[100];
    char department[50];
    int semester;
    int active;
} Student;

typedef struct
{
    int id;
    char name[100];
    char department[50];
    int active;
} Faculty;

typedef struct
{
    int id;
    char name[100];
    char faculty_username[50];
    int max_seats;
    int available_seats;
    int active;
} Course;

typedef struct
{
    int student_id;
    int course_id;
} Enrollment;

// Function prototypes
void handle_client(int client_socket);
int authenticate_user(char *username, char *password, User *user);
void admin_menu(int client_socket, User *user);
void faculty_menu(int client_socket, User *user);
void student_menu(int client_socket, User *user);
void add_student(int client_socket);
void add_faculty(int client_socket);
void activation(int client_socket);
void update_details(int client_socket);
void add_course(int client_socket, User *user);
void remove_course(int client_socket, User *user);
void view_enrollments(int client_socket, User *user);
void change_password(int client_socket, User *user);
void enroll_course(int client_socket, User *user);
void unenroll_course(int client_socket, User *user);
void view_enrolled_courses(int client_socket, User *user);
int get_user_by_username(char *username, User *user);
int get_student_by_username(char *username, Student *student);
int get_faculty_by_username(char *username, Faculty *faculty);
void create_data_directory();
void initialize_files();
void send_to_client(int client_socket,  char *message);
void recieve(int client_socket, char *buffer);
void signup(int client_socket);
void send_faculty_menu(int client_fd);
void send_admin_menu(int client_fd);
void send_welcome_message(int client_fd);
void send_logout_message(int client_fd);
void send_student_menu(int client_fd);
void send_enrolled_courses(int client_fd);
void send_message_add_student(int client_fd);
void send_message_add_faculty(int client_fd);
void send_student_status(int client_fd);
void send_message_update(int client_fd);
void send_add_course(int client_fd);
void send_remove_course(int client_fd);
void send_message_faculty_courses(int client_fd);
void send_message_course_enrollments(int client_fd,  char *course_name);
void send_message_change_password(int client_fd);
void send_message_available_courses(int client_fd);
void send_message_create_account(int client_fd);
#endif // SERVER_UTILS_H
