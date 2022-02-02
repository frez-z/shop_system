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

// save file as .csv as it ediable on excel if exist
#define INVENTORY_FILE "inventory.csv"

#define ITEM_MAX 100 // max number reserved for item

// struct for items
typedef struct inventory {
    char name[BUFFER_LENGTH];
    float price;
    int quantity;
} inventory;

// customer I/O functions prototypes
void customerMenu(void);
void customerProcess(void);
void customerDecision(int *item_cart);
int customerAddToCart(int inventory_id, int *quantity, int **cart_list);

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
inventory inventory_list[ITEM_MAX];
int inventory_count = 0;
int admin_mode = 0;

int main() {
    createConsoleWindow(85); // create fixed size console window
    createConsoleWindow(85); // make sure the window is created

    // create file inventory.csv if not exist
    if( access(INVENTORY_FILE, F_OK) != 0 ){

        // create inventory file
        FILE *fp;
        fp = fopen(INVENTORY_FILE, "w");
        if (fp == NULL) exit(1); // error, exit application
        
        fprintf(fp, "item_name,price,quantity\n"); // create .csv file header
        insertInventory(&fp); // insert default inventory
        fclose(fp);
    }
    
    inventory_count = loadInventory(); // load inventory from .csv file    
    
    customerMenu(); // call customer menu
}

// admin I/O functions

void adminProcess(void){
    if (admin_mode == 0){
        system("cls"); printf(BR GREEN);
        printf("\t\t\t\tadmin login interface" BR BR);

        char username[20], password[20];
        printf("\t\t\t\tusername :"); scanf("%s", username);
        printf("\t\t\t\tpassword :"); scanf(" %s", password);
        printf(RST);

        // if the username or password not same, return to main menu
        if (!((strcmp(username, USERNAME) == 0 && strcmp(password, PASSWORD) == 0))) {
            printf(RED BR "\t\t\t\tlogin fail!" RST BR BR "\t\t\t\tenter anything to continue");
            getch(); return;
        }
        admin_mode = 1; // no longer need to login afterwards (while aplication is running)
    }
    for(;;){
        system("cls");
        printf(BR "\t\t\t\tRestaurant De'Frencher" BR BR);
        printf("\t\t     created by " BLUE "Mr. Farez, Mr. Adam " RST "and " BLUE "Mr. Afiq" RST BR BR);
        printf("\t[1]" CYAN " edit inventory file via external app" RST BR BR);
        printf("\t[2]" CYAN " restock current inventory" RST BR BR);
        printf("\t[3] return to menu" RST BR BR);
        // error handling. only accept input in range 1 - 4 only
        int input = 0;
        while (input > 3 || input <= 0) input = getch() - '0';

        switch(input){
            case 1:
                system("cls");
                printf(BR BR "\t inventory.csv file is vital in this application." BR); usleep(1000000);
                printf("\t corrupted it will cause the app misbehave" BR); usleep(1000000);
                printf("\t ensure no emtpy row between item," BR); usleep(1000000);
                printf("\t not adding new column,"BR); usleep(1000000);
                printf("\t or editing current column name." BR); usleep(1000000);
                printf("\t close the external app to continue using this app" BR); usleep(1000000);
                printf("\t ...." BR); usleep(1000000);
                printf( "\t lauching external app.");
                system(INVENTORY_FILE); // ask the system to lauch the file.
                inventory_count = loadInventory(); // reloading inventory into application.
                system("cls");
                printf(BR BR "\t returned to app, loaded the data" BR); usleep(1000000);
                printf("\t redirect to admin menu" BR); usleep(1000000);
                break;
            case 2:
                adminRestockInventory();
                break;
            case 3:
                return;
                break;
        }
    }
}

