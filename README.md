# TermTracker

**Developed by Aarav Mehra**

TermTracker is a completely standalone, terminal-based application written in C. It acts as a comprehensive academic planner designed to help students strictly manage their daily schedule, track actionable tasks, and algorithmically monitor their attendance to prevent falling below mandatory academic thresholds.

The entire application runs entirely in your local terminal window, utilizing ANSI escape codes to render a fully colorized, dynamic user interface. All user data is securely stored locally in a binary file (`termtracker.dat`).

---

## Complete User Guide: Menus & Navigation

Once you log in, you are presented with the main menu. Here is exactly how to use every feature, input trick, and sub-menu in the application.

### 1. To-Do List
This menu manages your actionable tasks.

* **1 Add task:** Prompts you to type a task description, and then asks for a priority level: `1` for High (Red), `2` for Medium (Yellow), or `3` for Low (Green).
* **2 Mark task done:** Asks for the task number. It will cross the task out, turn it green, and dim the text to show it is completed.
* **3 Delete task:** Asks for a task number and completely erases it from existence, shifting the rest of the list up.
* **4 Sort by priority:** Runs a sorting algorithm that instantly groups your pending tasks by priority, pushing all High-priority tasks to the very top, and moving completed items to the bottom.
* **5 Remove completed tasks:** A cleanup utility. It sweeps through your entire list, deletes anything marked "done," and condenses the array to free up space.

### 2. Attendance Marker
This menu is your daily tracker.

* **1 Mark attendance:** 
  * It will ask for a date (`DD-MM-YYYY`). If you just press **`Enter`**, it defaults to today's date automatically.
  * It loops through your scheduled classes for that day. Type `P` for Present, `A` for Absent, or `C` for Cancelled. 
  * *Trick:* If a class hasn't happened yet, or you don't want to log it, just press **`Enter`** to skip it without saving any status.
* **2 See a day's attendance:** Enter any past date to see exactly what classes you attended or missed on that specific day.
* **3 All subjects summary:** Prints a master table showing every subject, your attended/total ratio, a visual percentage bar, and a mathematical calculation of whether you are "Safe" or in "Danger" regarding the 75% threshold.
* **4 Subject statistics:** Select a specific subject to see a deep-dive analysis. It shows total classes held, exactly how many you missed, and lists the exact dates of your latest 10 absences so you can spot patterns.

### 3. Timetable
This is where you build your weekly schedule. It runs on a 5-day, 10-period grid.

* **1 Change one period:** Allows you to select a specific day and a specific period (1-10) to edit without touching the rest of your schedule.
* **2 Change a whole day:** Loops through periods 1 to 10 for a single day, letting you build the day from scratch.
* **3 Change all days:** The nuclear option; prompts you to build your entire Monday-Friday schedule from scratch.

**Special Input Rules for the Timetable:**
* **The Asterisk (`*`) - Combined Classes:** If you have a double period (e.g., a 2-hour lab), type your subject followed by an asterisk (e.g., `Physics*`). The program will automatically link this period and the next one together. They will count as a single, combined block in your attendance tracker.
* **Pressing `Enter` (Blank):** If you have a free period, do not type anything. Just press the `Enter` key. The program will record it as a blank slot (`-`) and the attendance tracker will ignore it.
* **The Period (`.`) - Skip/Keep:** When editing a whole day, if you want to keep the subject that is already saved in a period, simply type a period (`.`) and press Enter. It will skip to the next period without erasing your old data.

---

## Core Features Detailed

TermTracker is divided into four main functional blocks:

### 1. Secure Account System
When you first run TermTracker, it forces you to create an account.
* **Password Hashing:** It does not store your password in plain text. It uses the `djb2` hashing algorithm to convert your password into a mathematical integer.
* **Account Recovery:** You must provide a custom security question and answer during setup, which is also securely hashed and used if you ever lock yourself out.

### 2. Timetable Manager
* **Structure:** It supports a standard 5-day academic week (Monday to Friday) with up to 10 periods per day, automatically inserting a lunch break between period 6 and 7.

### 3. Priority To-Do List
* **Smart Sorting:** You can trigger a bubble sort algorithm that instantly reorganizes your list, bringing pending High-priority tasks to the very top.

### 4. Advanced Attendance Tracker
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

## Data Structures

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
```
