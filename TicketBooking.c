#include <stdio.h>
#include <string.h>
int main() {
    const int STANDARD_PRICE = 150;
    const int PREMIUM_PRICE = 250;
    const int RECLINER_PRICE = 400;
    char name[50];
    char ticket_category[50];
    int ticket_count;
    int category_choice;
    int individual_price;
    float subtotal;
    float discount = 0.0f;
    float bill;
    printf("Enter your name: ");
    scanf("%s", name);
    printf("Enter no. of tickets: ");
    scanf("%d", &ticket_count);
    if (ticket_count < 1 || ticket_count > 20) {
        printf("Invalid ticket count\n");
        return 1;
    }
    printf("Enter ticket category:\n");
    printf("1. Standard\n");
    printf("2. Premium\n");
    printf("3. Recliner\n");
    scanf("%d", &category_choice);
    switch (category_choice) {
        case 1:
            strcpy(ticket_category, "Standard");
            individual_price = STANDARD_PRICE;
            break;
        case 2:
            strcpy(ticket_category, "Premium");
            individual_price = PREMIUM_PRICE;
            break;
        case 3:
            strcpy(ticket_category, "Recliner");
            individual_price = RECLINER_PRICE;
            break;
        default:
            printf("Invalid ticket category\n");
            return 1;
    }
    subtotal = ticket_count * individual_price;
    if (ticket_count >= 5) {
        discount = subtotal * 0.10f;
    } else {
        discount = 0.0f;
    }
    bill = subtotal - discount;
    printf("Customer: %s\n", name);
    printf("Ticket category: %s\n", ticket_category);
    printf("Tickets: %d\n", ticket_count);
    printf("Subtotal: %.2f\n", subtotal);
    printf("Discount: %.2f\n", discount);
    printf("Final Bill: %.2f\n", bill);
    return 0;
}