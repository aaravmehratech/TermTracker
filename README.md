# TermTracker

TermTracker is a lightweight, terminal-based application written in C designed to help students manage their daily academic lives. It features a customizable weekly timetable, a priority-based to-do list, and an attendance tracker to ensure you maintain your required attendance percentage.

All user data is stored locally in a binary file (`termtracker.dat`), and the app includes a secure login system using password hashing.

## Features

* **Account System**: User login with password hashing (djb2 algorithm) and a security question for recovery.
* **Timetable Manager**: Track classes across a 5-day week, 10 periods a day. Supports combined/double classes.
* **Priority To-Do List**: Add tasks with High, Medium, or Low priority. Sort, mark as complete, and clear finished tasks.
* **Attendance Tracker**: Log Present, Absent, or Cancelled classes. Calculates overall percentage and tells you exactly how many classes you need to attend (or can afford to miss) to maintain a 75% threshold.

---

## How to Compile and Run

Make sure you have a C compiler (like GCC) installed on your system.

**1. Compile the code:**
```bash
gcc -o termtracker termtracker.c
```

**2. Run the application (Linux/macOS):**
```bash
./termtracker
```

*(If you are on Windows, run `termtracker.exe` instead).*

---

## Data Structures Explained

The application organizes and stores your data using three primary C `struct` types. Here is a breakdown of how they work:

### 1. `struct task` (To-Do List Item)
```c
struct task {
    char text[60]; // The actual description of the task
    int pri;       // Priority level: 1 (High), 2 (Medium), 3 (Low)
    int done;      // Boolean flag: 0 (Pending), 1 (Completed)
};
```
This structure represents a single actionable item. By keeping the priority (`pri`) as a simple integer, the program can easily run a sorting algorithm to bring high-priority, pending tasks to the top of your list.

### 2. `struct rec` (Attendance Record)
```c
struct rec {
    int date;      // Stored as YYYYMMDD (e.g., 20261116)
    int period;    // The period number (0 to 9)
    char sub[20];  // Subject name
    char st;       // Status: 'P' (Present), 'A' (Absent), 'C' (Cancelled)
};
```
Instead of storing complex formatted date strings, the date is packed mathematically into a single integer. This makes it highly efficient for the computer to search, sort, and retrieve your attendance history.

### 3. `struct data` (The Central Database)
```c
struct data {
    char name[16];                // User's display name
    unsigned int pass;            // Hashed password (not plain text)
    char q[80];                   // Security question
    unsigned int ans;             // Hashed answer to the security question
    int ntask;                    // Current number of active tasks
    int nrec;                     // Current number of attendance records
    char tt[DAYS][PERIODS][20];   // 3D array for the Timetable [Day][Period][Subject String]
    struct task tasks[MAXTASK];   // Array of to-do list tasks (Max 100)
    struct rec recs[MAXREC];      // Array of attendance records (Max 3000)
};
```
This serves as the master container for the entire application. When the app starts, it reads this entire structure from `termtracker.dat` in one single chunk using `fread()`. When you make a change (like adding a task), it overwrites the file using `fwrite()`. 

---

## Memory Management: Fixed Arrays vs. Linked Lists

TermTracker currently uses **static, fixed-size arrays** (e.g., `MAXTASK 100` and `MAXREC 3000`) rather than dynamic memory allocation. 

### Why Fixed Arrays?
* **The Pros:** It allows the program to save and load the entire `struct data` state to a file in a single, lightning-fast line of code. 
* **The Cons:** It wastes a bit of memory. Even if you only have 2 tasks, the program still allocates memory for 100. Furthermore, if you hit 3,001 attendance records, the app runs out of space.

### How Linked Lists & Dynamic Memory Would Change It
Instead of demanding space for 3000 records immediately, dynamic memory allows a program to ask the operating system for memory *on the fly* using `malloc()`.

If implemented, the `task` struct would become a **Linked List** node:
```c
struct task {
    char text[60];
    int pri;
    int done;
    struct task *next; // Pointer to the next task in the chain
};
```
* **The Advantage:** There is no `MAXTASK` limit. You can add tasks infinitely until your RAM runs out. Deleting a task in the middle of the list is also faster because you just rewire the pointer, rather than shifting a massive array up or down.
* **The Trade-off:** Saving a linked list to a file is much harder. You can't just `fwrite` the whole database in one line anymore; you have to loop through the chain and save each item individually, carefully ignoring the memory pointers (since memory addresses change every time you restart the program).

---

## Git & GitHub Setup Guide

If you want to host this code on GitHub, you need to make sure your personal data and passwords (stored in `termtracker.dat`) are ignored by Git.

### Step 1: Initial Local Setup (VS Code Terminal)
Run these commands inside your project folder:
```bash
# 1. Turn the folder into a Git repository
git init

# 2. Create a .gitignore file so your personal data isn't uploaded
echo -e "termtracker.dat\ntermtracker\n*.exe\n*.out\n.vscode/" > .gitignore

# 3. Add all your code to the staging area
git add .

# 4. Save this version of your code
git commit -m "Initial commit: Added TermTracker source code and README"
```

### Step 2: Push to GitHub
1. Go to GitHub and create a **New Repository** (do not add a README or .gitignore from the GitHub website).
2. Copy the URL of your new repository.
3. Run these commands to upload your code:
```bash
git remote add origin <YOUR_GITHUB_REPO_URL_HERE>
git branch -M main
git push -u origin main
```

### Step 3: Updating Your Code Later
Whenever you change `termtracker.c` or this README, run these three commands to send the updates to GitHub:
```bash
git add .
git commit -m "Describe what you changed here"
git push
```