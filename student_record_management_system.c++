#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

#define FILENAME "students_sll_v2.csv"
#define EXPORT_FILENAME "student_records_export.csv"

// Case-insensitive string helper
int compareIgnoreCase(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2))
            return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        s1++; s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

// ==========================================
// 1. DATA STRUCTURE: SINGLY LINKED LIST
// ==========================================
typedef struct StudentNode {
    int rollNo;
    char name[30];
    char dept[20];
    int semester;
    float gpa;
    char status[10]; // "PASS" or "FAIL"
    struct StudentNode *next;
} StudentNode;

StudentNode* createNode(int rollNo, const char* name, const char* dept, int semester, float gpa) {
    StudentNode* newNode = (StudentNode*)malloc(sizeof(StudentNode));
    if (!newNode) return NULL;
    newNode->rollNo = rollNo;
    strncpy(newNode->name, name, 29);   newNode->name[29] = '\0';
    strncpy(newNode->dept, dept, 19);   newNode->dept[19] = '\0';
    newNode->semester = semester;
    newNode->gpa = gpa;
    strcpy(newNode->status, (gpa >= 4.0f) ? "PASS" : "FAIL");
    newNode->next = NULL;
    return newNode;
}

// Sorted insertion by Roll Number O(N)
StudentNode* insertSorted(StudentNode* head, int rollNo, const char* name, const char* dept, int semester, float gpa) {
    StudentNode* newNode = createNode(rollNo, name, dept, semester, gpa);

    if (head == NULL || rollNo < head->rollNo) {
        newNode->next = head;
        return newNode;
    }

    if (head->rollNo == rollNo) {
        strncpy(head->name, name, 29);
        strncpy(head->dept, dept, 19);
        head->semester = semester;
        head->gpa = gpa;
        strcpy(head->status, (gpa >= 4.0f) ? "PASS" : "FAIL");
        free(newNode);
        return head;
    }

    StudentNode* current = head;
    while (current->next != NULL && current->next->rollNo < rollNo) {
        current = current->next;
    }

    if (current->next != NULL && current->next->rollNo == rollNo) {
        strncpy(current->next->name, name, 29);
        strncpy(current->next->dept, dept, 19);
        current->next->semester = semester;
        current->next->gpa = gpa;
        strcpy(current->next->status, (gpa >= 4.0f) ? "PASS" : "FAIL");
        free(newNode);
        return head;
    }

    newNode->next = current->next;
    current->next = newNode;
    return head;
}

StudentNode* searchByRoll(StudentNode* head, int rollNo) {
    StudentNode* curr = head;
    while (curr != NULL) {
        if (curr->rollNo == rollNo) return curr;
        curr = curr->next;
    }
    return NULL;
}

