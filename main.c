#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <unistd.h>
#include <conio.h>
#include <math.h>
#include <string.h>

#define USERNAME "admin"
#define PASSWORD "admin"

#define BUFFER_LENGTH 255 // string length
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

#define INVENTORY_FILE "inventory.csv" // save file as .csv as it ediable on excel if installed

// struct for items
typedef struct inventory {
    char name[BUFFER_LENGTH];
    float price;
    int quantity;
} inventory;

// customer I/O functions prototypes
void customerMenu(void);
void customerProcess(void);
void customerOder(int *item_cart);
int customerAddToCart(int inventory_id, int quantity, int **cart_list);

// admin I/O functions prototypes
void adminRestockInventory(void);
void adminProcess(void);

// inventory I/O functions prototypes
void insertInventory(FILE **fp);
int loadInventory(void);
void updateInventory(int id, int change_of_quantity);

// utility functions prototypes
void createConsoleWindow(short width);

// global variables
inventory *itemList;
int itemCount = 12; // default item count
int admin_mode = 0;

int main() {
    createConsoleWindow(85); createConsoleWindow(85);// create fixed size console window

    // create file inventory.csv if not exist
    if( access(INVENTORY_FILE, F_OK) != 0 ){
        FILE *fp;
        fp = fopen(INVENTORY_FILE, "w");
        if (fp == NULL) exit(1); // error, exit application
        
        fprintf(fp, "item_name,price,quantity\n"); // create .csv file header
        insertInventory(&fp); // insert default inventory
        fclose(fp);
    }
    itemCount = loadInventory(); // load inventory file    
    customerMenu();
    system("cls");
}

// admin I/O functions --------------------------------------------------------

void adminProcess(void){
    if (admin_mode == 0){
        system("cls"); printf(BR GREEN);
        printf("\t\t\t\tadmin login interface" BR BR);

        char username[20], password[20];
        printf("\t\t\t\tusername :"); scanf("%s", username);
        printf("\t\t\t\tpassword :"); scanf(" %s", password); printf(RST);

        // if the username or password not same, return to main menu
        if (!((strcmp(username, USERNAME) == 0 && strcmp(password, PASSWORD) == 0))) {
            printf(RED BR "\t\t\t\tlogin fail!" RST BR BR "\t\t\t\tenter anything to continue");
            getch(); return;
        }
        admin_mode = 1; // no longer need to login afterwards (while aplication is running)
    }
    for(;;){
        system("cls");
        printf(BR "\t\t\t\t  Restaurant Group 2" BR BR);
        printf("\t\t     created by " BLUE "Mr. Farez, Mr. Adam " RST "and " BLUE "Mr. Afiq" RST BR BR);
        printf("\t[1]" CYAN " edit inventory file via external app" RST BR BR);
        printf("\t[2]" CYAN " restock current inventory" RST BR BR); printf("\t[3] return to menu" RST BR BR);
        // error handling. only accept input in range 1 - 4 only
        int input = 0; while (input > 3 || input <= 0) input = getch() - '0';

        switch(input){
            case 1:
                system("cls"); printf(BR BR "\t inventory.csv file is important in this application." BR); 
                printf("\t corrupt it will cause the app misbehave" BR); 
                printf("\t enter any key to continue...." BR); getch();
                system(INVENTORY_FILE); // ask the system to lauch the file.
                itemCount = loadInventory(); // reloading inventory into application.
                system("cls"); printf("\t press anything to redirect to admin menu" BR); getch();
                break;
            case 2:
                adminRestockInventory(); break;
            case 3:
                return; break;
        }
    }
}

void printInventory(void){
    printf(BR "\t\t\t\t  Restaurant Group 2" BR BR);
    printf("\t\t     created by " BLUE "Mr. Farez, Mr. Adam " RST "and " BLUE "Mr. Afiq" RST BR BR);
    printf("\t\t    |--------------------------------------------|" BR);
    printf("\t\t    | id |          item name          | on sale |" BR);
    printf("\t\t    |--------------------------------------------|" BR);

    for (int i = 0; i < itemCount; i++)
        printf("\t\t    | %2d |  %-25s  |   %4d  |" BR , i, itemList[i].name, itemList[i].quantity);
    printf("\t\t    |--------------------------------------------|" BR);
}

void adminRestockInventory(void){
    system("cls");
    printInventory();

    int inventory_id, change_of_quantity;
    printf(BR BR"\t\t    enter id to restock: "); scanf("%d", &inventory_id);
    printf("\t\t    enter quantity to added: "); scanf("%d", &change_of_quantity);

    // invalid id, recall function.
    if (inventory_id > itemCount || inventory_id < 0) adminRestockInventory();

    itemList[inventory_id].quantity += change_of_quantity; // update inventory array
    updateInventory(inventory_id, change_of_quantity); // update inventory file

    system("cls");
    printInventory();
    printf(BR BR"\t\t    exit? (y/n):");
    char choice;
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y') return;
    adminRestockInventory();
}

// customer I/O functions -----------------------------------------------------

