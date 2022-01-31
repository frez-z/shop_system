#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

#define BUFFER_LENGTH 255

#define BR "\n" // new line

// terminal colors
#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define MAGENTA "\033[95m"
#define CYAN "\033[96m"
#define WHITE "\033[97m"

#define RST "\033[0m" // reset colour to default

// save file as .csv as it ediable on excel if exist
#define INVENTORY_FILE "inventory.csv"
#define RECEIPT_FILE "receipt.csv"

#define MAX_ITEMS 100

typedef struct inventory {
    char name[BUFFER_LENGTH];
    float price;
    int quantity;
} inventory;

typedef struct receipt {
    int receipt_number;
    float total;
} receipt;

// file I/O related functions declarations
int loadInventory(void);
int loadReceipt(void);
int insertInventory(char inventory_name[], float price, int quantity, int *count);
int insertReceipt(float quantity, int *count);
int updateInventory(int inventory_id, int quantity, int count);

// admin related functions declarations
void insertDefaultInventory(FILE **fp);
void adminMainMenu(void);
void adminInsertInventory(void);
void adminRestockInventory(void);
void adminShowSales(void);
void showInventory(void);

// customer related functions declarations
void customerMainMenu(void);
void customerOrders(void);
void customerOrderSelection(int *cart);
int customerAddToCart(int **cart);

// utility functions declarations
void createConsoleWindow(short width);

inventory inventory_list[MAX_ITEMS]; // max 100 items
receipt receipt_list[200]; // max 200 receipts [transaction]

int inventory_count = 0; // number of exist items in inventory
int receipt_count = 0; // number of exist receipts in receipt

int main() {
    createConsoleWindow(85); // create console window with 85 columns
    createConsoleWindow(85); // make sure it is 85 columns
    system("cls");
    int flag = 0; // if 1 then go to admin mode

    // create file if not exist
    if( (access(INVENTORY_FILE, F_OK) != 0) || (access(RECEIPT_FILE, F_OK) != 0) ) {

        // create inventory file
        FILE *fp; fp = fopen(INVENTORY_FILE, "w");
        if (fp == NULL) exit(1);
        
        fprintf(fp, "item_name,price,quantity\n"); // create .csv file header
        insertDefaultInventory(&fp); // insert default inventory
        fclose(fp);

        // create receipt file
        fp = fopen(RECEIPT_FILE, "w");
        if (fp == NULL) exit(1);

        // create .csv file header and initalize receipt number equal to 100000
        fprintf(fp, "recipt_number,total\n100000,0\n");
        fclose(fp);
        flag = 1;
    }

    // load inventory file and receipt file into array of struct
    inventory_count = loadInventory();
    receipt_count = loadReceipt();

    if (flag == 1) adminMainMenu(); // go to admin mode

    customerMainMenu(); // go to customer mode

    return 0;
}

// customer functions start ---------------------------------------------------

void customerMainMenu(){
    system("cls");
    printf("\t\t\t    Restaurant De'Frencher" BR BR);
    printf("\t        created by " BLUE "Mr. Farez, Mr. Adam " RST "and" BLUE " Mr. Afiq" RST BR BR);
    printf("---------------------------------- Main Menu ---------------------------------" BR BR);
    printf("\t[1]" CYAN " dine in " RST YELLOW "[ not avaiable yet ]"RST BR BR);
    printf("\t[2]" CYAN " take away <- "RST GREEN"5%% off!! "RST CYAN"(our grand opening)" RST BR BR);
    printf("\t[3]" BLUE " administrator" RST BR BR);
    printf("\t[4] exit" BR BR);
    
    // error handling. only accept input in range 1 - 4 only
    int input;
    do {input = getch() - '0';} while (input > 4 || input <= 0);

    switch(input){
        case 1:
            // not available forever and ever and after the end of the world
            break;
        case 2:
            system("cls");
            customerOrders();
            break;
        case 3:
            adminMainMenu();
            break;
        case 4:
            system("cls");
            exit(0);
            break;
    }
    customerMainMenu();
}

