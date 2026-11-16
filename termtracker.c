#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define DAYS 5
#define PERIODS 10
#define MAXTASK 100
#define MAXREC 3000
#define MAXSUB 60

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define BOLD "\033[1m"
#define DIM "\033[2m"
#define RESET "\033[0m"

struct task {
    char text[60];
    int pri;
    int done;
};

struct rec {
    int date;
    int period;
    char sub[20];
    char st;
};

struct data {
    char name[16];
    unsigned int pass;
    char q[80];
    unsigned int ans;
    int ntask;
    int nrec;
    char tt[DAYS][PERIODS][20];
    struct task tasks[MAXTASK];
    struct rec recs[MAXREC];
};

struct data db;
const char *filename = "termtracker.dat";
const char *dayname[DAYS] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
const char *ptime[PERIODS] = {"8:10-9:00", "9:00-9:50", "9:50-10:40", "10:40-11:30", "11:30-12:20", "12:20-1:10", "1:40-2:30", "2:30-3:20", "3:20-4:10", "4:10-5:00"};
const char *pristr[3] = {"High", "Med", "Low"};

void getstr(char *msg, char *s, int n) {
    int c;
    printf("%s", msg);
    if (fgets(s, n, stdin) == NULL) {
        printf("\n");
        exit(0);
    }
    if (strchr(s, '\n') == NULL) {
        while ((c = getchar()) != '\n' && c != EOF) {}
    }
    s[strcspn(s, "\n")] = 0;
}

int getnum(char *msg) {
    char b[20];
    getstr(msg, b, 20);
    return atoi(b);
}

void upper(char *s) {
    int i;
    for (i = 0; s[i] != 0; i++) {
        s[i] = toupper((unsigned char)s[i]);
    }
}

void lower(char *s) {
    int i;
    for (i = 0; s[i] != 0; i++) {
        s[i] = tolower((unsigned char)s[i]);
    }
}

void trim(char *s) {
    int i, j = 0;
    int len = strlen(s);
    while (len > 0 && s[len - 1] == ' ') {
        len--;
    }
    s[len] = 0;
    while (s[j] == ' ') {
        j++;
    }
    for (i = 0; s[j + i] != 0; i++) {
        s[i] = s[j + i];
    }
    s[i] = 0;
}

unsigned int hash(char *s) {
    unsigned int h = 5381;
    int i;
    for (i = 0; s[i] != 0; i++) {
        h = h * 33 + (unsigned char)s[i];
    }
    return h;
}

int todaydate() {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return (tm->tm_year + 1900) * 10000 + (tm->tm_mon + 1) * 100 + tm->tm_mday;
}

int parsedate(char *b, int *date, int *wd) {
    int dd, mm, yy;
    struct tm t;
    time_t now;
    if (b[0] == 0) {
        now = time(NULL);
        t = *localtime(&now);
        *date = (t.tm_year + 1900) * 10000 + (t.tm_mon + 1) * 100 + t.tm_mday;
        *wd = t.tm_wday;
        return 1;
    }
    if (sscanf(b, "%d-%d-%d", &dd, &mm, &yy) != 3) {
        return 0;
    }
    memset(&t, 0, sizeof(t));
    t.tm_mday = dd;
    t.tm_mon = mm - 1;
    t.tm_year = yy - 1900;
    t.tm_hour = 12;
    if (yy < 2000 || yy > 2100) {
        return 0;
    }
    mktime(&t);
    if (t.tm_mday != dd || t.tm_mon != mm - 1 || t.tm_year != yy - 1900) {
        return 0;
    }
    *date = yy * 10000 + mm * 100 + dd;
    *wd = t.tm_wday;
    return 1;
}

void printdate(int date) {
    printf("%02d-%02d-%d", date % 100, (date / 100) % 100, date / 10000);
}

void savedata() {
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        printf(RED "Cannot save your data\n" RESET);
        return;
    }
    fwrite(&db, sizeof(struct data), 1, f);
    fclose(f);
}