void customerMenu(void){
    system("cls"); // clear screen
    printf(BR "\t\t\t\t  Restaurant Group 2" BR BR);
    printf("\t\t     created by " BLUE "Mr. Farez, Mr. Adam " RST "and " BLUE "Mr. Afiq" RST BR BR);
    printf("\t[1]" CYAN " dine in " RST YELLOW "[ unavailable due to Covid-19 ]" RST BR BR);
    printf("\t[2]" CYAN " take away <- " RST GREEN "2%% off"RST CYAN" promotion!" RST BR BR);
    printf("\t[3]" BLUE " admin interface" RST BR BR);
    printf("\t[4] exit" BR BR);
    
    // error handling. only accept input in range 1 - 4 only
    int input = 0; while (input > 4 || input <= 0) input = getch() - '0';

    switch(input){
        case 1: /* not available forever */ break;

        case 2:customerProcess(); break;

        case 3:adminProcess(); break;

        case 4: return; break; // exit the program
    }
    customerMenu(); // loop back to start using recursion
}

void customerProcess(void){
    float total_price = 0;
    int *cart_list; // list of item id in cart
    cart_list = calloc(itemCount, sizeof(int)); // allocate memory for cart list
    customerOder(&cart_list[0]); // select item and add it to cart

    // update inventory in the file
    for (int i= 0; i < itemCount; i++)
        if (cart_list[i] != 0) updateInventory(i, -cart_list[i]);

    system("cls");
    printf(BR "\t\t\t\t  Restaurant Group 2" BR BR);
    printf("\t\t     created by " BLUE "Mr. Farez, Mr. Adam " RST "and " BLUE "Mr. Afiq" RST BR BR);
    printf("\t       |------------------------------------------------------|" BR);
    printf("\t       |          item name          |  unit  |     total     |" BR);
    printf("\t       |------------------------------------------------------|" BR);
    for (int i = 0; i < itemCount; i++){
        if (cart_list[i] > 0){ // only print non-zero item in cart.
            float items_payment = cart_list[i] * itemList[i].price; // calculate price per item
            printf("\t       |  %-25s  |  %4d  |   RM %-8.2f |" BR, itemList[i].name, cart_list[i], items_payment);
            total_price += items_payment; // add sum of price per item to total whole price
        }
    }
    float total_before = total_price; // copy total price
    float promo = total_price * 0.02; total_price -= promo; // calculate promo deal at 3% off
    float tax = total_price * 0.06; total_price += tax; // calculate tax at 6%
    
    // print the amount user needs to pay
    printf("\t       |------------------------------------------------------|" BR);
    printf("\t       |                                                      |" BR);
    printf("\t       |  total                                RM %10.2f  |" BR, total_before);
    printf("\t       |  2%% of promotion                  [-] RM %10.2f  |" BR, promo);
    printf("\t       |  6%% tax                           [+] RM %10.2f  |" BR, tax);
    printf("\t       |  total inc. tax                       RM %10.2f  |" BR, total_price);
    printf("\t       |------------------------------------------------------|" BR BR BR);
    printf(GREEN "\t\t\t      Thank You eating with us!!" RST BR); getch();
}

void customerOder(int *cart_list){
    system("cls"); printf(BR "\t\t\t\t  Restaurant Group 2" BR BR);
    printf("\t\t     created by " BLUE "Mr. Farez, Mr. Adam " RST "and " BLUE "Mr. Afiq" RST BR BR);
    printf("\t|---------------------------------------------------------------------|" BR);
    printf("\t| id |          item name          |     price     | on sale |  cart  |" BR);
    printf("\t|---------------------------------------------------------------------|" BR);
    
    for (int index = 0; index < itemCount; index++){ // loop through all item in inventory
        if (itemList[index].quantity <= 0)
            printf("\t| %2d |"RED"  %-25s  "RST"|"RED"    RM %-7.2f "RST"|   %4d  |" , /* if item is out of stock = RED */
            index, itemList[index].name, itemList[index].price, itemList[index].quantity);
        else
            printf("\t| %2d |  %-25s  |    RM %-7.2f |   %4d  |", /* if item is in stock = WHITE */
            index, itemList[index].name, itemList[index].price, itemList[index].quantity);
        
        if (cart_list[index] == 0) printf("  ----  |"BR); // if item not in cart, display "----" at cart column
        else printf(BLUE "  %4d  "RST"|"BR, cart_list[index]);
    }
    printf("\t|---------------------------------------------------------------------|" BR BR);
    printf("\t[a] "CYAN"cart"RST"  [b] "BLUE"checkout" RST BR);
    
    // error handling. only accept input in range a - b only
    char input; do {input = (char)getch();}while (!(input == 'a' || input == 'b'));

    switch (input){
        case 'a': {
            int quantity, id;
            printf(BR"\titem id :"); scanf("%d", &id);
            printf(BR"\t"CYAN"[+] add item"RST BR"\t[-] remove item" RST BR);
            printf("\tquantity :"); scanf("%d", &quantity);
            int result = customerAddToCart(id, quantity, &cart_list);
            switch(result){
                case 10: printf(BR RED"\t'%s' out of stock!" RST, itemList[id].name); break;
                case 11: printf(BR RED"\t'%s' have no such quantity in cart!" RST, itemList[id].name); break;
                case -1: printf(BR RED"\titem with id = %d not exist!" RST, id); break;
                case  1: printf(BR GREEN"\tadded %d of '%s' to cart" RST, quantity, itemList[id].name); break;
                case  2: printf(BR YELLOW"\tremoved %d of '%s' from the cart" RST, abs(quantity), itemList[id].name); break;    
            }
            getch(); system("cls"); break;// wait for user to press any key, then clear the screen
        }
        case 'b':
            int test_sum = 0;
            for (int i = 0; i < itemCount; i++) test_sum += cart_list[i];
            
            if (test_sum == 0){ printf(BR BR RED"\tno item in cart!" RST BR BR); getch(); system("cls");}
            else return; // return to customerProcess
            break;
    }
    customerOder(cart_list); // recursive call to ensure it's infinity loop.
}

