#include "monkeylist.h"

static struct k_mutex monkey_mutex;
static struct Monkey* head = NULL;

// Function to initialize the Monkey list and mutex
void initMonkeyList() {
    k_mutex_init(&monkey_mutex);
    head = NULL;
}

// Function to append a Monkey node to the linked list or modify existing attributes if the Monkey already exists
void appendOrModifyMonkey(struct Monkey** head_ref, int num, int rssi, int record_time, enum main_state state) {
    k_mutex_lock(&monkey_mutex, K_FOREVER);

    struct Monkey* currentMonkey = *head_ref;

    // Check if a Monkey with the same num already exists
    while (currentMonkey != NULL) {
        if (currentMonkey->num == num) {
            // Monkey with the same num found, modify its attributes
            currentMonkey->rssi = rssi;
            currentMonkey->record_time = record_time;
            currentMonkey->state = state;
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
        newMonkey->next = NULL;

        if (*head_ref == NULL) {
            *head_ref = newMonkey;
            k_mutex_unlock(&monkey_mutex);
            return;
        }

        struct Monkey* lastMonkey = *head_ref;
        while (lastMonkey->next != NULL) {
            lastMonkey = lastMonkey->next;
        }
        lastMonkey->next = newMonkey;
    }

    k_mutex_unlock(&monkey_mutex);
}

// Function to remove a Monkey with a specific num from the linked list
void removeMonkey(struct Monkey** head_ref, int num) {
    k_mutex_lock(&monkey_mutex, K_FOREVER);

    struct Monkey* currentMonkey = *head_ref;
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
            *head_ref = currentMonkey->next;
        } else {
            prevMonkey->next = currentMonkey->next;
        }
        // Free the memory allocated for the Monkey
        k_heap_free(&monkeyListHeap,currentMonkey);
    }

    k_mutex_unlock(&monkey_mutex);
}

// Function to print the linked list of Monkeys
void printMonkeys(struct Monkey* monkeyList) {
    k_mutex_lock(&monkey_mutex, K_FOREVER);

    while (monkeyList != NULL) {
        printk("Monkey: num=%d, rssi=%d, record_time=%d, state=%d\n", 
               monkeyList->num, monkeyList->rssi, monkeyList->record_time, monkeyList->state);
        monkeyList = monkeyList->next;
    }

    k_mutex_unlock(&monkey_mutex);
}
