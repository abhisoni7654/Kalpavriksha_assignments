#include <stdio.h>

struct Student {
    int rollno;
    char name[50];
    int marks1,marks2,marks3;
};
int findTotal(int marks1,int marks2,int marks3) {
    return marks1+marks2+marks3;
}
float findAverage(int total) {
    return (float)total/3.0f;
}
char findGrade(float avg) {
    if (avg>=85) {
        return 'A';
    } else if (avg>=70) {
        return 'B';
    } else if (avg>=50) {
        return 'C';
    } else if (avg>=35) {
        return 'D';
    } else {
        return 'F';
    }
}
int findStars(char grade) {
    switch (grade) {
        case 'A':
            return 5;
        case 'B':
            return 4;
        case 'C':
            return 3;
        case 'D':
            return 2;
        default:
            return 0;
    }
}
void printRoll(struct Student arr[],int ind,int num) {
    if (ind>=num) {
        return;
    }
    printf("%d ",arr[ind].rollno);
    printRoll(arr,ind+1,num);
}
int main() {
    int num;
    printf("Enter no. of students: ");
    scanf("%d",&num);
    if(num<1||num>100){
        printf("Invalid no. of students");
        return 0;
    }
    struct Student students[100];
    for (int i=0;i<num;i++) {
        printf("\nEnter details for student %d:\n",i+1);
        printf("Enter roll no: ");
        scanf("%d",&students[i].rollno);
        printf("Enter name: ");
        scanf("%s",students[i].name);
        printf("Enter marks 1: ");
        scanf("%d",&students[i].marks1);
        printf("Enter marks 2: ");
        scanf("%d",&students[i].marks2);
        printf("Enter marks 3: ");
        scanf("%d",&students[i].marks3);
    }
    for (int i=0;i<num;i++) {
        int total=findTotal(
            students[i].marks1,
            students[i].marks2,
            students[i].marks3
        );
        float avg=findAverage(total);
        char grade=findGrade(avg);
        int stars=findStars(grade);
        printf("\nRoll: %d\n",students[i].rollno);
        printf("Name: %s\n",students[i].name);
        printf("Total: %d\n",total);
        printf("Average: %.2f\n",avg);
        printf("Grade: %c\n",grade);
        if (grade=='F') {
            continue;
        }
        printf("Performance: ");
        for (int j=0;j<stars;j++) {
            printf("*");
        }
        printf("\n");
    }
    printf("\nList of Roll Numbers: ");
    printRoll(students,0,num);
    printf("\n");
    return 0;
}