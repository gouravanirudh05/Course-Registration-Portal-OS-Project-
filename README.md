# Mini Project - Course Registration Portal (Academia)

**Gourav Anirudh – IMT2023005**  


---

## 1. Overview

This project simulates a **Course Registration Portal** using a **client-server architecture** via the terminal. The portal supports **three types of users**: Admin, Faculty, and Student — each with role-specific privileges.

Communication between the client and server is handled using **socket programming**, and data is stored in **files protected by locking mechanisms** to ensure consistency during concurrent access.

---

## 2. Application Architecture

The system comprises the following components:

- `client.c` – Client-side logic for handling different roles (Admin/Faculty/Student).  
- `server.c` – Server-side logic that handles all user requests.  
- `server_utils.h` – Contains shared macros, constants, and structure definitions.  
- `students.dat`, `faculty.dat`, `courses.dat`, `users.dat`, `enrollments.dat` – File-based data stores for persistence.  
- `Makefile` – For compiling the system easily.

---

## 3. How to Run the Code

1. Open **two terminals**.

2. In the **first terminal**, compile and run the server:
``` bash
gcc server.c server_utils.c -o server
./server
```
3. In the **second terminal**, compile and run the client:
``` bash
gcc client.c -o client
./client
```
4. Follow the role-based login and navigate the menu-based system.

## 4. `server_utils.h`

Key structures and constants defined:

- `User`: username, password, role (Admin/Faculty/Student), active status
    
- `Student`, `Faculty`: profile and academic details
    
- `Course`: name, assigned faculty, max and available seats, active status
    
- `Enrollment`: links student ID to course ID
## 5. Functionalities of Each User
### Admin
- Add Student / Faculty
- Activate / Deactivate Student
- Update Student / Faculty Details
- Add / Remove Courses
- Assign Faculty to Courses
- View all Users and Courses
- Change Password
    
###  Student
- Enroll in Courses (if seats available)
- Unenroll from Courses
- View Enrolled Courses
- Change Password
### Faculty
- Add / Remove Courses
- View Enrollments in Their Courses
- Change Password

## 6. Functionalities of the Server

The server handles:
- User authentication and session management
- Role-based request routing
- File I/O operations with file locking
- Course seat validation and enrollment
- Support for multiple clients via `pthread_create()`
## 7. Implementation of `client.c`

###  Key Functions

- `loginMenu()` – Select role and login
- `adminMenu()`, `facultyMenu()`, `studentMenu()` – Role-specific menu loops
- Uses `read_message()`/`send_message()` for socket communication
    Each menu option gathers inputs and sends requests to the server for execution.
    
## 8. Implementation of `server.c`

### Key Helper Functions
- `authenticate_user()` – Validates credentials
- `add_student()`, `add_faculty()` – Adds users to database
- `add_course()`, `remove_course()` – Course management
- `enroll_course()`, `unenroll_course()` – Handles student enrollments
- `view_enrollments()` – Lists students per course
- `change_password()` – Password update logic
- Uses `fcntl()` for file locking (read/write based on operation)
## 9. OS Concepts Used

###  File Locking
- `fcntl()` is used to implement locking:
    - **Read locks** for viewing (e.g., course list, enrollments)
    - **Write locks** for modifying (e.g., enroll/unenroll, update course)
        

### Socket Programming
-  **Server-side**:
    - `socket()`, `bind()`, `listen()`, `accept()`
- **Client-side**:
    - `socket()`, `connect()`
- **Data exchange**:
    - `recieve_message()`, `send_message()` between server and client
        

### File Handling
- Basic system calls used:
        - `open()`, `read()`, `write()`, `close()`
        
- Files used:
    - `students.dat`, `faculty.dat`, `courses.dat`, `users.dat`, `enrollments.dat`
        

### IPC 

- `pthread_create()` system call enables handling multiple clients concurrently by creating a thread for each client
## 10. Example Screenshots
![[/Images/Pasted image 20250515091036.png]]
![[/Images/Pasted image 20250515091059.png]]-  Creating new Student
      ![[Pasted image 20250515091215.png]]


 - Faculty Adding course 
    ![[Pasted image 20250515120328.png]]
 - Student Enrolling to course 
	 ![[Pasted image 20250515120637.png]]
 - Viewing all the courses enrolled to 
	  ![[Pasted image 20250515120723.png]]
 - Unenrolling from a course 
	 ![[Pasted image 20250515120752.png]]
	 
 - Viewing enrollments in the faculty role
	![[Pasted image 20250515131844.png]]
- Admin activating or deactivating a student
	![[Pasted image 20250515132204.png]]
-  The deactivated student
	![[Pasted image 20250515132231.png]]
- Changing password 
	![[Pasted image 20250515132311.png]]
- Removing of course by faculty 
	![[Pasted image 20250515132357.png]]
- Admin updating student details 
	![[Pasted image 20250515132455.png]]
- Admin adding student
	![[Pasted image 20250515132608.png]]
	 
 