void customerOrders(){
    float payment = 0;
    int cart_list[MAX_ITEMS]; // max 100 items in cart
    for (int i = 0; i < MAX_ITEMS; i++) cart_list[i] = 0; // initialize cart list to 0
    customerOrderSelection(&cart_list[0]);
    float tax;

    printf("\t\t\t    Restaurant De'Frencher" BR BR);
    printf("\t        created by " BLUE "Mr. Farez, Mr. Adam " RST "and" BLUE " Mr. Afiq" RST BR BR);

    // print cart
    printf("\t|-------------------------------------------------|" BR);
    printf("\t|           item_name             |     total     |" BR);
    printf("\t|-------------------------------------------------|" BR);
    for(int i = 0; i < inventory_count; i++){
        if (cart_list[i] == 0) continue;

        // calculate total price of each item
        float calculated_price = (float)cart_list[i] * inventory_list[i].price;

        // add sum of total price of each item to payment
        payment += calculated_price;
        
        // update file with new quantity
        updateInventory(i, inventory_list[i].quantity, inventory_count);

        printf("\t| %-31s | RM %-10.2f |" BR, inventory_list[i].name, calculated_price);
    }
    printf("\t|-------------------------------------------------|" BR);
    
    tax = payment * 0.06; // 6% tax
    insertReceipt((float)(payment + tax), &receipt_count); // insert receipt into receipt file

    printf("\t| %-31s |    %-10d |" BR, "your resit id", receipt_list[receipt_count-1].receipt_number);
    printf("\t|-------------------------------------------------|" BR);
    printf("\t| %-31s | RM %-10.2f |" BR, "payment", payment);
    printf("\t| %-31s | RM %-10.2f |" BR, "6% sst", tax);
    printf("\t| %-31s | RM %-10.2f |" BR, "total payment", payment + tax);
    printf("\t|-------------------------------------------------|" BR);

    getch();
}

void customerOrderSelection(int *cart_list){
    printf("\t\t\t    Restaurant De'Frencher" BR BR);
    printf("\t        created by " BLUE "Mr. Farez, Mr. Adam " RST "and" BLUE " Mr. Afiq" RST BR BR);
    printf("---------------------------------- Menu -------------------------------" BR BR);
    // show inventory
    printf("|--------------------------------------------------------------------------------|" BR);
    printf("| id |        item_name                |     price     |  available  |  in cart  |" BR);
    printf("|--------------------------------------------------------------------------------|" BR);
    for(int i = 0; i < inventory_count; i++){
        if (inventory_list[i].quantity <= 3) {
            if (cart_list[i] > 0)
                printf("| %-2d |"RED" %-31s "RST"| RM %-10.2f |   %6d    |"RST BLUE"  %6d   "RST"|" BR, i, inventory_list[i].name, inventory_list[i].price, inventory_list[i].quantity, cart_list[i]);
            else
                printf("| %-2d |"RED" %-31s "RST"| RM %-10.2f |   %6d    |  %6d   |" BR, i, inventory_list[i].name, inventory_list[i].price, inventory_list[i].quantity, cart_list[i]);
            continue;
        }
        if (cart_list[i] > 0)
                printf("| %-2d |"RED" %-31s "RST"| RM %-10.2f |   %6d    |"RST BLUE"  %6d   "RST"|" BR, i, inventory_list[i].name, inventory_list[i].price, inventory_list[i].quantity, cart_list[i]);
        else
            printf("| %-2d | %-31s | RM %-10.2f |   %6d    |  %6d   |" BR, i, inventory_list[i].name, inventory_list[i].price, inventory_list[i].quantity, cart_list[i]);
    }
    printf("|--------------------------------------------------------------------------------|" BR);

    printf("\t[a] add to cart" BR);
    printf("\t[b] checkout" BR);
    
    // error handling. only accept input 'a' or 'b' only
    int input;
    do {input = getch() - 'a';} while (input > 1 || input < 0);
    switch (input) {
        case 0:
            // add to cart
            int result = customerAddToCart(&cart_list);
            if (result == 1) printf(BR GREEN "Successfully added to cart" RST BR BR);
            else if (result == 2) printf(BR YELLOW "added to cart, but using cap amount of stock" RST BR BR);
            else if (result == -1) printf(BR RED "Failed to add to cart" RST BR BR);
            else if (result == 0) printf(BR RED "run out off stock" RST BR BR);
            getch();
            system("cls");
            customerOrderSelection(cart_list);
            break;
        case 1:
            // checkout
            system("cls");
            return;
            break;
    }
}

int customerAddToCart(int **cart_list){
    int inventory_id; int quantity; int FLAG = 1;
    printf( BR CYAN "\tmenu id :"); scanf("%d", &inventory_id);
    printf("\tquantity :"); scanf("%d", &quantity);
    printf(RST);
    if (inventory_id < 0 || inventory_id >= inventory_count) return -1; // error handling
    
    if (quantity < 0) quantity = 1; // if negative quantity, set to 1
    
    if (quantity > inventory_list[inventory_id].quantity){
        quantity = inventory_list[inventory_id].quantity; // if quantity exceed the stock, set to stock
        FLAG = 2;
    }
    if (quantity == 0) return 0; // if quantity is 0, return 0

    // update cart list
    (*cart_list)[inventory_id] += quantity;

    // update inventory without changing the original inventory file
    inventory_list[inventory_id].quantity -= quantity;
    
    return FLAG;
}

