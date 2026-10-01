#include <stdio.h>
#include <stdbool.h>

struct User {
    int id;
    char name[100];
    int age;
};

void create() {
    struct User u;
    struct User temp;
    bool duplicate = false;
    
    printf("Enter ID: ");
    scanf("%d", &u.id);
    
    FILE *file = fopen("users.txt", "r");
    if (file != NULL) {
        while (fscanf(file, "%d,%[^,],%d", &temp.id, temp.name, &temp.age) == 3) {
            if (temp.id == u.id) {
                duplicate = true;
                break;
            }
        }
        fclose(file);
    }
    
    if (duplicate) {
        printf("Error: User with this ID already exists.\n");
        return;
    }
    
    file = fopen("users.txt", "a");
    if (file != NULL) {
        printf("Enter fullname: ");
        scanf(" %[^\n]", u.name);
        
        printf("Enter age: ");
        scanf("%d", &u.age);
        
        fprintf(file, "%d,%s,%d\n", u.id, u.name, u.age);
        fclose(file);
        printf("User created successfully.\n");
    }
}

void read() {
    struct User u;
    FILE *file = fopen("users.txt", "r");
    
    if (file != NULL) {
        while (fscanf(file, "%d,%[^,],%d", &u.id, u.name, &u.age) == 3) {
            printf("ID: %d, Name: %s, Age: %d\n", u.id, u.name, u.age);
        }
        fclose(file);
    } else {
        printf("No users found.\n");
    }
}

void update() {
    int id;
    bool found = false;
    struct User u;
    
    printf("Enter ID to update: ");
    scanf("%d", &id);
    
    FILE *file = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    
    if (file != NULL && temp != NULL) {
        while (fscanf(file, "%d,%[^,],%d", &u.id, u.name, &u.age) == 3) {
            if (u.id == id) {
                printf("Enter new fullname: ");
                scanf(" %[^\n]", u.name);
                
                printf("Enter new age: ");
                scanf("%d", &u.age);
                found = true;
            }
            fprintf(temp, "%d,%s,%d\n", u.id, u.name, u.age);
        }
        fclose(file);
        fclose(temp);
        remove("users.txt");
        rename("temp.txt", "users.txt");
        
        if (!found) {
            printf("ID not found.\n");
        } else {
            printf("User updated.\n");
        }
    } else {
        if (file != NULL) {
            fclose(file);
        }
    }
}

void delete() {
    int id;
    bool found = false;
    struct User u;
    
    printf("Enter ID to delete: ");
    scanf("%d", &id);
    
    FILE *file = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    
    if (file != NULL && temp != NULL) {
        while (fscanf(file, "%d,%[^,],%d", &u.id, u.name, &u.age) == 3) {
            if (u.id != id) {
                fprintf(temp, "%d,%s,%d\n", u.id, u.name, u.age);
            } else {
                found = true;
            }
        }
        fclose(file);
        fclose(temp);
        remove("users.txt");
        rename("temp.txt", "users.txt");
        
        if (!found) {
            printf("ID not found.\n");
        } else {
            printf("User deleted.\n");
        }
    } else {
        if (file != NULL) {
            fclose(file);
        }
    }
}

int main() {
    int choice;
    
    while (true) {
        printf("\n1.Create 2.Read 3.Update 4.Delete 5.Exit\nChoice: ");
        scanf("%d", &choice);
        
        if (choice == 1) {
            create();
        } else if (choice == 2) {
            read();
        } else if (choice == 3) {
            update();
        } else if (choice == 4) {
            delete();
        } else if (choice == 5) {
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }
    
    return 0;
}