int loaddata() {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        return 0;
    }
    if (fread(&db, sizeof(struct data), 1, f) != 1 || db.ntask < 0 || db.ntask > MAXTASK || db.nrec < 0 || db.nrec > MAXREC || db.name[0] == 0) {
        fclose(f);
        memset(&db, 0, sizeof(struct data));
        return -1;
    }
    fclose(f);
    return 1;
}

int newpassword(unsigned int *h) {
    char p1[50], p2[50];
    getstr("New password (min 4 characters): ", p1, 50);
    if (strlen(p1) < 4) {
        printf(RED "Too short\n" RESET);
        return 0;
    }
    getstr("Type it again: ", p2, 50);
    if (strcmp(p1, p2) != 0) {
        printf(RED "Passwords don't match\n" RESET);
        return 0;
    }
    *h = hash(p1);
    return 1;
}

int hasstar(char *s) {
    int len = strlen(s);
    return len > 0 && s[len - 1] == '*';
}

void stripstar(char *s) {
    int len = strlen(s);
    if (len > 0 && s[len - 1] == '*') {
        s[len - 1] = 0;
    }
}

int classtype(int dd, int p) {
    int q = 0;
    while (q < PERIODS) {
        if (hasstar(db.tt[dd][q]) && q != 5 && q + 1 < PERIODS && strcmp(db.tt[dd][q], db.tt[dd][q + 1]) == 0) {
            if (q == p) return 1;
            if (q + 1 == p) return 2;
            q = q + 2;
        } else {
            q++;
        }
    }
    return 0;
}

int setperiod(int dd, int p) {
    int star;
    char s[40], m[100];
    sprintf(m, "period %-2d %-11s [%s]: ", p + 1, ptime[p], db.tt[dd][p][0] ? db.tt[dd][p] : "-");
    getstr(m, s, 20);
    trim(s);
    if (strcmp(s, ".") == 0) {
        return classtype(dd, p) == 1;
    }
    upper(s);
    star = hasstar(s);
    if (star) {
        stripstar(s);
        trim(s);
    }
    s[18] = 0;
    if (star && s[0] != 0) {
        strcat(s, "*");
    }
    strcpy(db.tt[dd][p], s);
    if (hasstar(s)) {
        if (p == 5 || p == PERIODS - 1) {
            stripstar(db.tt[dd][p]);
            if (p == 5) printf("      A combined class cannot go across lunch, so * is ignored\n");
            else printf("      There is no next period, so * is ignored\n");
            return 0;
        }
        strcpy(db.tt[dd][p + 1], s);
        printf("Period %-2d %-11s %s  (combined with period %d)\n", p + 2, ptime[p + 1], s, p + 1);
        return 1;
    }
    return 0;
}

void checkstars(int dd) {
    int p;
    for (p = 0; p < PERIODS; p++) {
        if (hasstar(db.tt[dd][p]) && classtype(dd, p) == 0) {
            printf("Note: period %d has a * but the next period does not have the same subject with *\n", p + 1);
            if (p == 5 || p == 6) printf("      (a combined class cannot go across lunch)\n");
            printf("      So it counts as a normal class\n");
        }
    }
}

void editday(int dd) {
    int p;
    printf("\n" CYAN BOLD "--- %s ---" RESET "\n", dayname[dd]);
    printf("(leave blank if no class, type . to keep the old one)\n");
    printf("(put * after the subject for a combined class, it fills the next period too)\n");
    for (p = 0; p < PERIODS; p++) {
        if (p == 6) printf("      ---- lunch 1:10 to 1:40 ----\n");
        if (setperiod(dd, p)) p++;
    }
    checkstars(dd);
}

void showtimetable() {
    int p, dd;
    printf("\n%-12s", "");
    for (dd = 0; dd < DAYS; dd++) {
        printf("%-10.3s", dayname[dd]);
    }
    printf("\n");
    for (p = 0; p < PERIODS; p++) {
        if (p == 6) printf("   -- lunch --\n");
        printf("%-12s", ptime[p]);
        for (dd = 0; dd < DAYS; dd++) {
            printf("%-10.9s", db.tt[dd][p][0] ? db.tt[dd][p] : "-");
        }
        printf("\n");
    }
    printf("(- means no class)\n");
}