// customer functions end -----------------------------------------------------



// admin functions start ------------------------------------------------------
void showInventory(){
    printf("|-------------------------------------------------------------------|" BR);
    printf("| id |        item_name                |     price     |  quantity  |" BR);
    printf("|-------------------------------------------------------------------|" BR);
    for(int i = 0; i < inventory_count; i++){
        if (inventory_list[i].quantity <= 5) {
            printf("| %-2d |"RED" %-31s "RST"| RM %-10.2f |   %5d    |" BR, i, inventory_list[i].name, inventory_list[i].price, inventory_list[i].quantity);
            continue;
        }
        printf("| %-2d | %-31s | RM %-10.2f |   %5d    |" BR, i, inventory_list[i].name, inventory_list[i].price, inventory_list[i].quantity);
    }
    printf("|-------------------------------------------------------------------|" BR);
}


void insertDefaultInventory(FILE **fp) {
    // predefined menu name
    char* menu_name[] = {
        "spaghetti-carbonara", "spaghetti-bolognese", "mac-and-cheese",
        "chicken-steak", "pizza-pepperoni(M)", "pizza-hawaiian(M)",
        "pizza-for-lover-(M)", "pizza-cheese-overload-(L)", "soft-drinks",
        "fruit-juice", "coffee", "plain-water"
    };

    // predefined menu price
    float menu_price[] = {
        10.60, 10.60, 8.90, 20.00, 
        25.00, 25.00, 28.50, 40.00, 
        5.00, 4.00, 3.00, 1.00
    };

    for (int i = 0; i < 12; i++) {
        fprintf(*fp, "%s,%.2f,%d\n", menu_name[i], menu_price[i], (rand() % 20) + 1);
    }
}

void adminMainMenu(void){
    system("cls");
    printf("\t\t\t    Restaurant De'Frencher" BR BR);
    printf("\t        created by " BLUE "Mr. Farez, Mr. Adam " RST "and" BLUE " Mr. Afiq" RST BR BR);
    printf("---------------------------- admin interface -----------------------------" BR BR);
    printf("\t[1] " CYAN "view inventory" RST BR BR);
    printf("\t[2] " BLUE "edit inventory <- via excel (if excel downloaded)" RST BR BR);
    printf("\t[3] " CYAN "add new inventory <- via terminal" RST BR BR);
    printf("\t[4] " CYAN "restock inventory <- via terminal" RST BR BR);
    printf("\t[5] " MAGENTA "view restaurant sales" RST BR BR);
    printf("\t[6] return to main menu" BR BR);
    printf("\t[7] "RED"exit application" RST BR BR);

    // error handling. only accept input in range 1 - 6 only
    int choice;
    do {choice = getch() - '0';} while (choice > 7 || choice <= 0);
    system("cls");
    switch(choice){
        case 1:
            printf("current inventory in restaurant:" BR BR);
            showInventory();
            getch();
            system("cls");
            break;
        case 2:
            printf("opening file using external app");
            system(INVENTORY_FILE);
            inventory_count = loadInventory(); // reload inventory
            break;
        case 3:
            adminInsertInventory();          
            break;
        case 4:
            adminRestockInventory();  
            break;
        case 5:
            adminShowSales();
            break;
        case 6:
            system("cls");
            return;
            break;
        case 7:
            system("cls");
            exit(0);
            break;
    }
    adminMainMenu();
}

void adminInsertInventory(void){
    printf("\t\t\t    Restaurant De'Frencher" BR BR);
    printf("\t        created by " BLUE "Mr. Farez, Mr. Adam " RST "and" BLUE " Mr. Afiq" RST BR BR);
    printf("---------------------------- admin interface -----------------------------" BR BR);
    char name[30];
    float price;
    int quantity;
    for(;;){
        printf("insert item name :"); scanf("%s", name);
        printf("insert item price :"); scanf("%f", &price);
        printf("insert item quantity :"); scanf("%d", &quantity);
        insertInventory(name, price, quantity, &inventory_count);

        printf(BR "insert another item? (y/n) :");
        char choice;
        scanf(" %c", &choice);
        if (choice == 'n' || choice == 'N') return;
        printf(BR "---------------------------------------------------------" BR BR);
    }
}

void adminRestockInventory(void){
    system("cls");
    printf(" inventory in restaurant\n");
    showInventory();

    int inventory_id;
    int change_quantity;
    printf(BR BR" enter id to restock: ");
    scanf("%d", &inventory_id);
    printf(" enter quantity to added: ");
    scanf("%d", &change_quantity);
    updateInventory(inventory_id, change_quantity, inventory_count);
    printf(BR" exit and view change? (y/n):");
    char choice;
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y') {
        system("cls");
        printf(" updated inventory in restaurant\n");
        showInventory();
        getch();
        return;
    }
    adminRestockInventory();
}