int customerAddToCart(int inventory_id, int quantity, int **cart_list){
    int FLAG = 1;
    if (inventory_id < 0 || inventory_id >= itemCount) return -1; // id not exist
    
    if (quantity > itemList[inventory_id].quantity) return 10; // item exceed the stock

    if (quantity < 0 && abs(quantity) > (*cart_list)[inventory_id]) return 11; // user remove more than what in cart

    if (quantity < 0) FLAG = 2; // user want to remove item from cart

    (*cart_list)[inventory_id] += quantity; // add or remove item to cart
    itemList[inventory_id].quantity -= quantity;  // deduct or add item to inventory
    
    return FLAG;
}

// inventory I/O functions ----------------------------------------------------

void insertInventory(FILE **fp) {

    char* menu_name[] = {
        "spaghetti-carbonara", "spaghetti-bolognese", "mac-and-cheese", "chicken-steak","pizza-pepperoni",
        "pizza-hawaiian", "vegen-pizza", "pizza-tuna-cheese", "soft-drinks", "fruit-juice", "coffee", "plain-water"
    }; // predefined menu name

    float menu_price[] = {
        10.60, 10.60, 8.90, 20.00, 29.00, 30.00,
        23.50, 35.00, 5.00, 4.00, 3.00, 1.00
    }; // predefined menu price

    for (int i = 0; i < 12; i++)
        fprintf(*fp, "%s,%.2f,%d\n", menu_name[i], menu_price[i], (rand() % 20) + 1); // add random quantity
}

int loadInventory(void) {
    FILE* filePointer; int count = -1; char buffer[BUFFER_LENGTH];

    if (itemList != NULL) free(itemList); // free the memory if it's not NULL
    itemList = (inventory*) calloc(itemCount, sizeof(inventory)); // allocate memory for inventory list
    if (itemList == NULL) {
                printf(RED "Error: reallocate memory failed!" RST); getch(); exit(-1);
    }

    filePointer = fopen(INVENTORY_FILE, "r");

    // basically loop through the file and read each line
    while(fgets(buffer, BUFFER_LENGTH, filePointer)) {
        if (count == -1){
            count++; continue; // ignore first line as it is header
        }
        else if (count >= (itemCount - 1)){ //reallocate more memory to store more items

            // reallocate memory by doubling the size of the array
            itemList = (inventory*) realloc(itemList, (itemCount*=2) * sizeof(inventory));
            if (itemList == NULL) {
                printf(RED "Error: reallocate memory failed!" RST); getch(); exit(-1);
            }
        }
        
        buffer[strcspn(buffer, "\n")] = 0; // remove newline character at the end of the line
        char* token = strtok(buffer, ","); // spliting the string by comma

        // load file data into struct
        strcpy(itemList[count].name, token);
        itemList[count].price = atof(strtok(NULL, ","));
        itemList[count].quantity = atoi(strtok(NULL, ","));

        count++;
    }
    fclose(filePointer); return count; // close file and return count
}

void updateInventory(int id, int change_of_quantity){
    FILE *fp;
    fp = fopen(INVENTORY_FILE, "w"); // open file to overwrite
    if (fp == NULL) return; // failed to open file
    fprintf(fp, "item_name,price,quantity\n");

    // rewrite file with updated data
    for (int i = 0; i < itemCount; i++)
        fprintf(fp, "%s,%.2f,%d\n", itemList[i].name, itemList[i].price, itemList[i].quantity);
    fclose(fp);
}

// utility functions ----------------------------------------------------------

void createConsoleWindow(short width){
    HANDLE wHnd; // Handle to write to the console. (using windows api)
    wHnd = GetStdHandle(STD_OUTPUT_HANDLE); // Set up the handles for writing to the console.
    HWND consoleWindow = GetConsoleWindow();

    SMALL_RECT windowSize = {0 , 0 , width, 35}; // window size
    COORD coord = {width + 1, 500}; // buffer size
    SetConsoleWindowInfo(wHnd, TRUE, &windowSize); // set window size
    SetConsoleScreenBufferSize(wHnd, coord); // set buffer size
    ShowScrollBar(consoleWindow, SB_HORZ, 0); // disable horizontal scroll bar

    // disable maximize and minimize button
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
}