void printInventory(void){
    printf(BR "\t\t\t\tRestaurant De'Frencher" BR BR);
    printf("\t\t     created by " BLUE "Mr. Farez, Mr. Adam " RST "and " BLUE "Mr. Afiq" RST BR BR);
    printf("\t\t    |--------------------------------------------|" BR);
    printf("\t\t    | id |          item name          | on sale |" BR);
    printf("\t\t    |--------------------------------------------|" BR);

    for (int i = 0; i < inventory_count; i++)
        printf("\t\t    | %2d |  %-25s  |   %4d  |" BR , i, inventory_list[i].name, inventory_list[i].quantity);
    printf("\t\t    |--------------------------------------------|" BR);
}

void adminRestockInventory(void){
    system("cls");
    printInventory();

    int inventory_id, change_of_quantity;
    printf(BR BR"\t\t    enter id to restock: "); scanf("%d", &inventory_id);
    printf("\t\t    enter quantity to added: "); scanf("%d", &change_of_quantity);

    // invalid id, recall function.
    if (inventory_id > inventory_count || inventory_id < 0) adminRestockInventory();

    inventory_list[inventory_id].quantity += change_of_quantity;
    updateInventory(inventory_id, change_of_quantity);

    system("cls");
    printInventory();
    printf(BR BR"\t\t    exit? (y/n):");
    char choice;
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y') return;
    adminRestockInventory();
}


// customer I/O functions

void customerMenu(void){
    system("cls"); // clear screen
    printf(BR "\t\t\t\tRestaurant De'Frencher" BR BR);
    printf("\t\t     created by " BLUE "Mr. Farez, Mr. Adam " RST "and " BLUE "Mr. Afiq" RST BR BR);
    printf("\t[1]" CYAN " dine in " RST YELLOW "[ unavailable due to Covid-19 ]" RST BR BR);
    printf("\t[2]" CYAN " take away <- " RST GREEN "2%% off"RST CYAN" promotion!" RST BR BR);
    printf("\t[3]" BLUE " admin interface" RST BR BR);
    printf("\t[4] exit" BR BR);
    
    // error handling. only accept input in range 1 - 4 only
    int input = 0;
    while (input > 4 || input <= 0) input = getch() - '0';

    switch(input){
        case 1:
            // not available forever and ever and after the end of the world
            break;
        case 2:
            customerProcess();
            break;
        case 3:
            adminProcess();
            break;
        case 4:
            system("cls"); exit(0); // exit the program
            break;
    }
    customerMenu(); // loop back to start using recursion
}

void customerProcess(void){
    // customerOder process [show menu <-> select item <-> add to cart -> checkout]
    float total_price = 0;
    int cart_list[ITEM_MAX];
    for (int i = 0; i < ITEM_MAX; i++) cart_list[i] = 0; // initialize cart_list to 0

    customerDecision(&cart_list[0]); // select item and add to cart

    // update inventory in the file
    for (int i= 0; i < inventory_count; i++){
        if (cart_list[i] != 0) updateInventory(i, -cart_list[i]);
    }

    system("cls");
    printf(BR "\t\t\t\tRestaurant De'Frencher" BR BR);
    printf("\t\t     created by " BLUE "Mr. Farez, Mr. Adam " RST "and " BLUE "Mr. Afiq" RST BR BR);
    printf("\t       |------------------------------------------------------|" BR);
    printf("\t       |          item name          |  unit  |     total     |" BR);
    printf("\t       |------------------------------------------------------|" BR);
    for (int i = 0; i < inventory_count; i++){
        if (cart_list[i] > 0){
            
            // print total price per item
            float items_payment = cart_list[i] * inventory_list[i].price;

            // print item name, unit, total
            printf("\t       |  %-25s  |  %4d  |   RM %-8.2f |" BR, inventory_list[i].name, cart_list[i], items_payment);
            
            // add sum of total price per item to total price
            total_price += items_payment;
        }
    }
    float total_before = total_price; // copy total price
    
    float promo = total_price * 0.02; // calculate promo deal at 3% off
    total_price -= promo;

    float tax = total_price * 0.06;   // calculate tax at 6%
    total_price += tax;

    printf("\t       |------------------------------------------------------|" BR);
    printf("\t       |                                                      |" BR);
    printf("\t       |  total                                   RM %7.2f  |" BR, total_before);
    printf("\t       |  2%% of promotion                     [-] RM %7.2f  |" BR, promo);
    printf("\t       |  6%% tax                              [+] RM %7.2f  |" BR, tax);
    printf("\t       |  total inc. tax                          RM %7.2f  |" BR, total_price);
    printf("\t       |------------------------------------------------------|" BR BR BR);

    printf(GREEN "\t\t\t      Thank You eating with us!!" RST BR);
    getch();
}

