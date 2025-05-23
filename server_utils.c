#include "server_utils.h"
void send_faculty_menu(int client_fd)
{
    char *menu =
        "\n==========================================================\n"
        "                    FACULTY MENU\n"
        "==========================================================\n\n"
        "+------------------------------------------+\n"
        "| 1. Add new Course                        |\n"
        "| 2. Remove offered Course                 |\n"
        "| 3. View enrollments in Courses           |\n"
        "| 4. Password Change                       |\n"
        "| 5. Exit                                  |\n"
        "+------------------------------------------+\n"
        "Enter your choice: ";

    send_to_client(client_fd, menu);
}
void send_admin_menu(int client_fd)
{
     char *menu =
        "\n==========================================================\n"
        "                     ADMIN MENU\n"
        "==========================================================\n\n"
        "+------------------------------------------+\n"
        "| 1. Add Student                           |\n"
        "| 2. Add Faculty                           |\n"
        "| 3. Activate/Deactivate Student           |\n"
        "| 4. Update Student/Faculty details        |\n"
        "| 5. Exit                                  |\n"
        "+------------------------------------------+\n"
        "Enter your choice: ";

    send_to_client(client_fd, menu);
}

void send_welcome_message(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "            Welcome to Course Registration Portal\n");
    send_to_client(client_fd, "==========================================================\n\n");

    send_to_client(client_fd, "+---------------------------+\n");
    send_to_client(client_fd, "| Options:-                |\n");
    send_to_client(client_fd, "| 1. Login                 |\n");
    send_to_client(client_fd, "| 2. Sign Up               |\n");
    send_to_client(client_fd, "+---------------------------+\n");
    send_to_client(client_fd, "Enter your choice: ");
}
void send_enrolled_courses(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "                   ENROLLED COURSES\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "+-------+--------------------+--------------------+\n");
    send_to_client(client_fd, "| ID    | Course Name        | Faculty Name|\n");
    send_to_client(client_fd, "+-------+--------------------+--------------------+\n");
}
void send_logout_message(int client_fd)
{
    send_to_client(client_fd, "Logging out...\n");
}
void send_student_menu(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "                    STUDENT MENU\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "+------------------------------------------+\n");
    send_to_client(client_fd, "| 1. Enroll to new Courses                 |\n");
    send_to_client(client_fd, "| 2. Unenroll from already enrolled Courses|\n");
    send_to_client(client_fd, "| 3. View enrolled Courses                 |\n");
    send_to_client(client_fd, "| 4. Password Change                       |\n");
    send_to_client(client_fd, "| 5. Exit                                  |\n");
    send_to_client(client_fd, "+------------------------------------------+\n");
    send_to_client(client_fd, "Enter your choice: ");
}
void send_message_add_student(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "                     ADD STUDENT\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "Username: ");
}
void send_message_add_faculty(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "                     ADD FACULTY\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "Username: ");
}
void send_student_status(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "              ACTIVATE/DEACTIVATE STUDENT\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "Enter student username: ");
}
void send_message_update(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "            UPDATE STUDENT/FACULTY DETAILS\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "Enter username: ");
}
void send_add_course(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "                     ADD COURSE\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "Course Name: ");
}
void send_remove_course(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "                    REMOVE COURSE\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "Enter Course ID: ");
}
void send_message_faculty_courses(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "                     YOUR COURSES\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "+-------+--------------------+----------+---------------+\n");
    send_to_client(client_fd, "| ID    | Name               | Max Seats| Available     |\n");
    send_to_client(client_fd, "+-------+--------------------+----------+---------------+\n");
}
void send_message_course_enrollments(int client_fd,  char *course_name)
{
    char buffer[1024]; // Ensure buffer is large enough to hold course details
    send_to_client(client_fd, "\n==========================================================\n");
    sprintf(buffer, "          ENROLLMENTS FOR COURSE: %s\n", course_name);
    send_to_client(client_fd, buffer);
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "+------------+------------------------------------+\n");
    send_to_client(client_fd, "| Student ID | Student Name                       |\n");
    send_to_client(client_fd, "+------------+------------------------------------+\n");
}
void send_message_change_password(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "                   CHANGE PASSWORD\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "Enter new password: ");
}
void send_message_available_courses(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "                  AVAILABLE COURSES\n");
    send_to_client(client_fd, "==========================================================\n\n");
    send_to_client(client_fd, "+-------+--------------------+--------------------+---------------+\n");
    send_to_client(client_fd, "| ID    | Name               | Faculty            | Available     |\n");
    send_to_client(client_fd, "+-------+--------------------+--------------------+---------------+\n");
}
void send_message_create_account(int client_fd)
{
    send_to_client(client_fd, "\n==========================================================\n");
    send_to_client(client_fd, "                  CREATE NEW ACCOUNT\n");
    send_to_client(client_fd, "==========================================================\n\n");
}