int createaccount() {
    char name[50], ans[50], q[80];
    int i;
    memset(&db, 0, sizeof(struct data));
    getstr("Your name (max 15 letters): ", name, 50);
    trim(name);
    if (name[0] == 0 || strlen(name) > 15) {
        printf(RED "Name must be 1 to 15 characters\n" RESET);
        return 0;
    }
    strcpy(db.name, name);
    if (!newpassword(&db.pass)) {
        return 0;
    }
    printf("\nMake a security question (used if you forget your password)\n");
    getstr("Your question: ", q, 80);
    trim(q);
    if (strlen(q) < 5) {
        printf(RED "Question is too short\n" RESET);
        return 0;
    }
    strcpy(db.q, q);
    getstr("Your answer: ", ans, 50);
    lower(ans);
    trim(ans);
    if (ans[0] == 0) {
        printf(RED "Answer cannot be empty\n" RESET);
        return 0;
    }
    db.ans = hash(ans);
    printf("\nAccount created! Now let's fill your timetable.\n");
    printf("For every day type the subject for each period.\n");
    printf("If there is no class in a period just press enter.\n");
    printf("If there is no class the whole day, press enter for all periods.\n");
    for (i = 0; i < DAYS; i++) {
        editday(i);
    }
    savedata();
    showtimetable();
    return 1;
}

int forgot() {
    char ans[50];
    printf("\nSecurity question: %s\n", db.q);
    getstr("Your answer: ", ans, 50);
    lower(ans);
    trim(ans);
    if (hash(ans) != db.ans) {
        printf(RED "Wrong answer\n" RESET);
        return 0;
    }
    printf(GREEN "Correct!\n" RESET);
    if (newpassword(&db.pass)) {
        savedata();
        printf(GREEN "Password changed\n" RESET);
        return 1;
    }
    return 0;
}

int login() {
    char pw[50], yn[10];
    int tries;
    printf("Welcome back %s!\n", db.name);
    for (tries = 0; tries < 3; tries++) {
        getstr("Password: ", pw, 50);
        if (hash(pw) == db.pass) return 1;
        printf(RED "Wrong password\n" RESET);
        getstr("Forgot password? (y/n): ", yn, 10);
        if (yn[0] == 'y' || yn[0] == 'Y') {
            if (forgot()) {
                printf("Now login with your new password\n");
                tries = -1;
            }
        }
    }
    printf(RED "Too many wrong tries\n" RESET);
    return 0;
}

void changepassword() {
    char pw[50];
    getstr("Old password: ", pw, 50);
    if (hash(pw) != db.pass) {
        printf(RED "Wrong password\n" RESET);
        return;
    }
    if (newpassword(&db.pass)) {
        savedata();
        printf(GREEN "Password changed\n" RESET);
    }
}

const char *pcolor(float per) {
    if (per >= 75) return GREEN;
    if (per >= 60) return YELLOW;
    return RED;
}

const char *stcolor(char st) {
    if (st == 'P') return GREEN;
    if (st == 'A') return RED;
    return YELLOW;
}

void printbar(float per, int size) {
    int i, fill = (int)(per / 100 * size);
    printf("%s[", pcolor(per));
    for (i = 0; i < size; i++) {
        putchar(i < fill ? '#' : '-');
    }
    printf("]%s", RESET);
}

void showtasks() {
    int i;
    if (db.ntask == 0) {
        printf("\nNo tasks yet\n");
        return;
    }
    printf("\n" BOLD "%-4s %-6s %-5s %s" RESET "\n", "No", "Done", "Pri", "Task");
    for (i = 0; i < db.ntask; i++) {
        const char *pc = db.tasks[i].pri == 1 ? RED : db.tasks[i].pri == 2 ? YELLOW : GREEN;
        printf("%-4d %s[%c]%s    %s%-5s%s %s%s%s\n", i + 1, db.tasks[i].done ? GREEN : "", db.tasks[i].done ? 'x' : ' ', RESET, pc, pristr[db.tasks[i].pri - 1], RESET, db.tasks[i].done ? DIM : "", db.tasks[i].text, RESET);
    }
}