StudentNode* searchByName(StudentNode* head, const char* queryName) {
    StudentNode* curr = head;
    while (curr != NULL) {
        if (compareIgnoreCase(curr->name, queryName) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

StudentNode* deleteSLL(StudentNode* head, int rollNo, int *found) {
    *found = 0;
    if (head == NULL) return NULL;

    if (head->rollNo == rollNo) {
        StudentNode* temp = head->next;
        free(head);
        *found = 1;
        return temp;
    }

    StudentNode* current = head;
    while (current->next != NULL && current->next->rollNo != rollNo) {
        current = current->next;
    }

    if (current->next != NULL) {
        StudentNode* temp = current->next;
        current->next = temp->next;
        free(temp);
        *found = 1;
    }
    return head;
}

int updateRecord(StudentNode* head, int rollNo, const char* name, const char* dept, int semester, float gpa) {
    StudentNode* target = searchByRoll(head, rollNo);
    if (target == NULL) return 0;

    if (strlen(name) > 0) { strncpy(target->name, name, 29); target->name[29] = '\0'; }
    if (strlen(dept) > 0) { strncpy(target->dept, dept, 19); target->dept[19] = '\0'; }
    if (semester > 0) target->semester = semester;
    if (gpa >= 0.0f) {
        target->gpa = gpa;
        strcpy(target->status, (gpa >= 4.0f) ? "PASS" : "FAIL");
    }
    return 1;
}

void getAnalytics(StudentNode* head, int *total, float *avgGPA, StudentNode **topper, int *passCount, int *failCount) {
    *total = 0;
    *avgGPA = 0.0f;
    *topper = NULL;
    *passCount = 0;
    *failCount = 0;

    if (head == NULL) return;

    float sumGPA = 0.0f;
    StudentNode* curr = head;
    *topper = head;

    while (curr != NULL) {
        (*total)++;
        sumGPA += curr->gpa;
        if (curr->gpa > (*topper)->gpa) *topper = curr;
        if (curr->gpa >= 4.0f) (*passCount)++;
        else (*failCount)++;
        curr = curr->next;
    }

    if (*total > 0) *avgGPA = sumGPA / (*total);
}

void freeList(StudentNode* head) {
    StudentNode* temp;
    while (head != NULL) {
        temp = head; head = head->next; free(temp);
    }
}

// ==========================================
// 2. PERSISTENCE ENGINE (CSV & EXCEL EXPORT)
// ==========================================
void saveToCSV(StudentNode* head) {
    FILE* fp = fopen(FILENAME, "w");
    if (fp != NULL) {
        StudentNode* curr = head;
        while (curr != NULL) {
            fprintf(fp, "%d,%s,%s,%d,%.2f,%s\n", curr->rollNo, curr->name, curr->dept, curr->semester, curr->gpa, curr->status);
            curr = curr->next;
        }
        fclose(fp);
    }
}

StudentNode* loadFromCSV() {
    StudentNode* head = NULL;
    FILE* fp = fopen(FILENAME, "r");
    if (fp == NULL) return NULL;

    char line[150];
    while (fgets(line, sizeof(line), fp)) {
        int rollNo = 0, semester = 1;
        char name[30] = "", dept[20] = "";
        float gpa = 0.0f;

        char* token = strtok(line, ",\n");
        if (token) rollNo = atoi(token);
        token = strtok(NULL, ",\n");
        if (token) strncpy(name, token, 29);
        token = strtok(NULL, ",\n");
        if (token) strncpy(dept, token, 19);
        token = strtok(NULL, ",\n");
        if (token) semester = atoi(token);
        token = strtok(NULL, ",\n");
        if (token) gpa = (float)atof(token);

        if (rollNo > 0 && strlen(name) > 0) {
            head = insertSorted(head, rollNo, name, dept, semester, gpa);
        }
    }
    fclose(fp);
    return head;
}

int exportToExcel(StudentNode* head) {
    FILE* fp = fopen(EXPORT_FILENAME, "w");
    if (fp == NULL) return 0;
    fprintf(fp, "Roll No,Student Name,Department,Semester,GPA,Status\n");
    StudentNode* curr = head;
    while (curr != NULL) {
        fprintf(fp, "\"%d\",\"%s\",\"%s\",\"Sem-%d\",\"%.2f\",\"%s\"\n",
                curr->rollNo, curr->name, curr->dept, curr->semester, curr->gpa, curr->status);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

// ==========================================
// 3. CYBER SAPPHIRE GRAPHICS FRONTEND
// ==========================================
void drawArrow(int x1, int y1, int x2, int y2) {
    setcolor(CYAN);
    setlinestyle(SOLID_LINE, 0, 2);
    line(x1, y1, x2, y2);
    line(x2, y2, x2 - 6, y2 - 4);
    line(x2, y2, x2 - 6, y2 + 4);
}

void drawLinkedList(StudentNode* head, int highlightRoll) {
    if (head == NULL) {
        setbkcolor(BLACK);
        setcolor(LIGHTGRAY);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
        outtextxy(200, 260, (char*)"[ Singly Linked List Empty ]");
        return;
    }

    // HEAD Pointer Badge
    setfillstyle(SOLID_FILL, RED);
    bar(25, 125, 75, 150);
    setbkcolor(RED);
    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(32, 133, (char*)"HEAD");

    drawArrow(75, 137, 95, 137);

    StudentNode* curr = head;
    int x = 95, y = 120;
    int col = 0, row = 0;

    while (curr != NULL && row < 5) {
        int nodeX1 = x + col * 155;
        int nodeY1 = y + row * 80;
        int nodeX2 = nodeX1 + 105;
        int nodeY2 = nodeY1 + 48;

        // Node Drop Shadow
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(nodeX1 + 3, nodeY1 + 3, nodeX2 + 3, nodeY2 + 3);

        int bgFill = BLUE;
        int fgText = WHITE;

        if (highlightRoll > 0 && curr->rollNo == highlightRoll) {
            bgFill = RED; fgText = WHITE;
        } else if (curr == head) {
            bgFill = CYAN; fgText = BLACK;
        }

        setfillstyle(SOLID_FILL, bgFill);
        bar(nodeX1, nodeY1, nodeX2, nodeY2);
        setcolor(WHITE);
        setlinestyle(SOLID_LINE, 0, 1);
        rectangle(nodeX1, nodeY1, nodeX2, nodeY2);

        setbkcolor(bgFill);
        setcolor(fgText);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);

        char rStr[15], nStr[15], gStr[15];
        sprintf(rStr, "#%d", curr->rollNo);
        strncpy(nStr, curr->name, 8); nStr[8] = '\0';
        sprintf(gStr, "%.2f | %s", curr->gpa, curr->status);

        outtextxy(nodeX1 + 6, nodeY1 + 6, rStr);
        outtextxy(nodeX1 + 6, nodeY1 + 20, nStr);
        
        setcolor(curr->gpa >= 4.0f ? LIGHTGREEN : RED);
        if (bgFill == CYAN) setcolor(BLACK);
        outtextxy(nodeX1 + 6, nodeY1 + 34, gStr);

        // Next Pointer Arrow Drawing
        if (curr->next != NULL) {
            if (col < 2) {
                drawArrow(nodeX2, nodeY1 + 24, nodeX2 + 50, nodeY1 + 24);
            } else {
                setcolor(CYAN);
                line(nodeX2, nodeY1 + 24, nodeX2 + 15, nodeY1 + 24);
                line(nodeX2 + 15, nodeY1 + 24, nodeX2 + 15, nodeY1 + 64);
                line(nodeX2 + 15, nodeY1 + 64, x, nodeY1 + 64);
                drawArrow(x, nodeY1 + 64, x, nodeY1 + 80);
            }
        } else {
            // NULL Terminal Box
            setfillstyle(SOLID_FILL, DARKGRAY);
            bar(nodeX2 + 10, nodeY1 + 14, nodeX2 + 45, nodeY1 + 34);
            setcolor(YELLOW);
            rectangle(nodeX2 + 10, nodeY1 + 14, nodeX2 + 45, nodeY1 + 34);
            setbkcolor(DARKGRAY);
            outtextxy(nodeX2 + 14, nodeY1 + 20, (char*)"NULL");
            drawArrow(nodeX2, nodeY1 + 24, nodeX2 + 10, nodeY1 + 24);
        }

        col++;
        if (col >= 3) { col = 0; row++; }
        curr = curr->next;
    }
}

void displaySortedList(StudentNode* head, int *yOffset, int *index) {
    StudentNode* curr = head;
    while (curr != NULL && *index <= 13) {
        setfillstyle(SOLID_FILL, DARKGRAY);
        bar(605, *yOffset - 3, 905, *yOffset + 24);
        setcolor(CYAN);
        rectangle(605, *yOffset - 3, 905, *yOffset + 24);

        char entry[90];
        sprintf(entry, "%2d. #%-4d | %-7.7s | S%d | %.2f", (*index)++, curr->rollNo, curr->name, curr->semester, curr->gpa);

        setbkcolor(DARKGRAY);
        setcolor(YELLOW);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        outtextxy(612, *yOffset + 4, entry);
        *yOffset += 28;

        curr = curr->next;
    }
}

void renderUI(StudentNode* head, const char* statusMsg, int isSuccess, int highlightRoll) {
    // 1. Header Banner
    setfillstyle(SOLID_FILL, BLUE);
    bar(0, 0, 920, 50);
    setbkcolor(BLUE);
    setcolor(YELLOW);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3);
    outtextxy(180, 8, (char*)"STUDENT RECORD SYSTEM (SLL)");

    // 2. Real-time Analytics Dashboard Badges
    int total = 0, passCount = 0, failCount = 0;
    float avgGPA = 0.0f;
    StudentNode* topper = NULL;
    getAnalytics(head, &total, &avgGPA, &topper, &passCount, &failCount);

    char b1[30], b2[30], b3[30];
    sprintf(b1, " Students: %d ", total);
    sprintf(b2, " Class Avg: %.2f ", avgGPA);
    sprintf(b3, " Pass: %d | Fail: %d ", passCount, failCount);

    setfillstyle(SOLID_FILL, CYAN);
    bar(15, 60, 145, 86);
    setbkcolor(CYAN); setcolor(BLACK);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(22, 68, b1);

    setfillstyle(SOLID_FILL, GREEN);
    bar(155, 60, 295, 86);
    setbkcolor(GREEN); setcolor(BLACK);
    outtextxy(162, 68, b2);

    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(305, 60, 475, 86);
    setbkcolor(DARKGRAY); setcolor(WHITE);
    outtextxy(312, 68, b3);

    // 3. Left Panel (Linked Canvas)
    setcolor(CYAN);
    setlinestyle(SOLID_LINE, 0, 2);
    rectangle(10, 95, 590, 520);

    drawLinkedList(head, highlightRoll);

    // 4. Right Panel (Directory Listing)
    setcolor(LIGHTGREEN);
    rectangle(600, 95, 910, 520);

    setfillstyle(SOLID_FILL, GREEN);
    bar(601, 96, 909, 126);
    setbkcolor(GREEN); setcolor(BLACK);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
    outtextxy(645, 102, (char*)"LINKED DIRECTORY");

    int yOffset = 138;
    int index = 1;
    displaySortedList(head, &yOffset, &index);

    // 5. Action Controls (7 Buttons Layout)
    int btnX1[] = {10, 140, 270, 400, 530, 660, 790};
    int btnX2[] = {130, 260, 390, 520, 650, 780, 910};
    const char* btnLabels[] = {"+ ADD", "? SEARCH", "- DELETE", "~ EDIT", "* STATS", "^ EXPORT", "x EXIT"};

    for (int i = 0; i < 7; i++) {
        setfillstyle(SOLID_FILL, BLUE);
        bar(btnX1[i], 532, btnX2[i], 572);
        setcolor(CYAN);
        setlinestyle(SOLID_LINE, 0, 1);
        rectangle(btnX1[i], 532, btnX2[i], 572);

        setbkcolor(BLUE); setcolor(WHITE);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        outtextxy(btnX1[i] + 12, 547, (char*)btnLabels[i]);
    }

    // 6. Dynamic Alert Banner
    if (strlen(statusMsg) > 0) {
        int bannerColor = isSuccess ? GREEN : RED;
        setfillstyle(SOLID_FILL, bannerColor);
        bar(10, 582, 910, 622);
        setcolor(WHITE);
        rectangle(10, 582, 910, 622);

        setbkcolor(bannerColor);
        setcolor(isSuccess ? BLACK : WHITE);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        outtextxy(25, 597, (char*)statusMsg);
    }
}

// ==========================================
// 4. IN-WINDOW GRAPHICAL MODAL INPUT
// ==========================================
void getGUIInput(const char* title, const char* prompt, char* outputBuffer, int maxLen) {
    outputBuffer[0] = '\0';
    int idx = 0;

    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(200, 200, 720, 390);
    setcolor(CYAN);
    setlinestyle(SOLID_LINE, 0, 2);
    rectangle(200, 200, 720, 390);

    setbkcolor(DARKGRAY);
    setcolor(YELLOW);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    outtextxy(220, 215, (char*)title);

    setcolor(WHITE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(220, 265, (char*)prompt);

    while (1) {
        setfillstyle(SOLID_FILL, BLACK);
        bar(220, 295, 700, 335);
        setcolor(CYAN);
        setlinestyle(SOLID_LINE, 0, 1);
        rectangle(220, 295, 700, 335);

        setbkcolor(BLACK);
        setcolor(GREEN);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
        outtextxy(230, 308, outputBuffer);

        char ch = getch();
        if (ch == 13) break;
        else if (ch == 8) {
            if (idx > 0) {
                idx--;
                outputBuffer[idx] = '\0';
            }
        } else if (isprint(ch) && idx < maxLen - 1) {
            outputBuffer[idx++] = ch;
            outputBuffer[idx] = '\0';
        }
    }
}

// ==========================================
// 5. MAIN PROGRAM LOOP
// ==========================================
int main() {
    initwindow(920, 640, (char*)"Student Record Management Engine - Singly Linked List");

    StudentNode* head = loadFromCSV();
    if (head == NULL) {
        head = insertSorted(head, 101, "Ayush", "AI & DS", 3, 8.9f);
        head = insertSorted(head, 102, "Shambhu", "AI & DS", 3, 9.4f);
        head = insertSorted(head, 103, "Prajwal", "AI & DS", 3, 8.7f);
        head = insertSorted(head, 105, "Pratik", "AI & DS", 3, 9.1f);
        saveToCSV(head);
    }

    char statusMsg[120] = "System Ready. Use hotkeys (1-7) or click buttons.";
    int highlightRoll = -1;
    int isSuccess = 1;
    int page = 0;

    while (1) {
        setactivepage(page);
        cleardevice();

        renderUI(head, statusMsg, isSuccess, highlightRoll);

        setvisualpage(page);

        int action = 0; // 1:ADD, 2:SEARCH, 3:DELETE, 4:EDIT, 5:STATS, 6:EXPORT, 7:EXIT

        if (ismouseclick(WM_LBUTTONDOWN)) {
            int mx, my;
            getmouseclick(WM_LBUTTONDOWN, mx, my);

            if (my >= 532 && my <= 572) {
                if (mx >= 10 && mx <= 130)       action = 1;
                else if (mx >= 140 && mx <= 260) action = 2;
                else if (mx >= 270 && mx <= 390) action = 3;
                else if (mx >= 400 && mx <= 520) action = 4;
                else if (mx >= 530 && mx <= 650) action = 5;
                else if (mx >= 660 && mx <= 780) action = 6;
                else if (mx >= 790 && mx <= 910) action = 7;
            }
        }

        if (kbhit()) {
            char choice = getch();
            if (choice == '1') action = 1;
            else if (choice == '2') action = 2;
            else if (choice == '3') action = 3;
            else if (choice == '4') action = 4;
            else if (choice == '5') action = 5;
            else if (choice == '6' || choice == 'e' || choice == 'E') action = 6;
            else if (choice == '7' || choice == 'q' || choice == 'Q') action = 7;
        }

        if (action == 1) { // ADD
            char rStr[15], name[30], dept[20], semStr[10], gStr[15];
            getGUIInput("ADD STUDENT RECORD", "Enter Roll Number:", rStr, 10);
            if (strlen(rStr) > 0) {
                int rollNo = atoi(rStr);
                getGUIInput("ADD STUDENT RECORD", "Enter Student Name:", name, 28);
                getGUIInput("ADD STUDENT RECORD", "Enter Department:", dept, 18);
                getGUIInput("ADD STUDENT RECORD", "Enter Semester (1-8):", semStr, 5);
                getGUIInput("ADD STUDENT RECORD", "Enter GPA (0.0 - 10.0):", gStr, 10);

                int semester = atoi(semStr);
                float gpa = (float)atof(gStr);
                head = insertSorted(head, rollNo, name, strlen(dept) > 0 ? dept : "AI & DS", semester > 0 ? semester : 1, gpa);
                saveToCSV(head);
                sprintf(statusMsg, "SUCCESS: Roll #%d (%s) inserted into SLL!", rollNo, name);
                isSuccess = 1;
            }
            highlightRoll = -1;
        }
        else if (action == 2) { // SEARCH
            char query[30];
            getGUIInput("SEARCH STUDENT", "Enter Roll No or Name to Search:", query, 28);
            if (strlen(query) > 0) {
                StudentNode* res = NULL;
                if (isdigit(query[0])) {
                    res = searchByRoll(head, atoi(query));
                } else {
                    res = searchByName(head, query);
                }

                if (res != NULL) {
                    sprintf(statusMsg, "FOUND: Roll #%d - %s [%s] Sem-%d | GPA: %.2f (%s)",
                            res->rollNo, res->name, res->dept, res->semester, res->gpa, res->status);
                    highlightRoll = res->rollNo;
                    isSuccess = 1;
                } else {
                    sprintf(statusMsg, "ERROR: Student '%s' was not found in SLL.", query);
                    highlightRoll = -1;
                    isSuccess = 0;
                }
            }
        }
        else if (action == 3) { // DELETE
            char rStr[15];
            int found = 0;
            getGUIInput("DELETE STUDENT", "Enter Roll Number to Delete:", rStr, 10);
            if (strlen(rStr) > 0) {
                int rollNo = atoi(rStr);
                head = deleteSLL(head, rollNo, &found);
                if (found) {
                    saveToCSV(head);
                    sprintf(statusMsg, "SUCCESS: Roll #%d deleted & pointers re-linked!", rollNo);
                    isSuccess = 1;
                } else {
                    sprintf(statusMsg, "ERROR: Student Roll #%d not found.", rollNo);
                    isSuccess = 0;
                }
            }
            highlightRoll = -1;
        }
        else if (action == 4) { // EDIT
            char rStr[15];
            getGUIInput("UPDATE STUDENT", "Enter Roll Number to Edit:", rStr, 10);
            if (strlen(rStr) > 0) {
                int rollNo = atoi(rStr);
                StudentNode* res = searchByRoll(head, rollNo);
                if (res != NULL) {
                    char name[30], dept[20], semStr[10], gStr[15];
                    getGUIInput("UPDATE STUDENT", "Enter New Name (leave blank to skip):", name, 28);
                    getGUIInput("UPDATE STUDENT", "Enter New Dept (leave blank to skip):", dept, 18);
                    getGUIInput("UPDATE STUDENT", "Enter New Semester (0 to skip):", semStr, 5);
                    getGUIInput("UPDATE STUDENT", "Enter New GPA (-1 to skip):", gStr, 10);

                    updateRecord(head, rollNo, name, dept, atoi(semStr), (float)atof(gStr));
                    saveToCSV(head);
                    sprintf(statusMsg, "SUCCESS: Roll #%d record updated successfully!", rollNo);
                    highlightRoll = rollNo;
                    isSuccess = 1;
                } else {
                    sprintf(statusMsg, "ERROR: Roll #%d not found to edit.", rollNo);
                    isSuccess = 0;
                }
            }
        }
        else if (action == 5) { // STATS
            int total = 0, passCount = 0, failCount = 0;
            float avgGPA = 0.0f;
            StudentNode* topper = NULL;
            getAnalytics(head, &total, &avgGPA, &topper, &passCount, &failCount);

            if (topper != NULL) {
                sprintf(statusMsg, "TOPPER: Roll #%d %s (GPA: %.2f) | Class Avg: %.2f | Pass: %d",
                        topper->rollNo, topper->name, topper->gpa, avgGPA, passCount);
                highlightRoll = topper->rollNo;
                isSuccess = 1;
            } else {
                sprintf(statusMsg, "ERROR: No student records in list.");
                isSuccess = 0;
            }
        }
        else if (action == 6) { // EXPORT
            if (exportToExcel(head)) {
                sprintf(statusMsg, "EXCEL EXPORT SUCCESS: Saved to '%s'", EXPORT_FILENAME);
                isSuccess = 1;
            } else {
                sprintf(statusMsg, "EXCEL EXPORT ERROR: Failed to write file.");
                isSuccess = 0;
            }
            highlightRoll = -1;
        }
        else if (action == 7) { // EXIT
            saveToCSV(head);
            break;
        }

        page = 1 - page;
        delay(60);
    }

    freeList(head);
    closegraph();
    return 0;
}