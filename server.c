#include "server_utils.h"
// Global variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// Main function
int main()
{
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t thread_id;
    // Create data directory and initialize files
    create_data_directory();
    initialize_files();
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(1);
    }
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Setsockopt failed");
        exit(1);
    }
    // Configure address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(1);
    }
    // Listen for connections
    if (listen(server_fd, 10) < 0)
    {
        perror("Listen failed");
        exit(1);
    }
    printf("Server started on port %d\n", 8080);
    // Accept connections and create threads to handle clients
    while (1)
    {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept failed");
            continue;
        }
        printf("New client connected\n");
        // Create a new thread to handle the client
        if (pthread_create(&thread_id, NULL, (void *)handle_client, (void *)(intptr_t)client_fd) != 0)
        {
            perror("Thread creation failed");
            close(client_fd);
            continue;
        }
        // Detach the thread to automatically clean up when it exits
        pthread_detach(thread_id);
    }
    // Close the server socket
    close(server_fd);
    return 0;
}
// Create data directory if it doesn't exist
void create_data_directory()
{
    // Using mkdir system call to create directory
    if (mkdir("data", 0777) == -1)
    {
        if (errno != EEXIST)
        {
            perror("Failed to create data directory");
            exit(1);
        }
    }
}
// Initialize files with default admin user if they don't exist
void initialize_files()
{
    int fd;
    // Create users file if it doesn't exist
    if ((fd = open("data/users.dat", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("Failed to open users file");
        exit(1);
    }
    // Check if the file is empty
    if (lseek(fd, 0, SEEK_END) == 0)
    {
        // File is empty, add default admin user
        User admin;
        admin.id = 1;
        strcpy(admin.username, "admin");
        strcpy(admin.password, "admin123");
        admin.role = ADMIN;
        admin.active = 1;
        // Write admin user to file
        if (write(fd, &admin, sizeof(User)) == -1)
        {
            perror("Failed to write admin user");
            close(fd);
            exit(1);
        }
        printf("Default admin user created\n");
    }
    close(fd);
    // Create other files if they don't exist
    if ((fd = open("data/students.dat", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("Failed to open students file");
        exit(1);
    }
    close(fd);
    if ((fd = open("data/faculty.dat", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("Failed to open faculty file");
        exit(1);
    }
    close(fd);
    if ((fd = open("data/courses.dat", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("Failed to open courses file");
        exit(1);
    }
    close(fd);
}
// Handle client connection
void handle_client(int client_fd)
{
    char username[50];
    char password[50];
    User user;
    char buffer[1024];
    char choice[1024];
    send_welcome_message(client_fd);
    recieve(client_fd, choice);
    if (atoi(choice) == 2)
    {
        signup(client_fd);
        close(client_fd);
        printf("Client disconnected\n");
        return;
    }
    // Get username
    send_to_client(client_fd, "Username: ");
    recieve(client_fd, username);
    // Get password
    send_to_client(client_fd, "Password: ");
    recieve(client_fd, password);
    // Authenticate user
    if (authenticate_user(username, password, &user))
    {
        // Check if user is active
        if (!user.active)
        {
            send_to_client(client_fd, "Your account is inactive. Please contact the administrator.\n");
            close(client_fd);
            return;
        }
        // Send welcome message based on role
        sprintf(buffer, "Welcome, %s!\n", username);
        send_to_client(client_fd, buffer);
        switch (user.role)
        {
        case ADMIN:
            admin_menu(client_fd, &user);
            break;
        case FACULTY:
            faculty_menu(client_fd, &user);
            break;
        case STUDENT:
            student_menu(client_fd, &user);
            break;
        default:
            send_to_client(client_fd, "Invalid role.Disconnecting.\n");
            break;
        }
    }
    else
    {
        send_to_client(client_fd, "Invalid username or password. Disconnecting.\n");
    }
    // Close client socket
    close(client_fd);
    printf("Client disconnected\n");
}
// Authenticate user
int authenticate_user(char *username, char *password, User *user)
{
    int fd;
    User t;
    // Open users file
    if ((fd = open("data/users.dat", O_RDONLY)) == -1)
    {
        perror("Failed to open users file");
        return 0;
    }
    // Read users from file and check credentials
    while (read(fd, &t, sizeof(User)) > 0)
    {
        if (strcmp(t.username, username) == 0 && strcmp(t.password, password) == 0)
        {
            *user = t;
            close(fd);
            return 1;
        }
    }
    close(fd);
    return 0;
}
// Admin menu
void admin_menu(int client_fd, User *user)
{
    char choice[1024];
    int flag = 1;
    while (flag)
    {
        send_admin_menu(client_fd);
        // Get choice
        recieve(client_fd, choice);
        // Process choice
        switch (atoi(choice))
        {
        case 1:
            add_student(client_fd);
            break;
        case 2:
            add_faculty(client_fd);
            break;
        case 3:
            activation(client_fd);
            break;
        case 4:
            update_details(client_fd);
            break;
        case 5:
            send_logout_message(client_fd);
            flag = 0;
            break;
        default:
            send_to_client(client_fd, "Invalid choice. Please try again.\n");
            break;
        }
    }
}
// Faculty menu
void faculty_menu(int client_fd, User *user)
{
    char choice[1024];
    int flag = 1;
    while (flag)
    {
        send_faculty_menu(client_fd);
        recieve(client_fd, choice);
        // Process choice
        switch (atoi(choice))
        {
        case 1:
            add_course(client_fd, user);
            break;
        case 2:
            remove_course(client_fd, user);
            break;
        case 3:
            view_enrollments(client_fd, user);
            break;
        case 4:
            change_password(client_fd, user);
            break;
        case 5:
            send_logout_message(client_fd);
            flag = 0;
            break;
        default:
            send_to_client(client_fd, "Invalid choice. Please try again.\n");
            break;
        }
    }
}
// Student menu
void student_menu(int client_fd, User *user)
{
    char choice[1024];
    int flag = 1;
    while (flag)
    {
        send_student_menu(client_fd);
        // Get choice
        recieve(client_fd, choice);
        // Process choice
        switch (atoi(choice))
        {
        case 1:
            enroll_course(client_fd, user);
            break;
        case 2:
            unenroll_course(client_fd, user);
            break;
        case 3:
            view_enrolled_courses(client_fd, user);
            break;
        case 4:
            change_password(client_fd, user);
            break;
        case 5:
            send_logout_message(client_fd);
            flag = 0;
            break;
        default:
            send_to_client(client_fd, "Invalid choice. Please try again.\n");
            break;
        }
    }
}
// Add student
void add_student(int client_fd)
{
    char buffer[1024];
    Student student;
    User user;
    int fd, user_fd;
    send_message_add_student(client_fd);
    recieve(client_fd, buffer);
    // Check if username already exists
    if (get_user_by_username(buffer, &user))
    {
        send_to_client(client_fd, "Username already exists. Please try again.\n");
        return;
    }
    strcpy(user.username, buffer);
    // Get password
    send_to_client(client_fd, "Password: ");
    recieve(client_fd, user.password);
    // Get name
    send_to_client(client_fd, "Name: ");
    recieve(client_fd, student.name);
    // Get department
    send_to_client(client_fd, "Department: ");
    recieve(client_fd, student.department);
    // Get semester
    send_to_client(client_fd, "Semester: ");
    recieve(client_fd, buffer);
    student.semester = atoi(buffer);
    // Set student as active
    student.active = 1;
    // Lock files for writing
    pthread_mutex_lock(&mutex);
    // Open student file
    if ((fd = open("data/students.dat", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("Failed to open students file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to add student. Please try again.\n");
        return;
    }
    // Get next student ID
    student.id = lseek(fd, 0, SEEK_END) / sizeof(Student) + 1;
    // Write student to file
    if (write(fd, &student, sizeof(Student)) == -1)
    {
        perror("Failed to write student");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to add student. Please try again.\n");
        return;
    }
    close(fd);
    // Open user file
    if ((user_fd = open("data/users.dat", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("Failed to open users file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to add student. Please try again.\n");
        return;
    }
    // Get next user ID
    user.id = lseek(user_fd, 0, SEEK_END) / sizeof(User) + 1;
    user.role = STUDENT;
    user.active = 1;
    // Write user to file
    if (write(user_fd, &user, sizeof(User)) == -1)
    {
        perror("Failed to write user");
        close(user_fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to add student. Please try again.\n");
        return;
    }
    close(user_fd);
    // Unlock files
    pthread_mutex_unlock(&mutex);
    send_to_client(client_fd, "Student added successfully.\n");
}
// Add faculty
void add_faculty(int client_fd)
{
    char buffer[1024];
    Faculty faculty;
    User user;
    int fd, user_fd;
    send_message_add_faculty(client_fd);
    recieve(client_fd, buffer);
    // Check if username already exists
    if (get_user_by_username(buffer, &user))
    {
        send_to_client(client_fd, "Username already exists. Please try again.\n");
        return;
    }
    strcpy(user.username, buffer);
    // Get password
    send_to_client(client_fd, "Password: ");
    recieve(client_fd, user.password);
    // Get name
    send_to_client(client_fd, "Name: ");
    recieve(client_fd, faculty.name);
    // Get department
    send_to_client(client_fd, "Department: ");
    recieve(client_fd, faculty.department);
    // Set faculty as active
    faculty.active = 1;
    // Lock files for writing
    pthread_mutex_lock(&mutex);
    // Open faculty file
    if ((fd = open("data/faculty.dat", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("Failed to open faculty file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to add faculty. Please try again.\n");
        return;
    }
    // Get next faculty ID
    faculty.id = lseek(fd, 0, SEEK_END) / sizeof(Faculty) + 1;
    // Write faculty to file
    if (write(fd, &faculty, sizeof(Faculty)) == -1)
    {
        perror("Failed to write faculty");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to add faculty. Please try again.\n");
        return;
    }
    close(fd);
    // Open user file
    if ((user_fd = open("data/users.dat", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("Failed to open users file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to add faculty. Please try again.\n");
        return;
    }
    // Get next user ID
    user.id = lseek(user_fd, 0, SEEK_END) / sizeof(User) + 1;
    user.role = FACULTY;
    user.active = 1;
    // Write user to file
    if (write(user_fd, &user, sizeof(User)) == -1)
    {
        perror("Failed to write user");
        close(user_fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to add faculty. Please try again.\n");
        return;
    }
    close(user_fd);
    // Unlock files
    pthread_mutex_unlock(&mutex);
    send_to_client(client_fd, "Faculty added successfully.\n");
}
// Activate/deactivate student
void activation(int client_fd)
{
    char buffer[1024];
    User user;
    Student student;
    int fd, user_fd;
    send_student_status(client_fd);
    recieve(client_fd, buffer);
    // Check if username exists and is a student
    if (!get_user_by_username(buffer, &user) || user.role != STUDENT)
    {
        send_to_client(client_fd, "Student not found. Please try again.\n");
        return;
    }
    // Get student details
    if (!get_student_by_username(buffer, &student))
    {
        send_to_client(client_fd, "Student details not found. Please try again.\n");
        return;
    }
    // Toggle active status
    user.active = !user.active;
    student.active = !student.active;
    // Lock files for writing
    pthread_mutex_lock(&mutex);
    // Update user file
    if ((user_fd = open("data/users.dat", O_RDWR)) == -1)
    {
        perror("Failed to open users file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to update student status. Please try again.\n");
        return;
    }
    // Seek to user position
    if (lseek(user_fd, (user.id - 1) * sizeof(User), SEEK_SET) == -1)
    {
        perror("Failed to seek in users file");
        close(user_fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to update student status. Please try again.\n");
        return;
    }
    // Write updated user
    if (write(user_fd, &user, sizeof(User)) == -1)
    {
        perror("Failed to write user");
        close(user_fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to update student status. Please try again.\n");
        return;
    }
    close(user_fd);
    // Update student file
    if ((fd = open("data/students.dat", O_RDWR)) == -1)
    {
        perror("Failed to open students file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to update student status. Please try again.\n");
        return;
    }
    // Seek to student position
    if (lseek(fd, (student.id - 1) * sizeof(Student), SEEK_SET) == -1)
    {
        perror("Failed to seek in students file");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to update student status. Please try again.\n");
        return;
    }
    // Write updated student
    if (write(fd, &student, sizeof(Student)) == -1)
    {
        perror("Failed to write student");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to update student status. Please try again.\n");
        return;
    }
    close(fd);
    // Unlock files
    pthread_mutex_unlock(&mutex);
    // Send success message
    if (student.active)
    {
        send_to_client(client_fd, "Student activated successfully.\n");
    }
    else
    {
        send_to_client(client_fd, "Student deactivated successfully.\n");
    }
}
// Update student/faculty details
void update_details(int client_fd)
{
    char buffer[1024];
    User user;
    Student student;
    Faculty faculty;
    int fd;
    send_message_update(client_fd);
    recieve(client_fd, buffer);
    // Check if username exists
    if (!get_user_by_username(buffer, &user))
    {
        send_to_client(client_fd, "User not found. Please try again.\n");
        return;
    }
    // Update based on role
    if (user.role == STUDENT)
    {
        // Get student details
        if (!get_student_by_username(buffer, &student))
        {
            send_to_client(client_fd, "Student details not found. Please try again.\n");
            return;
        }
        // Get updated details
        send_to_client(client_fd, "Enter new name (leave empty to keep current): ");
        recieve(client_fd, buffer);
        if (strlen(buffer) > 0)
        {
            strcpy(student.name, buffer);
        }
        send_to_client(client_fd, "Enter new department (leave empty to keep current): ");
        recieve(client_fd, buffer);
        if (strlen(buffer) > 0)
        {
            strcpy(student.department, buffer);
        }
        send_to_client(client_fd, "Enter new semester (leave empty to keep current): ");
        recieve(client_fd, buffer);
        if (strlen(buffer) > 0)
        {
            student.semester = atoi(buffer);
        }
        // Lock file for writing
        pthread_mutex_lock(&mutex);
        // Open student file
        if ((fd = open("data/students.dat", O_RDWR)) == -1)
        {
            perror("Failed to open students file");
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "Failed to update student details. Please try again.\n");
            return;
        }
        // Seek to student position
        if (lseek(fd, (student.id - 1) * sizeof(Student), SEEK_SET) == -1)
        {
            perror("Failed to seek in students file");
            close(fd);
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "Failed to update student details. Please try again.\n");
            return;
        }
        // Write updated student
        if (write(fd, &student, sizeof(Student)) == -1)
        {
            perror("Failed to write student");
            close(fd);
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "Failed to update student details. Please try again.\n");
            return;
        }
        close(fd);
        // Unlock file
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Student details updated successfully.\n");
    }
    else if (user.role == FACULTY)
    {
        // Get faculty details
        if (!get_faculty_by_username(buffer, &faculty))
        {
            send_to_client(client_fd, "Faculty details not found. Please try again.\n");
            return;
        }
        // Get updated details
        send_to_client(client_fd, "Enter new name (leave empty to keep current): ");
        recieve(client_fd, buffer);
        if (strlen(buffer) > 0)
        {
            strcpy(faculty.name, buffer);
        }
        send_to_client(client_fd, "Enter new department (leave empty to keep current): ");
        recieve(client_fd, buffer);
        if (strlen(buffer) > 0)
        {
            strcpy(faculty.department, buffer);
        }
        // Lock file for writing
        pthread_mutex_lock(&mutex);
        // Open faculty file
        if ((fd = open("data/faculty.dat", O_RDWR)) == -1)
        {
            perror("Failed to open faculty file");
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "Failed to update faculty details. Please try again.\n");
            return;
        }
        // Seek to faculty position
        if (lseek(fd, (faculty.id - 1) * sizeof(Faculty), SEEK_SET) == -1)
        {
            perror("Failed to seek in faculty file");
            close(fd);
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "Failed to update faculty details. Please try again.\n");
            return;
        }
        // Write updated faculty
        if (write(fd, &faculty, sizeof(Faculty)) == -1)
        {
            perror("Failed to write faculty");
            close(fd);
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "Failed to update faculty details. Please try again.\n");
            return;
        }
        close(fd);
        // Unlock file
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Faculty details updated successfully.\n");
    }
    else
    {
        send_to_client(client_fd, "Cannot update admin details.\n");
    }
}
// Add course
void add_course(int client_fd, User *user)
{
    char buffer[1024];
    Course course;
    int fd;
    Faculty faculty;
    // Get faculty details
    if (!get_faculty_by_username(user->username, &faculty))
    {
        send_to_client(client_fd, "Faculty details not found. Please contact the administrator.\n");
        return;
    }
    send_add_course(client_fd);
    recieve(client_fd, course.name);
    // Get max seats
    send_to_client(client_fd, "Maximum Seats: ");
    recieve(client_fd, buffer);
    course.max_seats = atoi(buffer);
    course.available_seats = course.max_seats;
    // Set course as active and assign faculty
    course.active = 1;
    strcpy(course.faculty_username, user->username);
    // Lock file for writing
    pthread_mutex_lock(&mutex);
    // Open course file
    if ((fd = open("data/courses.dat", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("Failed to open courses file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to add course. Please try again.\n");
        return;
    }
    // Get next course ID
    course.id = lseek(fd, 0, SEEK_END) / sizeof(Course) + 1;
    // Write course to file
    if (write(fd, &course, sizeof(Course)) == -1)
    {
        perror("Failed to write course");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to add course. Please try again.\n");
        return;
    }
    close(fd);
    // Unlock file
    pthread_mutex_unlock(&mutex);
    send_to_client(client_fd, "Course added successfully.\n");
}
// Remove course
void remove_course(int client_fd, User *user)
{
    char buffer[1024];
    Course course;
    int fd, found = 0;
    send_remove_course(client_fd);
    recieve(client_fd, buffer);
    int course_id = atoi(buffer);
    // Lock file for reading and writing
    pthread_mutex_lock(&mutex);
    // Open course file
    if ((fd = open("data/courses.dat", O_RDWR)) == -1)
    {
        perror("Failed to open courses file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to remove course. Please try again.\n");
        return;
    }
    // Seek to course position
    if (lseek(fd, (course_id - 1) * sizeof(Course), SEEK_SET) == -1)
    {
        perror("Failed to seek in courses file");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Course not found. Please try again.\n");
        return;
    }
    // Read course
    if (read(fd, &course, sizeof(Course)) != sizeof(Course))
    {
        perror("Failed to read course");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Course not found. Please try again.\n");
        return;
    }
    // Check if course belongs to faculty
    if (strcmp(course.faculty_username, user->username) != 0)
    {
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "You can only remove courses that you have created.\n");
        return;
    }
    // Deactivate course
    course.active = 0;
    // Seek back to course position
    if (lseek(fd, (course_id - 1) * sizeof(Course), SEEK_SET) == -1)
    {
        perror("Failed to seek in courses file");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to remove course. Please try again.\n");
        return;
    }
    // Write updated course
    if (write(fd, &course, sizeof(Course)) == -1)
    {
        perror("Failed to write course");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to remove course. Please try again.\n");
        return;
    }
    close(fd);
    // Unlock file
    pthread_mutex_unlock(&mutex);
    send_to_client(client_fd, "Course removed successfully.\n");
}
// View enrollments
void view_enrollments(int client_fd, User *user)
{
    char buffer[1024];
    Course course;
    int fd, enr_fd;
    Enrollment enrollment;
    Student student;

    send_message_faculty_courses(client_fd);

    // Lock file for reading
    pthread_mutex_lock(&mutex);

    // Open course file
    if ((fd = open("data/courses.dat", O_RDONLY)) == -1)
    {
        perror("Failed to open courses file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to view courses. Please try again.\n");
        return;
    }

    // Display courses taught by this faculty
    while (read(fd, &course, sizeof(Course)) == sizeof(Course))
    {
        if (course.active && strcmp(course.faculty_username, user->username) == 0)
        {
            sprintf(buffer, "| %-5d | %-18s | %-8d | %-13d |\n",
                    course.id, course.name, course.max_seats, course.available_seats);
            send_to_client(client_fd, buffer);
        }
    }
    send_to_client(client_fd, "+-------+--------------------+----------+---------------+\n");
    close(fd);

    // Get course ID to view enrollments
    send_to_client(client_fd, "\nEnter Course ID to view enrollments (0 to cancel): ");
    recieve(client_fd, buffer);
    int course_id = atoi(buffer);
    if (course_id == 0)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }

    // Open course file and validate course
    if ((fd = open("data/courses.dat", O_RDONLY)) == -1)
    {
        perror("Failed to open courses file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to view enrollments. Please try again.\n");
        return;
    }

    int course_found = 0;
    while (read(fd, &course, sizeof(Course)) == sizeof(Course))
    {
        if (course.id == course_id)
        {
            course_found = 1;
            break;
        }
    }
    close(fd);

    if (!course_found)
    {
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Course not found. Please try again.\n");
        return;
    }

    if (strcmp(course.faculty_username, user->username) != 0)
    {
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "You can only view enrollments for courses that you have created.\n");
        return;
    }

    // Create enrollment file path
    char enrollment_file[100];
    sprintf(enrollment_file, "data/enrollments_%d.dat", course_id);

    if ((enr_fd = open(enrollment_file, O_RDONLY)) == -1)
    {
        if (errno == ENOENT)
        {
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "No enrollments for this course.\n");
            return;
        }
        perror("Failed to open enrollments file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to view enrollments. Please try again.\n");
        return;
    }

    send_message_course_enrollments(client_fd, course.name);

    // Read enrollments and find student details
    while (read(enr_fd, &enrollment, sizeof(Enrollment)) == sizeof(Enrollment))
    {
        int student_fd = open("data/students.dat", O_RDONLY);
        if (student_fd == -1)
        {
            perror("Failed to open students file");
            continue;
        }

        int found = 0;
        while (read(student_fd, &student, sizeof(Student)) == sizeof(Student))
        {
            if (student.id == enrollment.student_id && student.active)
            {
                found = 1;
                break;
            }
        }
        close(student_fd);

        if (!found)
        {
            fprintf(stderr, "Student ID %d not found or inactive\n", enrollment.student_id);
            continue;
        }

        // Display student info
        sprintf(buffer, "| %-10d | %-34s |\n", student.id, student.name);
        send_to_client(client_fd, buffer);
    }

    send_to_client(client_fd, "+------------+------------------------------------+\n");
    close(enr_fd);
    pthread_mutex_unlock(&mutex);
}

// Change password
void change_password(int client_fd, User *user)
{
    char buffer[1024];
    int fd;
    send_message_change_password(client_fd);
    recieve(client_fd, buffer);
    // Update password
    strcpy(user->password, buffer);
    // Lock file for writing
    pthread_mutex_lock(&mutex);
    // Open user file
    if ((fd = open("data/users.dat", O_RDWR)) == -1)
    {
        perror("Failed to open users file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to change password. Please try again.\n");
        return;
    }
    // Seek to user position
    if (lseek(fd, (user->id - 1) * sizeof(User), SEEK_SET) == -1)
    {
        perror("Failed to seek in users file");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to change password. Please try again.\n");
        return;
    }
    // Write updated user
    if (write(fd, user, sizeof(User)) == -1)
    {
        perror("Failed to write user");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to change password. Please try again.\n");
        return;
    }
    close(fd);
    // Unlock file
    pthread_mutex_unlock(&mutex);
    send_to_client(client_fd, "Password changed successfully.\n");
}
// Enroll course
void enroll_course(int client_fd, User *user)
{
    char buffer[1024];
    Course course;
    int fd, enr_fd;
    Enrollment enrollment;
    Student student;
    // Get student details
    if (!get_student_by_username(user->username, &student))
    {
        send_to_client(client_fd, "Student details not found. Please contact the administrator.\n");
        return;
    }
    send_message_available_courses(client_fd);
    pthread_mutex_lock(&mutex);
    // Open course file
    if ((fd = open("data/courses.dat", O_RDONLY)) == -1)
    {
        perror("Failed to open courses file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to view courses. Please try again.\n");
        return;
    }
    // Read courses
    while (read(fd, &course, sizeof(Course)) == sizeof(Course))
    {
        if (course.active && course.available_seats > 0)
        {
            // Get faculty name
            Faculty faculty;
            if (get_faculty_by_username(course.faculty_username, &faculty))
            {
                sprintf(buffer, "| %-5d | %-18s | %-18s | %-13d |\n", course.id, course.name, faculty.name, course.available_seats);
                send_to_client(client_fd, buffer);
            }
        }
    }
    send_to_client(client_fd, "+-------+--------------------+--------------------+---------------+\n");
    close(fd);
    // Get course ID
    send_to_client(client_fd, "\nEnter Course ID to enroll (0 to cancel): ");
    recieve(client_fd, buffer);
    int course_id = atoi(buffer);
    if (course_id == 0)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }
    // Open course file with write lock
    if ((fd = open("data/courses.dat", O_RDWR)) == -1)
    {
        perror("Failed to open courses file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to enroll in course. Please try again.\n");
        return;
    }
    // Lock the specific course record
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = (course_id - 1) * sizeof(Course);
    lock.l_len = sizeof(Course);
    // Apply the lock
    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        perror("Failed to lock course record");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to enroll in course. Please try again.\n");
        return;
    }
    // Seek to course position
    if (lseek(fd, (course_id - 1) * sizeof(Course), SEEK_SET) == -1)
    {
        perror("Failed to seek in courses file");
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Course not found. Please try again.\n");
        return;
    }
    // Read course
    if (read(fd, &course, sizeof(Course)) != sizeof(Course))
    {
        perror("Failed to read course");
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Course not found. Please try again.\n");
        return;
    }
    // Check if course is active and has available seats
    if (!course.active)
    {
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "This course is not available for enrollment.\n");
        return;
    }
    if (course.available_seats <= 0)
    {
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "This course has no available seats.\n");
        return;
    }
    // Create enrollment file path
    char enrollment_file[100];
    sprintf(enrollment_file, "data/enrollments_%d.dat", course_id);
    // Check if student is already enrolled
    if ((enr_fd = open(enrollment_file, O_RDONLY)) != -1)
    {
        while (read(enr_fd, &enrollment, sizeof(Enrollment)) == sizeof(Enrollment))
        {
            if (enrollment.student_id == student.id)
            {
                close(enr_fd);
                // Release the lock
                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                pthread_mutex_unlock(&mutex);
                send_to_client(client_fd, "You are already enrolled in this course.\n");
                return;
            }
        }
        close(enr_fd);
    }
    // Open enrollment file for writing
    if ((enr_fd = open(enrollment_file, O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1)
    {
        perror("Failed to open enrollments file");
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to enroll in course. Please try again.\n");
        return;
    }
    // Create enrollment record
    enrollment.student_id = student.id;
    enrollment.course_id = course_id;
    // Write enrollment record
    if (write(enr_fd, &enrollment, sizeof(Enrollment)) == -1)
    {
        perror("Failed to write enrollment");
        close(enr_fd);
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to enroll in course. Please try again.\n");
        return;
    }
    close(enr_fd);
    // Update available seats
    course.available_seats--;
    // Seek back to course position
    if (lseek(fd, (course_id - 1) * sizeof(Course), SEEK_SET) == -1)
    {
        perror("Failed to seek in courses file");
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to update course. Please try again.\n");
        return;
    }
    // Write updated course
    if (write(fd, &course, sizeof(Course)) == -1)
    {
        perror("Failed to write course");
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to update course. Please try again.\n");
        return;
    }
    // Release the lock
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    // Unlock file
    pthread_mutex_unlock(&mutex);
    send_to_client(client_fd, "Enrolled in course successfully.\n");
}
// Unenroll course
void unenroll_course(int client_fd, User *user)
{
    char buffer[1024];
    Course course;
    int fd, enr_fd, t_fd;
    Enrollment enrollment;
    Student student;
    // Get student details
    if (!get_student_by_username(user->username, &student))
    {
        send_to_client(client_fd, "Student details not found. Please contact the administrator.\n");
        return;
    }
    // Display enrolled courses
    send_enrolled_courses(client_fd);
    // Lock file for reading
    pthread_mutex_lock(&mutex);
    // Open course file
    if ((fd = open("data/courses.dat", O_RDONLY)) == -1)
    {
        perror("Failed to open courses file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to view courses. Please try again.\n");
        return;
    }
    // Read courses
    int found = 0;
    while (read(fd, &course, sizeof(Course)) == sizeof(Course))
    {
        if (course.active)
        {
            // Create enrollment file path
            char enrollment_file[100];
            sprintf(enrollment_file, "data/enrollments_%d.dat", course.id);
            // Check if student is enrolled
            if ((enr_fd = open(enrollment_file, O_RDONLY)) != -1)
            {
                int enrolled = 0;
                while (read(enr_fd, &enrollment, sizeof(Enrollment)) == sizeof(Enrollment))
                {
                    if (enrollment.student_id == student.id)
                    {
                        enrolled = 1;
                        break;
                    }
                }
                close(enr_fd);
                if (enrolled)
                {
                    // Get faculty name
                    Faculty faculty;
                    if (get_faculty_by_username(course.faculty_username, &faculty))
                    {
                        sprintf(buffer, "| %-5d | %-18s | %-18s |\n", course.id, course.name, faculty.name);
                        send_to_client(client_fd, buffer);
                        found = 1;
                    }
                }
            }
        }
    }
    send_to_client(client_fd, "+-------+--------------------+--------------------+\n");
    close(fd);
    if (!found)
    {
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "You are not enrolled in any courses.\n");
        return;
    }
    // Get course ID
    send_to_client(client_fd, "\nEnter Course ID to unenroll (0 to cancel): ");
    recieve(client_fd, buffer);
    int course_id = atoi(buffer);
    if (course_id == 0)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }
    // Open course file with write lock
    if ((fd = open("data/courses.dat", O_RDWR)) == -1)
    {
        perror("Failed to open courses file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to unenroll from course. Please try again.\n");
        return;
    }
    // Lock the specific course record
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = (course_id - 1) * sizeof(Course);
    lock.l_len = sizeof(Course);
    // Apply the lock
    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        perror("Failed to lock course record");
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to unenroll from course. Please try again.\n");
        return;
    }
    // Seek to course position
    if (lseek(fd, (course_id - 1) * sizeof(Course), SEEK_SET) == -1)
    {
        perror("Failed to seek in courses file");
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Course not found. Please try again.\n");
        return;
    }
    // Read course
    if (read(fd, &course, sizeof(Course)) != sizeof(Course))
    {
        perror("Failed to read course");
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Course not found. Please try again.\n");
        return;
    }
    // Create enrollment file path
    char enrollment_file[100];
    sprintf(enrollment_file, "data/enrollments_%d.dat", course_id);
    // Create temporary file path
    char temp_file[100];
    sprintf(temp_file, "data/temp_enrollments_%d.dat", course_id);
    // Open enrollment file for reading
    if ((enr_fd = open(enrollment_file, O_RDONLY)) == -1)
    {
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "You are not enrolled in this course.\n");
        return;
    }
    // Open temporary file for writing
    if ((t_fd = open(temp_file, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
    {
        perror("Failed to open temporary file");
        close(enr_fd);
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to unenroll from course. Please try again.\n");
        return;
    }
    // Copy enrollments except the one to remove
    int f_enroll = 0;
    while (read(enr_fd, &enrollment, sizeof(Enrollment)) == sizeof(Enrollment))
    {
        if (enrollment.student_id == student.id)
        {
            f_enroll = 1;
        }
        else
        {
            if (write(t_fd, &enrollment, sizeof(Enrollment)) == -1)
            {
                perror("Failed to write enrollment");
                close(enr_fd);
                close(t_fd);
                unlink(temp_file);
                // Release the lock
                lock.l_type = F_UNLCK;
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                pthread_mutex_unlock(&mutex);
                send_to_client(client_fd, "Failed to unenroll from course. Please try again.\n");
                return;
            }
        }
    }
    close(enr_fd);
    close(t_fd);
    if (!f_enroll)
    {
        unlink(temp_file);
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "You are not enrolled in this course.\n");
        return;
    }
    // Replace enrollment file with temporary file
    if (rename(temp_file, enrollment_file) == -1)
    {
        perror("Failed to rename file");
        unlink(temp_file);
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to unenroll from course. Please try again.\n");
        return;
    }
    // Update available seats
    course.available_seats++;
    // Seek back to course position
    if (lseek(fd, (course_id - 1) * sizeof(Course), SEEK_SET) == -1)
    {
        perror("Failed to seek in courses file");
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to update course. Please try again.\n");
        return;
    }
    // Write updated course
    if (write(fd, &course, sizeof(Course)) == -1)
    {
        perror("Failed to write course");
        // Release the lock
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        close(fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to update course. Please try again.\n");
        return;
    }
    // Release the lock
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
    // Unlock file
    pthread_mutex_unlock(&mutex);
    send_to_client(client_fd, "Unenrolled from course successfully.\n");
}
// View enrolled courses
void view_enrolled_courses(int client_fd, User *user)
{
    char buffer[1024];
    Course course;
    int fd, enr_fd;
    Enrollment enrollment;
    Student student;
    // Get student details
    if (!get_student_by_username(user->username, &student))
    {
        send_to_client(client_fd, "Student details not found. Please contact the administrator.\n");
        return;
    }
    send_enrolled_courses(client_fd);
    // Lock file for reading
    pthread_mutex_lock(&mutex);
    // Open course file
    if ((fd = open("data/courses.dat", O_RDONLY)) == -1)
    {
        perror("Failed to open courses file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to view courses. Please try again.\n");
        return;
    }
    // Read courses
    int found = 0;
    while (read(fd, &course, sizeof(Course)) == sizeof(Course))
    {
        if (course.active)
        {
            // Create enrollment file path
            char enrollment_file[100];
            sprintf(enrollment_file, "data/enrollments_%d.dat", course.id);
            // Check if student is enrolled
            if ((enr_fd = open(enrollment_file, O_RDONLY)) != -1)
            {
                int enrolled = 0;
                // Set read lock
                struct flock lock;
                lock.l_type = F_RDLCK;
                lock.l_whence = SEEK_SET;
                lock.l_start = 0;
                lock.l_len = 0; // Lock the entire file
                // Apply the lock
                if (fcntl(enr_fd, F_SETLKW, &lock) == -1)
                {
                    perror("Failed to lock enrollment file");
                    close(enr_fd);
                    continue;
                }
                while (read(enr_fd, &enrollment, sizeof(Enrollment)) == sizeof(Enrollment))
                {
                    if (enrollment.student_id == student.id)
                    {
                        enrolled = 1;
                        break;
                    }
                }
                // Release the lock
                lock.l_type = F_UNLCK;
                fcntl(enr_fd, F_SETLK, &lock);
                close(enr_fd);
                if (enrolled)
                {
                    // Get faculty name
                    Faculty faculty;
                    if (get_faculty_by_username(course.faculty_username, &faculty))
                    {
                        sprintf(buffer, "| %-5d | %-18s | %-18s |\n", course.id, course.name, faculty.name);
                        send_to_client(client_fd, buffer);
                        found = 1;
                    }
                }
            }
        }
    }
    close(fd);
    // Unlock file
    pthread_mutex_unlock(&mutex);
    if (!found)
    {
        send_to_client(client_fd, "You are not enrolled in any courses.\n");
    }
    send_to_client(client_fd, "+-------+--------------------+--------------------+\n");
}
// Get user by username
int get_user_by_username(char *username, User *user)
{
    int fd;
    User t;
    // Open users file
    if ((fd = open("data/users.dat", O_RDONLY)) == -1)
    {
        perror("Failed to open users file");
        return 0;
    }
    // Read users from file
    while (read(fd, &t, sizeof(User)) > 0)
    {
        if (strcmp(t.username, username) == 0)
        {
            *user = t;
            close(fd);
            return 1;
        }
    }
    close(fd);
    return 0;
}
// Get student by username
int get_student_by_username(char *username, Student *student)
{
    int fd, user_fd;
    User user;
    Student t;
    // Get user by username
    if (!get_user_by_username(username, &user) || user.role != STUDENT)
    {
        return 0;
    }
    // Open students file
    if ((fd = open("data/students.dat", O_RDONLY)) == -1)
    {
        perror("Failed to open students file");
        return 0;
    }
    // Read students from file
    while (read(fd, &t, sizeof(Student)) > 0)
    {
        if (t.id == user.id)
        {
            *student = t;
            close(fd);
            return 1;
        }
    }
    close(fd);
    return 0;
}
// Get faculty by username
int get_faculty_by_username(char *username, Faculty *faculty)
{
    int fd, user_fd;
    User user;
    Faculty t;
    // Get user by username
    if (!get_user_by_username(username, &user) || user.role != FACULTY)
    {
        return 0;
    }
    // Open faculty file
    if ((fd = open("data/faculty.dat", O_RDONLY)) == -1)
    {
        perror("Failed to open faculty file");
        return 0;
    }
    // Read faculty from file
    while (read(fd, &t, sizeof(Faculty)) > 0)
    {
        if (t.id == user.id)
        {
            *faculty = t;
            close(fd);
            return 1;
        }
    }
    close(fd);
    return 0;
}
// Send message to client
void send_to_client(int client_fd, char *message)
{
    if (write(client_fd, message, strlen(message)) == -1)
    {
        perror("Failed to send message");
    }
}
// Receive message from client
void recieve(int client_fd, char *buffer)
{
    int bytes_read = read(client_fd, buffer, 1024 - 1);
    if (bytes_read <= 0)
    {
        if (bytes_read == 0)
        {
            // Client disconnected
            printf("Client disconnected\n");
        }
        else
        {
            perror("Failed to receive message");
        }
        buffer[0] = '\0';
        return;
    }
    // Remove newline character
    if (buffer[bytes_read - 1] == '\n')
    {
        buffer[bytes_read - 1] = '\0';
    }
    else
    {
        buffer[bytes_read] = '\0';
    }
}
// Create a new account
void signup(int client_fd)
{
    char buffer[1024];
    User user;
    int user_fd;
    send_message_create_account(client_fd);
    // Get username
    send_to_client(client_fd, "Username: ");
    recieve(client_fd, buffer);
    // Check if username already exists
    if (get_user_by_username(buffer, &user))
    {
        send_to_client(client_fd, "Username already exists. Please try again.\n");
        return;
    }
    strcpy(user.username, buffer);
    // Get password
    send_to_client(client_fd, "Password: ");
    recieve(client_fd, user.password);
    // Get role
    send_to_client(client_fd, "Role (1 for Admin, 2 for Faculty, 3 for Student): ");
    recieve(client_fd, buffer);
    user.role = atoi(buffer);
    if (user.role < 1 || user.role > 3)
    {
        send_to_client(client_fd, "Invalid role. Please try again.\n");
        return;
    }
    // Set user as active
    user.active = 1;
    // Lock file for writing
    pthread_mutex_lock(&mutex);
    // Open user file
    if ((user_fd = open("data/users.dat", O_RDWR | O_CREAT, 0666)) == -1)
    {
        perror("Failed to open users file");
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to create account. Please try again.\n");
        return;
    }
    // Get next user ID
    user.id = lseek(user_fd, 0, SEEK_END) / sizeof(User) + 1;
    // Write user to file
    if (write(user_fd, &user, sizeof(User)) == -1)
    {
        perror("Failed to write user");
        close(user_fd);
        pthread_mutex_unlock(&mutex);
        send_to_client(client_fd, "Failed to create account. Please try again.\n");
        return;
    }
    close(user_fd);
    // If role is student or faculty, create corresponding record
    if (user.role == STUDENT)
    {
        Student student;
        int fd;
        // Get student details
        send_to_client(client_fd, "Name: ");
        recieve(client_fd, student.name);
        send_to_client(client_fd, "Department: ");
        recieve(client_fd, student.department);
        send_to_client(client_fd, "Semester: ");
        recieve(client_fd, buffer);
        student.semester = atoi(buffer);
        // Set student as active
        student.active = 1;
        student.id = user.id;
        // Open student file
        if ((fd = open("data/students.dat", O_RDWR | O_CREAT, 0666)) == -1)
        {
            perror("Failed to open students file");
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "Failed to create student record. Please try again.\n");
            return;
        }
        // Write student to file
        if (write(fd, &student, sizeof(Student)) == -1)
        {
            perror("Failed to write student");
            close(fd);
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "Failed to create student record. Please try again.\n");
            return;
        }
        close(fd);
    }
    else if (user.role == FACULTY)
    {
        Faculty faculty;
        int fd;
        // Get faculty details
        send_to_client(client_fd, "Name: ");
        recieve(client_fd, faculty.name);
        send_to_client(client_fd, "Department: ");
        recieve(client_fd, faculty.department);
        // Set faculty as active
        faculty.active = 1;
        faculty.id = user.id;
        // Open faculty file
        if ((fd = open("data/faculty.dat", O_RDWR | O_CREAT, 0666)) == -1)
        {
            perror("Failed to open faculty file");
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "Failed to create faculty record. Please try again.\n");
            return;
        }
        // Write faculty to file
        if (write(fd, &faculty, sizeof(Faculty)) == -1)
        {
            perror("Failed to write faculty");
            close(fd);
            pthread_mutex_unlock(&mutex);
            send_to_client(client_fd, "Failed to create faculty record. Please try again.\n");
            return;
        }
        close(fd);
    }
    // Unlock file
    pthread_mutex_unlock(&mutex);
    send_to_client(client_fd, "Account created successfully. Please login with your new credentials.\n");
}