void addtask() {
    char s[60];
    int p;
    if (db.ntask == MAXTASK) {
        printf(RED "List is full, delete some tasks\n" RESET);
        return;
    }
    getstr("Task: ", s, 60);
    trim(s);
    if (s[0] == 0) {
        printf("Nothing added\n");
        return;
    }
    p = getnum("priority (1 high, 2 medium, 3 low): ");
    if (p < 1 || p > 3) p = 2;
    strcpy(db.tasks[db.ntask].text, s);
    db.tasks[db.ntask].pri = p;
    db.tasks[db.ntask].done = 0;
    db.ntask++;
    savedata();
    printf(GREEN "Task added\n" RESET);
}

void donetask() {
    int n;
    showtasks();
    if (db.ntask == 0) return;
    n = getnum("task number to mark done: ");
    if (n < 1 || n > db.ntask) {
        printf(RED "No such task\n" RESET);
        return;
    }
    db.tasks[n - 1].done = 1;
    savedata();
    printf(GREEN "Nice, done!\n" RESET);
}

void deletetask() {
    int n, i;
    showtasks();
    if (db.ntask == 0) return;
    n = getnum("task number to delete: ");
    if (n < 1 || n > db.ntask) {
        printf(RED "No such task\n" RESET);
        return;
    }
    for (i = n - 1; i < db.ntask - 1; i++) {
        db.tasks[i] = db.tasks[i + 1];
    }
    db.ntask--;
    savedata();
    printf(GREEN "Deleted\n" RESET);
}

void sorttasks() {
    int i, j;
    struct task t;
    for (i = 0; i < db.ntask - 1; i++) {
        for (j = 0; j < db.ntask - 1 - i; j++) {
            if (db.tasks[j].done * 10 + db.tasks[j].pri > db.tasks[j + 1].done * 10 + db.tasks[j + 1].pri) {
                t = db.tasks[j];
                db.tasks[j] = db.tasks[j + 1];
                db.tasks[j + 1] = t;
            }
        }
    }
    savedata();
    printf(GREEN "Sorted\n" RESET);
}

void cleardone() {
    int i, k = 0;
    for (i = 0; i < db.ntask; i++) {
        if (!db.tasks[i].done) {
            db.tasks[k] = db.tasks[i];
            k++;
        }
    }
    printf("%d completed task(s) removed\n", db.ntask - k);
    db.ntask = k;
    savedata();
}

void todomenu() {
    int c;
    for (;;) {
        showtasks();
        printf("\n" CYAN BOLD "--- To do list ---" RESET "\n");
        printf("1 Add task\n2 Mark task done\n3 Delete task\n4 Sort by priority\n5 Remove completed tasks\n6 Back\n");
        c = getnum("choice: ");
        if (c == 1) addtask();
        else if (c == 2) donetask();
        else if (c == 3) deletetask();
        else if (c == 4) sorttasks();
        else if (c == 5) cleardone();
        else if (c == 6) break;
        else printf(RED "Wrong choice\n" RESET);
    }
}

int findrec(int date, int p) {
    int i;
    for (i = 0; i < db.nrec; i++) {
        if (db.recs[i].date == date && db.recs[i].period == p) return i;
    }
    return -1;
}