void customerDecision(int *cart_list){
    system("cls");
    printf(BR "\t\t\t\tRestaurant De'Frencher" BR BR);
    printf("\t\t     created by " BLUE "Mr. Farez, Mr. Adam " RST "and " BLUE "Mr. Afiq" RST BR BR);
    printf("       |---------------------------------------------------------------------|" BR);
    printf("       | id |          item name          |     price     | on sale |  cart  |" BR);
    printf("       |---------------------------------------------------------------------|" BR);

    for (int i = 0; i < inventory_count; i++){
        if (inventory_list[i].quantity <= 0)
            printf("       | %2d |"RED"  %-25s  "RST"|"RED"  RM %-9.2f "RST"|   %4d  |" , i, inventory_list[i].name, inventory_list[i].price, inventory_list[i].quantity);
        else
            printf("       | %2d |  %-25s  |  RM %-9.2f |   %4d  |", i, inventory_list[i].name, inventory_list[i].price, inventory_list[i].quantity);

        if (cart_list[i] == 0) printf("  ----  |"BR);
        else printf(BLUE "  %4d  "RST"|"BR, cart_list[i]);
    }
    printf("       |---------------------------------------------------------------------|" BR BR);
    printf("       [a] "CYAN"cart"RST"  [b] "BLUE"checkout" RST);
    char input;
    do {input = (char)getch();}while (!(input == 'a' || input == 'b'));

    switch (input){
        case 'a':
        {
            int quantity, id, err_quantity = 0;
            printf(BR BR "       " CYAN "[positive quantity] add item" RST BR);
            printf("       [negative quantity] remove item" RST BR BR);
            printf("       item id :"); scanf("%d", &id);
            printf("       quantity :"); scanf("%d", &quantity);
            int result = customerAddToCart(id, &quantity, &cart_list);
            switch(result){
                case 0: printf(BR RED"       '%s' out of stock!" RST, inventory_list[id].name); break;
                case -1: printf(BR RED"       item with id = %d not exist!" RST, id); break;
                case 1: printf(BR GREEN"       added %d of '%s' to cart" RST, quantity, inventory_list[id].name); break;
                case 2: printf(BR GREEN"       added all '%s' to cart" RST, inventory_list[id].name); break;
                case 3: printf(BR YELLOW"       removed %d of '%s' from the cart" RST, abs(quantity), inventory_list[id].name); break;
                case 4: printf(BR MAGENTA"       removed all '%s' from the cart" RST, inventory_list[id].name); break;
                case 5: printf(BR RED"       nothing need to be removed!" RST); break;
            }
            getch(); // wait for user to press any key
            system("cls");
            break;
        }
        case 'b':
            return; // return to customer process
            break;
    }
    customerDecision(cart_list); // recursive call to ensure it's infinity loop.
}

