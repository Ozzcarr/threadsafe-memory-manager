#include "linked_list.h"


pthread_rwlock_t list_lock;

/**
 * @brief Initializes the linked list.
 *
 * @param head A double pointer to the head of the linked list.
 * @param size The size in bytes to allocate.
 */
void list_init(Node **head, size_t size) {
    mem_init(size);
    *head = NULL;
    pthread_rwlock_init(&list_lock, NULL);
}

/**
 * @brief Inserts the specified data at the end of the linked list.
 *
 * @param head A double pointer to the head of the linked list.
 * @param data The data to insert into the linked list.
 */
void list_insert(Node **head, uint16_t data) {

    Node *new_node = mem_alloc(sizeof(Node));
    if (!new_node) {
        // printf_red("Memory allocation for insertion failed!\n");
        return;
    }

    pthread_rwlock_wrlock(&list_lock);

    if (*head == NULL)
        *head = new_node;
    else {
        Node *current = *head;
        while (current->next != NULL) current = current->next;
        current->next = new_node;
    }
    new_node->data = data;
    new_node->next = NULL;

    pthread_rwlock_unlock(&list_lock);
}

/**
 * @brief Inserts the specified data after the given node.
 *
 * @param prev_node A pointer to the node to insert data after.
 * @param data The data to insert into the linked list.
 */
void list_insert_after(Node *prev_node, uint16_t data) {
    if (!prev_node) {
        // printf_red("Previous node is null!\n");
        return;
    }

    pthread_rwlock_wrlock(&list_lock);

    Node *next_node = prev_node->next;
    Node *new_node = mem_alloc(sizeof(Node));
    if (!new_node) {
        pthread_rwlock_unlock(&list_lock);
        // printf_red("Memory allocation for insertion after failed!\n");
        return;
    }
    new_node->data = data;
    prev_node->next = new_node;
    new_node->next = next_node;

    pthread_rwlock_unlock(&list_lock);
}

/**
 * @brief Inserts the specified data before the given node.
 *
 * @param head A double pointer to the head of the linked list.
 * @param prev_node A pointer to the node to insert data before.
 * @param data The data to insert into the linked list.
 */
void list_insert_before(Node **head, Node *next_node, uint16_t data) {
    if (*head == NULL || !next_node) return;

    pthread_rwlock_wrlock(&list_lock);

    Node *new_node = mem_alloc(sizeof(Node));
    if (!new_node) {
        pthread_rwlock_unlock(&list_lock);
        // printf_red("Memory allocation for insertion before failed!\n");
        return;
    }

    new_node->data = data;
    new_node->next = next_node;

    // If insert before the head node.
    if (next_node == *head) {
        *head = new_node;
        pthread_rwlock_unlock(&list_lock);
        return;
    }

    // Insert before a general node.
    Node *current = *head;
    while (current && current->next != next_node) current = current->next;
    if (current) current->next = new_node;

    pthread_rwlock_unlock(&list_lock);
}

/**
 * @brief Removes a node with the specified data from the linked list.
 *
 * @param head A double pointer to the head of the linked list.
 * @param data The data to insert into the linked list.
 */
void list_delete(Node **head, uint16_t data) {
    if (*head == NULL) return;

    pthread_rwlock_wrlock(&list_lock);

    // If the data is on the first node.
    if ((*head)->data == data) {
        Node *temp = *head;
        *head = temp->next;
        mem_free(temp);
        pthread_rwlock_unlock(&list_lock);
        return;
    }

    // If the data is on a general node.
    Node *current = *head;
    while (current->next) {
        if (current->next->data == data) {
            Node *temp = current->next;
            current->next = temp->next;
            mem_free(temp);
            pthread_rwlock_unlock(&list_lock);
            return;
        }
        current = current->next;
    }

    pthread_rwlock_unlock(&list_lock);
}

/**
 * @brief Searches for a node with the specified data and returns a pointer to
 * it.
 *
 * @param head A double pointer to the head of the linked list.
 * @param data The data to insert into the linked list.
 * @return A pointer to the returned node.
 */
Node *list_search(Node **head, uint16_t data) {
    pthread_rwlock_rdlock(&list_lock);

    Node *current = *head;
    while (current) {
        if (current->data == data) {
            pthread_rwlock_unlock(&list_lock);
            return current;
        }
        current = current->next;
    }

    pthread_rwlock_unlock(&list_lock);
    return NULL;
}

/**
 * @brief Prints all elements of the list.
 *
 * @param head A double pointer to the head of the list.
 */
void list_display(Node **head) { list_display_range(head, NULL, NULL); }

/**
 * @brief Prints all elements of the list between two nodes (inclusive).
 *
 * @param head A double pointer to the head of the linked list.
 * @param start_node A pointer to the start node (NULL for start of linked
 * list).
 * @param end_node A pointer to the end node (NULL for end of linked list).
 */
void list_display_range(Node **head, Node *start_node, Node *end_node) {
    pthread_rwlock_rdlock(&list_lock);

    printf("[");
    if (!start_node) start_node = *head;
    if (end_node) end_node = end_node->next;
    while (start_node && start_node != end_node) {
        printf("%d", start_node->data);
        start_node = start_node->next;
        if (start_node && start_node != end_node) printf(", ");
    }
    printf("]");

    pthread_rwlock_unlock(&list_lock);
}

/**
 * @brief Counts the number of nodes in the linked list.
 *
 * @param head A double pointer to the head of the linked list.
 * @return The number of nodes in the linked list.
 */
int list_count_nodes(Node **head) {
    pthread_rwlock_rdlock(&list_lock);

    if (*head == NULL) {
        pthread_rwlock_unlock(&list_lock);
        return 0;
    }
    int count = 0;

    Node *current = *head;
    while (current) {
        count++;
        current = current->next;
    }

    pthread_rwlock_unlock(&list_lock);
    return count;
}

/**
 * @brief Frees all the nodes in the linked list.
 *
 * @param head A double pointer to the head of the linked list.
 */
void list_cleanup(Node **head) {
    pthread_rwlock_wrlock(&list_lock);

    Node *current = *head;
    while (current) {
        Node *temp = current;
        current = current->next;
        mem_free(temp);
    }
    *head = NULL;
    mem_deinit();

    pthread_rwlock_unlock(&list_lock);
    pthread_rwlock_destroy(&list_lock);
}