void markattendance() {
    char b[30], s[10];
    int date, wd, p, r, ct, count = 0;
    char pn[10], tstr[30], st[12], en[12];
    getstr("Date dd-mm-yyyy (enter for today): ", b, 30);
    if (!parsedate(b, &date, &wd)) {
        printf(RED "Invalid date\n" RESET);
        return;
    }
    if (date > todaydate()) {
        printf(RED "You cannot mark attendance for a future date\n" RESET);
        return;
    }
    if (wd == 0 || wd == 6) {
        printf(RED "Saturday and sunday - no classes\n" RESET);
        return;
    }
    printf("\n%s ", dayname[wd - 1]);
    printdate(date);
    printf("\n");
    for (p = 0; p < PERIODS; p++) {
        if (db.tt[wd - 1][p][0] == 0) continue;
        ct = classtype(wd - 1, p);
        if (ct == 2) continue;
        count++;
        r = findrec(date, p);
        if (ct == 1) {
            sprintf(pn, "%d+%d", p + 1, p + 2);
            sscanf(ptime[p], "%[^-]", st);
            strcpy(en, strchr(ptime[p + 1], '-') + 1);
            sprintf(tstr, "%s-%s", st, en);
        } else {
            sprintf(pn, "%d", p + 1);
            strcpy(tstr, ptime[p]);
        }
        printf("Period %-4s %-11s %-20s", pn, tstr, db.tt[wd - 1][p]);
        if (ct == 1) printf(" (combined, counts as 1)");
        if (r != -1) printf(" (marked: %s%c%s)", stcolor(db.recs[r].st), db.recs[r].st, RESET);
        getstr("\n   P present / A absent / C class not held / Enter skip: ", s, 10);
        s[0] = tolower((unsigned char)s[0]);
        if (s[0] != 'p' && s[0] != 'a' && s[0] != 'c') continue;
        if (r == -1) {
            if (db.nrec == MAXREC) {
                printf("Attendance memory full\n");
                return;
            }
            r = db.nrec;
            db.nrec++;
        }
        db.recs[r].date = date;
        db.recs[r].period = p;
        strcpy(db.recs[r].sub, db.tt[wd - 1][p]);
        stripstar(db.recs[r].sub);
        db.recs[r].st = toupper((unsigned char)s[0]);
    }
    if (count == 0) {
        printf("No classes on this day in your timetable\n");
        return;
    }
    savedata();
    printf(GREEN "Attendance saved\n" RESET);
}

void viewdate() {
    char b[30];
    int date, wd, p, r, found = 0;
    getstr("Date dd-mm-yyyy (enter for today): ", b, 30);
    if (!parsedate(b, &date, &wd)) {
        printf(RED "Invalid date\n" RESET);
        return;
    }
    printf("\nAttendance on ");
    printdate(date);
    printf("\n");
    for (p = 0; p < PERIODS; p++) {
        r = findrec(date, p);
        if (r == -1) continue;
        found = 1;
        printf("Period %-2d %-11s %-20s %s%s%s", p + 1, ptime[p], db.recs[r].sub, stcolor(db.recs[r].st), db.recs[r].st == 'P' ? "Present" : db.recs[r].st == 'A' ? "Absent" : "Class not held", RESET);
        if (wd >= 1 && wd <= DAYS && classtype(wd - 1, p) == 1) printf("  (combined, 2 periods)");
        printf("\n");
    }
    if (!found) printf("Nothing marked\n");
}

