#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 1000
#define MAX_STR_LEN 128
#define MAX_LINE_LEN 256
#define MIN_CAP 1
#define MAX_CAP 1000

typedef struct Node {
    int key;
    char *data;
    struct Node *prev;
    struct Node *next;
    // separate chain for hash collisions
    struct Node *hashNext;
} Node;

typedef struct LRUCache {
    int capacity;
    int size;
    Node *head;
    Node *tail;
    // hash table using separate chaining
    Node *buckets[HASH_SIZE];
} LRUCache;

// keep negative keys within valid bucket range
int hashKey(int key) {
    int idx = key % HASH_SIZE;
    if (idx < 0) {
        idx += HASH_SIZE;
    }
    return idx;
}

void printQueue(LRUCache *cache) {
    printf("Queue (MRU -> LRU): ");
    Node *curr = cache->head;
    while (curr != NULL) {
        printf("(%d:%s)", curr->key, curr->data);
        if (curr->next != NULL) {
            printf(" <-> ");
        }
        curr = curr->next;
    }
    printf("\n");
}

Node *mapGet(LRUCache *cache, int key) {
    int idx = hashKey(key);
    Node *curr = cache->buckets[idx];
    while (curr != NULL) {
        if (curr->key == key) {
            return curr;
        }
        curr = curr->hashNext;
    }
    return NULL;
}

void mapPut(LRUCache *cache, Node *node) {
    int idx = hashKey(node->key);
    node->hashNext = cache->buckets[idx];
    cache->buckets[idx] = node;
}

void mapRemove(LRUCache *cache, int key) {
    int idx = hashKey(key);
    Node *curr = cache->buckets[idx];
    Node *prev = NULL;
    while (curr != NULL) {
        if (curr->key == key) {
            if (prev == NULL) {
                cache->buckets[idx] = curr->hashNext;
            } else {
                prev->hashNext = curr->hashNext;
            }
            curr->hashNext = NULL;
            return;
        }
        prev = curr;
        curr = curr->hashNext;
    }
}

void addToFront(LRUCache *cache, Node *node) {
    // mark this node as the most recently used
    node->prev = NULL;
    node->next = cache->head;
    if (cache->head != NULL) {
        cache->head->prev = node;
    } else {
        cache->tail = node;
    }
    cache->head = node;
}

void removeFromQueue(LRUCache *cache, Node *node) {
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        cache->head = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        cache->tail = node->prev;
    }
    node->prev = NULL;
    node->next = NULL;
}

void moveToFront(LRUCache *cache, Node *node) {
    // update the usage order after an access
    if (cache->head == node) {
        return;
    }
    removeFromQueue(cache, node);
    addToFront(cache, node);
}

// the tail holds the least recently used node
Node *removeTail(LRUCache *cache) {
    if (cache->tail == NULL) {
        return NULL;
    }
    Node *lruNode = cache->tail;
    removeFromQueue(cache, lruNode);
    return lruNode;
}

LRUCache *createCache(int capacity) {
    LRUCache *cache = (LRUCache *)malloc(sizeof(LRUCache));
    if (cache == NULL) {
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;
    for (int i = 0; i < HASH_SIZE; i++) {
        cache->buckets[i] = NULL;
    }
    printf("Cache created.\n");
    return cache;
}

char *get(LRUCache *cache, int key) {
    if (cache == NULL) {
        return NULL;
    }
    Node *node = mapGet(cache, key);
    if (node == NULL) {
        printf("not found\n");
        return NULL;
    }
    // a successful access makes this key most recently used
    moveToFront(cache, node);
    printf("found\n");

    return node->data;
}

void put(LRUCache *cache, int key, const char *val) {
    if (cache == NULL || cache->capacity <= 0) {
        return;
    }
    Node *existing = mapGet(cache, key);
    if (existing != NULL) {
        free(existing->data);
        existing->data = strdup(val);
        moveToFront(cache, existing);

        printf("Updated existing node.\n");
        printQueue(cache);
        return;
    }
    if (cache->size == cache->capacity) {
        // make room by removing the least recently used entry
        Node *lruNode = removeTail(cache);
        if (lruNode != NULL) {
            printf("EVICT -> Removed LRU key %d (%s)\n",lruNode->key, lruNode->data);
            mapRemove(cache, lruNode->key);
            free(lruNode->data);
            free(lruNode);
            cache->size--;
        }
    }
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }
    newNode->key = key;
    newNode->data = strdup(val);
    newNode->prev = NULL;
    newNode->next = NULL;
    newNode->hashNext = NULL;
    addToFront(cache, newNode);
    mapPut(cache, newNode);
    cache->size++;
    printf("Inserted.\n");
    printQueue(cache);
}

void freeCache(LRUCache *cache) {
    if (cache == NULL) {
        return;
    }
    Node *curr = cache->head;
    while (curr != NULL) {
        Node *next = curr->next;
        free(curr->data);
        free(curr);
        curr = next;
    }
    free(cache);
}

int main(void) {
    LRUCache *cache = NULL;
    char line[MAX_LINE_LEN];
    char cmd[32];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (sscanf(line, "%31s", cmd) != 1) {
            continue;
        }
        if (strcmp(cmd, "createCache") == 0) {
            int capacity;
            if (sscanf(line, "%*31s %d", &capacity) != 1) {
                printf("Invalid input. Correct syntax: createCache <size>\n");
                continue;
            }
            if (capacity < MIN_CAP || capacity > MAX_CAP) {
                printf("Invalid capacity. Must be between %d and %d.\n",MIN_CAP, MAX_CAP);
                continue;
            }
            if (cache != NULL) {
                freeCache(cache);
            }
            cache = createCache(capacity);
        } else if (strcmp(cmd, "put") == 0) {
            if (cache == NULL) {
                printf("Create cache first using createCache <size>.\n");
                continue;
            }
            int key;
            char val[MAX_STR_LEN];
            if (sscanf(line, "%*31s %d %127s", &key, val) != 2) {
                printf("Invalid input. Correct syntax: put <key> <data>\n");
                continue;
            }
            put(cache, key, val);
        } else if (strcmp(cmd, "get") == 0) {
            if (cache == NULL) {
                printf("Create cache first using createCache <size>.\n");
                continue;
            }
            int key;
            if (sscanf(line, "%*31s %d", &key) != 1) {
                printf("Invalid input. Correct syntax: get <key>\n");
                continue;
            }
            char *result = get(cache, key);
            if (result != NULL) {
                printf("%s\n", result);
                printQueue(cache);
            } else {
                printf("NULL\n");
            }
        } else if (strcmp(cmd, "exit") == 0) {
            printf("Exiting.....\n");
            break;
        } else {
            printf("Unknown command '%s'. Use: createCache, put, get, exit.\n",cmd);
        }
    }
    freeCache(cache);
    return 0;
}