int customerAddToCart(int inventory_id, int *quantity, int **cart_list){
    int FLAG = 1;
    if (inventory_id < 0 || inventory_id >= inventory_count) return -1; // error handling
    
    if (*quantity >= inventory_list[inventory_id].quantity){
        *quantity = inventory_list[inventory_id].quantity; // if quantity exceed the stock, set to stock
        FLAG = 2; // set FLAG to 2 to indicate that the item is at the stock
    }
    
    if (*quantity == 0) return 0; // out of stock
    // update cart list

    if (*quantity + (*cart_list)[inventory_id] < (*cart_list)[inventory_id]){
        // user enter negative quantity (remove item from cart)

        FLAG = 3; // set FLAG to 3 to indicate that SOME item is removed from cart

        if (abs(*quantity) >= (*cart_list)[inventory_id]){
            // remove all item from cart
            *quantity = -(*cart_list)[inventory_id];
            FLAG = 4; // set FLAG to 4 to indicate that ALL item is removed from cart

            if ((*cart_list)[inventory_id] == 0) FLAG = 5;
        }
    }

    (*cart_list)[inventory_id] += *quantity; // will be deducted to update inventory

    // update inventory without changing the original inventory file
    inventory_list[inventory_id].quantity -= *quantity;
    
    return FLAG;
}

// inventory I/O functions ----------------------------------------------------

void insertInventory(FILE **fp) {

    // predefined menu name
    char* menu_name[] = {
        "spaghetti-carbonara", "spaghetti-bolognese", "mac-and-cheese", "chicken-steak",
        "pizza-pepperoni", "pizza-hawaiian", "vegen-pizza", "pizza-tuna-cheese",
        "soft-drinks", "fruit-juice", "coffee", "plain-water"
    };

    // predefined menu price
    float menu_price[] = {
        10.60, 10.60, 8.90, 20.00, 29.00, 30.00,
        23.50, 35.00, 5.00, 4.00, 3.00, 1.00
    };

    for (int i = 0; i < 12; i++)
        fprintf(*fp, "%s,%.2f,%d\n", menu_name[i], menu_price[i], (rand() % 20) + 1); // add random quantity
}

int loadInventory(void) {
    FILE* filePointer;
    char buffer[BUFFER_LENGTH];

    filePointer = fopen(INVENTORY_FILE, "r");

    int count = -1;
    while(fgets(buffer, BUFFER_LENGTH, filePointer)) {
        if (count == -1){
            count++; continue; // ignore first line as it is header
        }
        else if (count >= (ITEM_MAX - 1)) {
            // if count is more than max item, break
            printf(RED "Error: item exceeded max limit" RST BR);
            getch();
            return 0;
        }

        buffer[strcspn(buffer, "\n")] = 0;

        char* token = strtok(buffer, ",");

        // load file data into struct
        strcpy(inventory_list[count].name, token);
        inventory_list[count].price = atof(strtok(NULL, ","));
        inventory_list[count].quantity = atoi(strtok(NULL, ","));

        count++;
    }

    fclose(filePointer);
    return count;
}

void updateInventory(int id, int change_of_quantity){
    FILE *fp;
    fp = fopen(INVENTORY_FILE, "w"); // open file to overwrite
    if (fp == NULL) return; // failed to open file
    fprintf(fp, "item_name,price,quantity\n");

    // rewrite file with updated data
    for (int i = 0; i < inventory_count; i++)
        fprintf(fp, "%s,%.2f,%d\n", inventory_list[i].name, inventory_list[i].price, inventory_list[i].quantity);
    fclose(fp);
}

// utility functions ----------------------------------------------------------

void createConsoleWindow(short width){
    const short HEIGHT = 500;
    HANDLE wHnd;    // Handle to write to the console. (using windows api)
    // Set up the handles for writing:
    wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND consoleWindow = GetConsoleWindow();

    SMALL_RECT windowSize = {0 , 0 , width, 35}; // window size
    COORD coord = {width + 1, HEIGHT}; // buffer size
    SetConsoleWindowInfo(wHnd, TRUE, &windowSize); // set window size
    SetConsoleScreenBufferSize(wHnd, coord); // set buffer size
    ShowScrollBar(consoleWindow, SB_HORZ, 0); // disable horizontal scroll bar

    // disable maximize and minimize button
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
}