void summary() {
    char names[MAXSUB][20];
    int att[MAXSUB], tot[MAXSUB];
    int n = 0, i, j, a = 0, t = 0, need, skip;
    float per;
    for (i = 0; i < db.nrec; i++) {
        if (db.recs[i].st == 'C') continue;
        for (j = 0; j < n; j++) {
            if (strcmp(names[j], db.recs[i].sub) == 0) break;
        }
        if (j == n) {
            if (n == MAXSUB) continue;
            strcpy(names[n], db.recs[i].sub);
            att[n] = 0;
            tot[n] = 0;
            n++;
        }
        tot[j]++;
        if (db.recs[i].st == 'P') att[j]++;
    }
    if (n == 0) {
        printf("\nNo attendance marked yet\n");
        return;
    }
    printf("\n" BOLD "%-20s %-9s %-8s %-12s %s" RESET "\n", "Subject", "Attended", "Percent", "Bar", "Status");
    for (i = 0; i < n; i++) {
        per = att[i] * 100.0 / tot[i];
        printf("%-20s %3d/%-5d %s%6.1f%%%s  ", names[i], att[i], tot[i], pcolor(per), per, RESET);
        printbar(per, 10);
        printf("  ");
        if (per >= 75) {
            skip = (4 * att[i] - 3 * tot[i]) / 3;
            printf(GREEN "Safe, can miss %d more class(es)" RESET "\n", skip);
        } else {
            need = 3 * tot[i] - 4 * att[i];
            printf("%s%s, attend next %d class(es) to reach 75%%%s\n", pcolor(per), per >= 60 ? "Low" : "Danger", need, RESET);
        }
        a += att[i];
        t += tot[i];
    }
    per = a * 100.0 / t;
    printf("\nOverall: %d/%d = %s%.1f%%%s\n", a, t, pcolor(per), per, RESET);
    printf(GREEN "Green 75%% and above" RESET "   " YELLOW "Yellow 60%% to 75%%" RESET "   " RED "Red below 60%%" RESET "\n");
}

int addname(char names[][20], int n, char *name) {
    int k;
    char s[20];
    strcpy(s, name);
    stripstar(s);
    for (k = 0; k < n; k++) {
        if (strcmp(names[k], s) == 0) return n;
    }
    if (n < MAXSUB) {
        strcpy(names[n], s);
        n++;
    }
    return n;
}

void subjectstats() {
    char names[MAXSUB][20];
    int keys[MAXREC];
    int n = 0, i, j, c, pr = 0, ab = 0, nh = 0, tot, need, skip, start, t;
    float per;
    for (i = 0; i < DAYS; i++) {
        for (j = 0; j < PERIODS; j++) {
            if (db.tt[i][j][0] != 0) n = addname(names, n, db.tt[i][j]);
        }
    }
    for (i = 0; i < db.nrec; i++) {
        n = addname(names, n, db.recs[i].sub);
    }
    if (n == 0) {
        printf("\nNo subjects yet\n");
        return;
    }
    printf("\n");
    for (i = 0; i < n; i++) {
        printf("%d %s\n", i + 1, names[i]);
    }
    c = getnum("which subject: ");
    if (c < 1 || c > n) {
        printf(RED "Wrong choice\n" RESET);
        return;
    }
    c--;
    n = 0;
    for (i = 0; i < db.nrec; i++) {
        if (strcmp(db.recs[i].sub, names[c]) != 0) continue;
        if (db.recs[i].st == 'P') pr++;
        else if (db.recs[i].st == 'A') {
            ab++;
            keys[n] = db.recs[i].date * 100 + db.recs[i].period;
            n++;
        } else nh++;
    }
    tot = pr + ab;
    printf("\n" BOLD CYAN "%s" RESET "\n", names[c]);
    printf("Classes held    : %d\n", tot);
    printf("Present         : " GREEN "%d" RESET "\n", pr);
    printf("Absent          : " RED "%d" RESET "\n", ab);
    printf("Class not held  : " YELLOW "%d" RESET "\n", nh);
    if (tot == 0) {
        printf("Nothing marked for this subject yet\n");
        return;
    }
    per = pr * 100.0 / tot;
    printf("Attendance      : %s%.1f%%%s  ", pcolor(per), per, RESET);
    printbar(per, 20);
    printf("\n");
    if (per >= 75) {
        skip = (4 * pr - 3 * tot) / 3;
        printf(GREEN "Safe, you can miss %d more class(es) and stay above 75%%" RESET "\n", skip);
    } else {
        need = 3 * tot - 4 * pr;
        printf("%sAttend the next %d class(es) in a row to reach 75%%%s\n", pcolor(per), need, RESET);
    }
    if (n > 0) {
        for (i = 0; i < n - 1; i++) {
            for (j = 0; j < n - 1 - i; j++) {
                if (keys[j] > keys[j + 1]) {
                    t = keys[j];
                    keys[j] = keys[j + 1];
                    keys[j + 1] = t;
                }
            }
        }
        start = n > 10 ? n - 10 : 0;
        printf("\nAbsent on");
        if (start > 0) printf(" (latest 10)");
        printf(":\n");
        for (i = start; i < n; i++) {
            printf("   " RED);
            printdate(keys[i] / 100);
            printf(RESET "  Period %d\n", keys[i] % 100 + 1);
        }
    }
}

