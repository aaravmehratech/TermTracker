# TermTracker

**Developed by Aarav Mehra**

TermTracker is a completely standalone, terminal-based application written in C. It acts as a comprehensive academic planner designed to help students strictly manage their daily schedule, track actionable tasks, and algorithmically monitor their attendance to prevent falling below mandatory academic thresholds.

The entire application runs entirely in your local terminal window, utilizing ANSI escape codes to render a fully colorized, dynamic user interface. All user data is securely stored locally in a binary file (`termtracker.dat`).

---

## Core Features Detailed

TermTracker is divided into four main functional blocks. Here is exactly what each feature does:

### 1. Secure Account System
When you first run TermTracker, it forces you to create an account. 
* **Password Hashing:** It does not store your password in plain text. It uses the `djb2` hashing algorithm to convert your password into a mathematical integer. 
* **Data Protection:** Every time you log in, it hashes your input and compares it against the saved integer. 
* **Account Recovery:** You must provide a custom security question and answer during setup, which is also securely hashed and used if you ever lock yourself out.

### 2. Timetable Manager
This is a robust 2D grid that represents your physical academic week.
* **Structure:** It supports a standard 5-day academic week (Monday to Friday) with up to 10 periods per day.
* **Combined Classes:** The system supports dynamic scheduling. If you append an asterisk (`*`) to a subject (e.g., `Physics*`), the program automatically spans this class across two consecutive periods, treating it as a single block for attendance tracking.

### 3. Priority To-Do List
A built-in task manager that keeps track of your immediate academic goals.
* **Weighted Priorities:** When you add a task, you assign it a priority of High (Red), Medium (Yellow), or Low (Green). 
* **Smart Sorting:** You can trigger a bubble sort algorithm that instantly reorganizes your list, bringing pending High-priority tasks to the very top, and pushing completed tasks to the bottom.
* **Task Cleanup:** Features a single-button clear function that sweeps through your array, deleting completed items and shifting pending tasks up to prevent memory fragmentation.

### 4. Advanced Attendance Tracker
This is the mathematical core of the application, designed to keep you above a 75% attendance threshold.
* **Marking:** You can log a class as Present, Absent, or Cancelled. (Cancelled classes are ignored by the math).
* **Live Calculations:** For every subject, the application calculates your current percentage.
* **Predictive Math:** 
  * If you are *above* 75%, it calculates exactly how many consecutive classes you can afford to skip before you drop into the danger zone.
  * If you are *below* 75%, it calculates exactly how many consecutive classes you must attend to claw your way back to safety.

---

## How to Compile and Run

Make sure you have a C compiler installed on your system.

**1. Compile the code:**
```bash
gcc -o termtracker termtracker.c
```

**2. Run the application:**
```bash
./termtracker
```

---

## Data Structures Deep Dive

TermTracker organizes and stores your data using three primary C `struct` types. Here is a granular breakdown of how the memory is laid out:

### 1. The Task Structure (`struct task`)
This structure represents a single to-do list item.

```c
struct task {
    char text[60]; // A string holding the actual description of the task
    int pri;       // Priority level: 1 (High), 2 (Medium), 3 (Low)
    int done;      // Boolean flag: 0 (Pending), 1 (Completed)
};
```
* **Design Choice:** By keeping the priority (`pri`) and completion status (`done`) as simple integers, the program can execute sorting algorithms extremely fast by simply comparing integer weights.

### 2. The Attendance Record (`struct rec`)
This structure logs a single instance of attendance for a specific class on a specific day.

```c
struct rec {
    int date;      // Stored mathematically (e.g., 20261116)
    int period;    // The period number (0 to 9)
    char sub[20];  // Subject name (e.g., "Mathematics")
    char st;       // Status: 'P' (Present), 'A' (Absent), 'C' (Cancelled)
};
```
* **Design Choice:** Instead of storing complex formatted date strings (like "16-11-2026"), the date is packed mathematically into a single integer (`YYYYMMDD`). This makes it highly efficient for the CPU to search, sort, and retrieve your chronological attendance history using basic greater-than or less-than operators.

### 3. The Central Database (`struct data`)
This serves as the master container for the entire application's state. 

```c
struct data {
    char name[16];                // User's display name
    unsigned int pass;            // djb2 Hashed password 
    char q[80];                   // Security question string
    unsigned int ans;             // djb2 Hashed answer
    int ntask;                    // Current active count of tasks
    int nrec;                     // Current active count of attendance records
    char tt[DAYS][PERIODS][20];   // 3D Array: [5 Days][10 Periods][20-char Subject]
    struct task tasks[MAXTASK];   // Array of to-do list structs (Max 100)
    struct rec recs[MAXREC];      // Array of attendance structs (Max 3000)
};
```
* **Design Choice:** TermTracker relies on **static, fixed-size arrays** (e.g., allocating space for exactly 3000 records immediately). 
* **The Benefit:** Because the entire application state is grouped into this single `struct data db;` block in memory, the program can save everything to your hard drive in one single, lightning-fast line of code: `fwrite(&db, sizeof(struct data), 1, f);`.

---

## Future Memory Optimizations (Dynamic vs. Static)

While the current static array implementation allows for blazing-fast save/load times, it operates with strict limits (`MAXTASK 100` and `MAXREC 3000`) and wastes memory if those slots are empty.

A future optimization would be to migrate from fixed arrays to **Dynamic Memory Allocation** using **Linked Lists**.

If implemented, the static task array would be replaced by a dynamic chain:
```c
struct task {
    char text[60];
    int pri;
    int done;
    struct task *next; // Memory pointer to the next task in the chain
};
```

```diff
+ ADVANTAGE: Infinite scaling. 
+ You can add tasks infinitely until your physical RAM is exhausted.
+ Deleting a task is instant because you simply rewire the *next pointer.

- DISADVANTAGE: I/O Complexity.
- You can no longer save the program in one line of code. 
- The program must traverse the entire linked list, saving each node individually while stripping out the memory pointers (which become invalid the moment the program closes).
```