#include <stdio.h>
#include <string.h>
#include <stdbool.h> 
#define MAX_REPORTS 20
#define MAX_EMPLOYEES 100

struct Employee {
    int id;
    char name[50];
    char desig[50];
    struct Employee* manager;
    struct Employee* reports[MAX_REPORTS];
    int report_count;
};

// global registry of employees
struct Employee registry[MAX_EMPLOYEES];
int total_registered = 0;
// bool to check if ceo exists already
bool has_ceo = false;

struct Employee* findEmployee(int id) {
    if (id == 0) {
        return NULL;
    }
    for (int i = 0; i < total_registered; i++) {
        if (registry[i].id == id) {
            return &registry[i];
        }
    }
    return NULL;
}
// check empty space in employee arrat
struct Employee* getFreeSlot() {
    for (int i = 0; i < total_registered; i++) {
        if (registry[i].id == 0) {
            return &registry[i];
        }
    }
    if (total_registered < MAX_EMPLOYEES) {
        int index = total_registered;
        total_registered++;
        return &registry[index];
    }
    return NULL;
}

void addEmployee() {
    int id, mgr_id;
    char name[50], desig[50];
    printf("Enter Employee ID: ");
    scanf("%d", &id);
    if (id == 0) {
        printf("Employee ID can't be 0\n");
        return;
    }
    if (findEmployee(id) != NULL) {
        printf("Employee already exists\n");
        return;
    }
    printf("Enter Name: ");
    scanf(" %s", name);
    printf("Enter Designation: ");
    scanf(" %s", desig);
    printf("Enter Reporting Manager ID (0 for CEO): ");
    scanf("%d", &mgr_id);
    // ceo check
    if (mgr_id == 0) {
        if (has_ceo == true) {
            printf("Only one CEO is allowed\n");
            return;
        }
    }
    if(mgr_id==id){
        printf("Employee cant report to themselves");
        return;
    }
    struct Employee* newEmp = getFreeSlot();
    if (newEmp == NULL) {
        printf("Max employees reached\n");
        return;
    }
    newEmp->id = id;
    strcpy(newEmp->name, name);
    strcpy(newEmp->desig, desig);
    newEmp->report_count = 0;
    newEmp->manager = NULL;
    if (mgr_id != 0) {
        struct Employee* mgr = findEmployee(mgr_id);
        if (mgr != NULL) {
                newEmp->manager = mgr;
                mgr->reports[mgr->report_count] = newEmp;
                mgr->report_count++;
        } else {
            printf("Reporting Manager does not exists\n");
            newEmp->id = 0;
            return;
        }
    } else {
        has_ceo = true;
    }
    printf("Employee added\n");
}

void removeEmployee() {
    int id;
    printf("Enter Employee ID to remove: ");
    scanf("%d", &id);
    struct Employee* emp = findEmployee(id);
    if (emp == NULL) {
        printf("Employee does not exists\n");
        return;
    }
    // ceo removal
    if (emp->manager == NULL) {
        printf("CEO can't be removed\n");
        return;
    }
    struct Employee* mgr = emp->manager;
    struct Employee* new_manager = NULL;
    int min_reports = MAX_REPORTS + 1;
    for (int i = 0; i < mgr->report_count; i++) {
        struct Employee* sibling = mgr->reports[i];
        if (sibling != emp) {
            if (sibling->report_count < min_reports) {
                min_reports = sibling->report_count;
                new_manager = sibling;
            }
        }
    }
    if (new_manager == NULL) {
        new_manager = mgr;
    }
    for (int i = 0; i < emp->report_count; i++) {
        struct Employee* sub = emp->reports[i];
        sub->manager = new_manager;
        new_manager->reports[new_manager->report_count] = sub;
        new_manager->report_count++;
    }
    //delete employee from manager reports
    int index = -1;
    for (int i = 0; i < mgr->report_count; i++) {
        if (mgr->reports[i]->id == id) {
            index = i;
            break;
        }
    }
    for (int i = index; i < mgr->report_count - 1; i++) {
            mgr->reports[i] = mgr->reports[i + 1];
    }
    mgr->report_count--;
    // i have set employee's id=0 to mark employee as deleted in registry arrat
    emp->id = 0;
    printf("Employee removed. their direct reports transferred to %s.\n", new_manager->name);
}

void printHierarchy(struct Employee* emp, int spaces) {
    if (emp == NULL) {
        return;
    }
    for(int i=0; i<spaces; i++) {
        printf("\t");
    }
    printf("%d - %s - %s\n", emp->id, emp->name, emp->desig);
    for (int i = 0; i < emp->report_count; i++) {
        printHierarchy(emp->reports[i], spaces+1);
    }
}

int countEmployees(struct Employee* emp) {
    if (emp == NULL) {
        return 0;
    }
    int count = 1;
    for (int i = 0; i < emp->report_count; i++) {
        count += countEmployees(emp->reports[i]);
    }
    return count;
}

void displayEmployeeHierarchy() {
    int id;
    printf("Enter Employee ID: ");
    scanf("%d", &id);
    struct Employee* emp = findEmployee(id);
    if (emp == NULL) {
        printf("Employee does not exists\n");
        return;
    }
    printHierarchy(emp, 0);
    int total = countEmployees(emp) - 1; 
    printf("Employees reporting to %d: %d\n", id, total);
}

void displayCompleteHierarchy() {
    printf("#############################\n");
    int total_in_org = 0;
    for (int i = 0; i < total_registered; i++) {
        if (registry[i].id != 0 && registry[i].manager == NULL) {
            printHierarchy(&registry[i], 0);
            total_in_org += countEmployees(&registry[i]);
        }
    }
    printf("Total Employees: %d\n", total_in_org);
}

int main() {
    int choice;
    while (1) {
        printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: {
                addEmployee();
                break;
            }
            case 2: {
                removeEmployee();
                break;
            }
            case 3: {
                displayEmployeeHierarchy();
                break;
            }
            case 4: {
                displayCompleteHierarchy();
                break;
            }
            case 5: {
                return 0;
            }
            default: {
                printf("Invalid choice, try again.\n");
            }
        }
    }
    return 0;
}