void attmenu() {
    int c;
    for (;;) {
        printf("\n" CYAN BOLD "--- Attendance marker ---" RESET "\n");
        printf("1 Mark attendance\n2 See a day's attendance\n3 All subjects summary\n4 Subject statistics\n5 Back\n");
        c = getnum("choice: ");
        if (c == 1) markattendance();
        else if (c == 2) viewdate();
        else if (c == 3) summary();
        else if (c == 4) subjectstats();
        else if (c == 5) break;
        else printf(RED "Wrong choice\n" RESET);
    }
}

void ttmenu() {
    int c, i, dd, p;
    for (;;) {
        showtimetable();
        printf("\n" CYAN BOLD "--- Timetable ---" RESET "\n");
        printf("1 Change one period\n2 Change a whole day\n3 Change all days\n4 Back\n");
        c = getnum("choice: ");
        if (c == 1) {
            for (i = 0; i < DAYS; i++) {
                printf("%d %s\n", i + 1, dayname[i]);
            }
            dd = getnum("which day: ");
            if (dd < 1 || dd > DAYS) {
                printf(RED "Wrong choice\n" RESET);
                continue;
            }
            dd--;
            printf("\n%s\n", dayname[dd]);
            for (i = 0; i < PERIODS; i++) {
                if (i == 6) printf("      ---- lunch 1:10 to 1:40 ----\n");
                printf("Period %-2d %-11s %s\n", i + 1, ptime[i], db.tt[dd][i][0] ? db.tt[dd][i] : "-");
            }
            p = getnum("which period (1-10): ");
            if (p < 1 || p > PERIODS) {
                printf(RED "Wrong choice\n" RESET);
                continue;
            }
            printf("(leave blank for no class, type . to keep the old one)\n");
            setperiod(dd, p - 1);
            checkstars(dd);
            savedata();
            printf(GREEN "Period changed\n" RESET);
        } else if (c == 2) {
            for (i = 0; i < DAYS; i++) {
                printf("%d %s\n", i + 1, dayname[i]);
            }
            dd = getnum("which day: ");
            if (dd >= 1 && dd <= DAYS) {
                editday(dd - 1);
                savedata();
            } else {
                printf(RED "Wrong choice\n" RESET);
            }
        } else if (c == 3) {
            for (i = 0; i < DAYS; i++) editday(i);
            savedata();
        } else if (c == 4) {
            break;
        } else {
            printf(RED "Wrong choice\n" RESET);
        }
    }
}

void usermenu() {
    int c;
    for (;;) {
        printf("\n" CYAN BOLD "=== %s's TermTracker ===" RESET "\n", db.name);
        printf("1 To do list\n2 Attendance marker\n3 Timetable\n4 Change password\n5 Exit\n");
        c = getnum("choice: ");
        if (c == 1) todomenu();
        else if (c == 2) attmenu();
        else if (c == 3) ttmenu();
        else if (c == 4) changepassword();
        else if (c == 5) {
            printf("Bye %s\n", db.name);
            break;
        } else {
            printf(RED "Wrong choice\n" RESET);
        }
    }
}

int main() {
    int r;
    printf(CYAN BOLD "=== TERMTRACKER ===" RESET "\n");
    r = loaddata();
    if (r == -1) {
        printf("\n%s is damaged. Rename or delete it and run again.\n", filename);
        return 1;
    }
    if (r == 0) {
        printf("No account found, let's make one.\n\n");
        if (!createaccount()) return 1;
    } else {
        if (!login()) return 1;
    }
    usermenu();
    return 0;
}