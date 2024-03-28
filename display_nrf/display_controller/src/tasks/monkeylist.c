#include "monkeylist.h"

static struct k_mutex monkey_mutex;
static struct Monkey* head = NULL;

// Global variable to hold the head of the monkey list
struct Monkey* head_ref = NULL;

// Function to initialize the Monkey list and mutex
void initMonkeyList() {
    k_mutex_init(&monkey_mutex);
    head = NULL;
}

// Function to append a Monkey node to the linked list or modify existing attributes if the Monkey already exists
void appendOrModifyMonkey(int num, int rssi, int record_time, enum main_state state, bt_addr_le_t address) {
    k_mutex_lock(&monkey_mutex, K_FOREVER);

    struct Monkey* currentMonkey = head_ref;

    // Check if a Monkey with the same num already exists
    while (currentMonkey != NULL) {
        if (currentMonkey->num == num) {
            // Monkey with the same num found, modify its attributes
            currentMonkey->rssi = rssi;
            currentMonkey->record_time = record_time;
            currentMonkey->state = state;
            currentMonkey->btAddress = address;
            k_mutex_unlock(&monkey_mutex);
            return;
        }
        currentMonkey = currentMonkey->next;
    }

    // No Monkey with the same num found, create and append a new Monkey to the end of the list
    struct Monkey* newMonkey = k_heap_alloc(&monkeyListHeap,sizeof(struct Monkey), K_NO_WAIT);
    if (newMonkey != NULL) {
        newMonkey->num = num;
        newMonkey->rssi = rssi;
        newMonkey->record_time = record_time;
        newMonkey->state = state;
        newMonkey->btAddress = address;
        newMonkey->next = NULL;

        if (head_ref == NULL) {
            head_ref = newMonkey;
            k_mutex_unlock(&monkey_mutex);
            return;
        }

        struct Monkey* lastMonkey = head_ref;
        while (lastMonkey->next != NULL) {
            lastMonkey = lastMonkey->next;
        }
        lastMonkey->next = newMonkey;
    }

    k_mutex_unlock(&monkey_mutex);
}

// Function to remove a Monkey with a specific num from the linked list
void removeMonkey(int num) {
    k_mutex_lock(&monkey_mutex, K_FOREVER);

    struct Monkey* currentMonkey = head_ref;
    struct Monkey* prevMonkey = NULL;

    // Traverse the list to find the Monkey with the given num
    while (currentMonkey != NULL && currentMonkey->num != num) {
        prevMonkey = currentMonkey;
        currentMonkey = currentMonkey->next;
    }

    // If the Monkey with the given num is found
    if (currentMonkey != NULL) {
        // If the Monkey to be removed is the head
        if (prevMonkey == NULL) {
            head_ref = currentMonkey->next;
        } else {
            prevMonkey->next = currentMonkey->next;
        }
        // Free the memory allocated for the Monkey
        k_heap_free(&monkeyListHeap,currentMonkey);
    }

    k_mutex_unlock(&monkey_mutex);
}

// Function to print the linked list of Monkeys
void printMonkeys() {
    k_mutex_lock(&monkey_mutex, K_FOREVER);
    struct Monkey* monkeyList = head_ref;
    while (monkeyList != NULL) {
        printk("Monkey: num=%d, rssi=%d, record_time=%d, state=%d\n", 
               monkeyList->num, monkeyList->rssi, monkeyList->record_time, monkeyList->state);
        monkeyList = monkeyList->next;
    }

    k_mutex_unlock(&monkey_mutex);
}

// Function to get an array of all the monkeys and the total number of monkeys
void getAllMonkeys(struct Monkey* monkeysArray) {
    k_mutex_lock(&monkey_mutex, K_FOREVER);
    struct Monkey* monkeyList = head_ref;
    int count = 0;

    // Copy monkeys to the array
    while (monkeyList != NULL) {
        monkeysArray[count++] = *monkeyList;
        monkeyList = monkeyList->next;
    }

    k_mutex_unlock(&monkey_mutex);
}

// Function to get a single monkey from the list at a specific index
bool getMonkeyAtIndex(struct Monkey* monkey, int index) {
    k_mutex_lock(&monkey_mutex, K_FOREVER);
    struct Monkey* monkeyList = head_ref;
    int count = 0;

    // Traverse the list to find the monkey at the specified index
    while (monkeyList != NULL && count < index) {
        count++;
        monkeyList = monkeyList->next;
    }

    // If the monkey at the specified index is found
    if (count == index && monkeyList != NULL) {
        *monkey = *monkeyList; // Copy the monkey data
        k_mutex_unlock(&monkey_mutex);
        return true;
    }

    k_mutex_unlock(&monkey_mutex);
    return false; // Monkey not found at the specified index
}

// Function to get a single monkey from the list by its device id
bool getMonkeyByID(struct Monkey* monkey, int id)
{
    k_mutex_lock(&monkey_mutex, K_FOREVER);
    struct Monkey* monkeyList = head_ref;

    // Copy monkeys to the array
    while (monkeyList != NULL) {
        if(monkeyList->num == id)
            *monkey = *monkeyList;
        monkeyList = monkeyList->next;
    }

    k_mutex_unlock(&monkey_mutex);
}


// Function to get three monkeys from the list starting from a specific index
// Returns true if monkeys are retrieved successfully, false otherwise
bool getThreeMonkeys(struct Monkey* monkeys, int startIndex) {
    k_mutex_lock(&monkey_mutex, K_FOREVER);
    struct Monkey* monkeyList = head_ref;
    int count = 0;

    // Traverse the list to find the starting index
    while (monkeyList != NULL && count < startIndex) {
        count++;
        monkeyList = monkeyList->next;
    }

    // If the starting index is found
    if (count == startIndex && monkeyList != NULL) {
        // Copy the data of three monkeys if available
        for (int i = 0; i < 3 && monkeyList != NULL; i++) {
            monkeys[i] = *monkeyList;
            monkeyList = monkeyList->next;
        }
        k_mutex_unlock(&monkey_mutex);
        return true;
    }

    k_mutex_unlock(&monkey_mutex);
    return false; // Starting index not found or insufficient monkeys after the starting index
}

// Function to get the number of monkeys in the list
int getNumMonkeys() {
    k_mutex_lock(&monkey_mutex, K_FOREVER);
    struct Monkey* monkeyList = head_ref;
    int count = 0;

    // Count the number of monkeys in the list
    while (monkeyList != NULL) {
        count++;
        monkeyList = monkeyList->next;
    }

    k_mutex_unlock(&monkey_mutex);
    return count;
}