void adminShowSales(void){
    printf("\t\t\t    Restaurant De'Frencher" BR BR);
    printf("\t        created by " BLUE "Mr. Farez, Mr. Adam " RST "and" BLUE " Mr. Afiq" RST BR BR);
    printf("---------------------------- sales -----------------------------" BR BR);

    float total_sales = 0; float tax;
    for (int i = 0; i < receipt_count; i++) total_sales += receipt_list[i].total;
    printf("receipt count: %d" BR, receipt_count);
    printf("total expenses: RM %.2f" BR BR, total_sales);

    tax = total_sales * 0.06;
    printf("total tax: RM %.2f" BR BR, tax);

    printf("total sales: RM %.2f" BR BR, total_sales - tax);

    // sale profit is 30% of total sales
    printf("total profit: RM %.2f" BR BR, (total_sales - tax) * 0.35);

    getch();

}

// admin functions end --------------------------------------------------------



// file I/O realted functions start -------------------------------------------

int loadInventory(void){
    FILE* filePointer;
    char buffer[BUFFER_LENGTH];

    filePointer = fopen(INVENTORY_FILE, "r");

    int count = 0;
    while(fgets(buffer, BUFFER_LENGTH, filePointer)) {
        if (count == 0){
            count++; continue; // ignore first line
        }

        buffer[strcspn(buffer, "\n")] = 0;

        char* token = strtok(buffer, ",");
        strcpy(inventory_list[count - 1].name, token);
        inventory_list[count - 1].price = atof(strtok(NULL, ","));
        inventory_list[count - 1].quantity = atoi(strtok(NULL, ","));

        count++;
    }

    fclose(filePointer);
    return count - 1;
}

int loadReceipt(void){
    FILE* filePointer;
    char buffer[BUFFER_LENGTH];

    filePointer = fopen(RECEIPT_FILE, "r");

    int count = 0;
    while(fgets(buffer, BUFFER_LENGTH, filePointer)) {

        buffer[strcspn(buffer, "\n")] = 0;

        receipt_list[count].receipt_number = atoi(strtok(buffer, ","));
        receipt_list[count].total = atof(strtok(NULL, ","));
        count++;
    }

    fclose(filePointer);
    return count;
}

int insertInventory(char inventory_name[], float price, int quantity, int *count){
    FILE* fp;
    fp = fopen(INVENTORY_FILE, "a+");
    if (fp == NULL) return 0;
    fprintf(fp, "%s,%.2f,%d\n", inventory_name, price, quantity);
    strcpy(inventory_list[*count].name, inventory_name);
    inventory_list[*count].price = price;
    inventory_list[*count].quantity = quantity;
    *count = *count + 1;
    fclose(fp);
    return 1;
}

int insertReceipt(float quantity, int *count){
    FILE* fp;
    char buffer[BUFFER_LENGTH];
    fp = fopen(RECEIPT_FILE, "a+");
    if (fp == NULL) return 0;
    int receipt_number = ++receipt_list[*count - 1].receipt_number;
    fprintf(fp, "%d,%.2f\n", receipt_number, quantity);
    receipt_list[*count].receipt_number = receipt_number;
    receipt_list[*count].total = quantity;
    *count++;
    fclose(fp);
    return 1;
}

int updateInventory(int inventory_id, int change_quantity, int count){
    inventory_list[inventory_id].quantity += change_quantity;
    FILE *fp;
    fp = fopen(INVENTORY_FILE, "w");
    if (fp == NULL) return 0;
    fprintf(fp, "item_name,price,quantity\n");
    for (int i = 0; i < count; i++){
        fprintf(fp, "%s,%.2f,%d\n", inventory_list[i].name, inventory_list[i].price, inventory_list[i].quantity);
    }
    fclose(fp);
    return 1;
}

// file I/O realted functions end ---------------------------------------------------


// windows related functions start ------------------------------------------------

void createConsoleWindow(short width){
    HANDLE wHnd;    // Handle to write to the console. (using windows api)
    // Set up the handles for writing:
    wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND consoleWindow = GetConsoleWindow();

    SMALL_RECT windowSize = {0 , 0 , width, 30}; // window size
    COORD coord = {width + 1, 60}; // buffer size
    SetConsoleWindowInfo(wHnd, TRUE, &windowSize); // set window size
    SetConsoleScreenBufferSize(wHnd, coord); // set buffer size same as window size
    ShowScrollBar(consoleWindow, SB_HORZ, 0); // disable scroll bar

    // disable maximize and minimize button
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
}

// windows related functions end --------------------------------------------------