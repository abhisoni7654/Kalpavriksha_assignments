#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_BLOCKS 1024
#define BLOCK_SIZE 512
#define MAX_NAME 50
#define MAX_BLOCKS_PER_FILE 100

// tracks available disk blocks
struct FreeBlock {
    int blockIndex;
    struct FreeBlock* next;
    struct FreeBlock* prev;
};

// stores files and directories in a circular list
struct FileNode {
    char name[MAX_NAME + 1];
    int isDirectory; // 1 = directory, 0 = file
    int size;        // file size in bytes
    int blockCount;  // allocated block count
    int blockPointers[MAX_BLOCKS_PER_FILE]; 
    
    struct FileNode* parent;
    struct FileNode* childHead; // first child in circular list
    struct FileNode* next;      
};

char virtualDisk[NUM_BLOCKS][BLOCK_SIZE]; 
struct FreeBlock* freeHead = NULL;
struct FreeBlock* freeTail = NULL;
int usedBlocks = 0;

struct FileNode* root = NULL;
struct FileNode* cwd = NULL;

// free block management

void initFreeBlocks() {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        struct FreeBlock* newBlock = malloc(sizeof(struct FreeBlock));
        newBlock->blockIndex = i;
        newBlock->next = NULL;
        newBlock->prev = freeTail;

        if (freeTail == NULL) {
            freeHead = newBlock;
        } else {
            freeTail->next = newBlock;
        }
        freeTail = newBlock;
    }
}

int allocateBlock() {
    if (freeHead == NULL) return -1; // no free blocks left
    
    struct FreeBlock* temp = freeHead;
    int idx = temp->blockIndex;
    
    freeHead = freeHead->next;
    if (freeHead != NULL) {
        freeHead->prev = NULL;
    } else {
        freeTail = NULL; // list is now empty
    }
    
    free(temp);
    usedBlocks++;
    return idx;
}

void releaseBlock(int idx) {
    struct FreeBlock* newBlock = malloc(sizeof(struct FreeBlock));
    newBlock->blockIndex = idx;
    newBlock->next = NULL;
    newBlock->prev = freeTail;

    if (freeTail != NULL) {
        freeTail->next = newBlock;
    } else {
        freeHead = newBlock;
    }
    freeTail = newBlock;
    usedBlocks--;
}

void initFileSystem() {
    initFreeBlocks();
    root = malloc(sizeof(struct FileNode));
    strcpy(root->name, "/");
    root->isDirectory = 1;
    root->size = 0;
    root->blockCount = 0;
    root->parent = NULL;
    root->childHead = NULL;
    root->next = root; // root points to itself
    cwd = root;
}

void insertChild(struct FileNode* parent, struct FileNode* child) {
    if (parent->childHead == NULL) {
        parent->childHead = child;
        child->next = child;
    } else {
        struct FileNode* last = parent->childHead;
        while (last->next != parent->childHead) {
            last = last->next;
        }
        last->next = child;
        child->next = parent->childHead;
    }
}

void removeChild(struct FileNode* parent, struct FileNode* target) {
    if (parent->childHead == NULL) return;
    struct FileNode* curr = parent->childHead;
    struct FileNode* prev = NULL;
    struct FileNode* last = parent->childHead;
    // keep track of the last node for head updates
    while (last->next != parent->childHead) {
        last = last->next;
    }
    do {
        if (curr == target) {
            if (curr->next == curr) {
                // this is the only child
                parent->childHead = NULL;
            } else {
                if (curr == parent->childHead) {
                    // update the head
                    parent->childHead = curr->next;
                    last->next = parent->childHead;
                } else {
                    // skip the node that's being removed
                    prev->next = curr->next;
                }
            }
            break;
        }
        prev = curr;
        curr = curr->next;
    } while (curr != parent->childHead);
}

struct FileNode* findChild(struct FileNode* parent, char* name, int isDirectory) {
    if (parent->childHead == NULL) return NULL;
    struct FileNode* curr = parent->childHead;
    do {
        if (strcmp(curr->name, name) == 0 && curr->isDirectory == isDirectory) {
            return curr;
        }
        curr = curr->next;
    } while (curr != parent->childHead);
    return NULL;
}

void makeDirectory(char* name) {
    if (findChild(cwd, name, 1) != NULL) {
        printf("This directory already exists\n");
        return;
    }
    
    struct FileNode* newDir = malloc(sizeof(struct FileNode));
    strcpy(newDir->name, name);
    newDir->isDirectory = 1;
    newDir->size = 0;
    newDir->blockCount = 0;
    newDir->parent = cwd;
    newDir->childHead = NULL;
    
    insertChild(cwd, newDir);
    printf("Directory '%s' creation successful.\n", name);
}

void createFile(char* name) {
    if (findChild(cwd, name, 0) != NULL) {
        printf("This file already exists.\n");
        return;
    }
    struct FileNode* newFile = malloc(sizeof(struct FileNode));
    strcpy(newFile->name, name);
    newFile->isDirectory = 0;
    newFile->size = 0;
    newFile->blockCount = 0;
    newFile->parent = cwd;
    newFile->childHead = NULL;
    insertChild(cwd, newFile);
    printf("File '%s' creation successful.\n", name);
}

void writeToFile(char* name, char* data) {
    struct FileNode* file = findChild(cwd, name, 0);
    if (file == NULL) {
        printf("File '%s' not found.\n", name);
        return;
    }
    int dataLen = strlen(data);
    int requiredBlocks = (dataLen + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (requiredBlocks == 0) requiredBlocks = 1; // reserve one block for an empty file
    if (NUM_BLOCKS - usedBlocks < requiredBlocks) {
        printf("Disk is full. Cannot write file.\n");
        return;
    }
    // release old blocks before overwriting
    for (int i = 0; i < file->blockCount; i++) {
        releaseBlock(file->blockPointers[i]);
    }
    file->blockCount = 0;
    file->size = dataLen;
    int bytesWritten = 0;
    for (int i = 0; i < requiredBlocks; i++) {
        int blk = allocateBlock();
        file->blockPointers[file->blockCount++] = blk;
        
        int chunk = (dataLen - bytesWritten > BLOCK_SIZE) ? BLOCK_SIZE : (dataLen - bytesWritten);
        strncpy(virtualDisk[blk], data + bytesWritten, chunk);
        bytesWritten += chunk;
    }
    printf("Data write successful (size=%d bytes).\n", dataLen);
}

void readFromFile(char* name) {
    struct FileNode* file = findChild(cwd, name, 0);
    if (file == NULL) {
        printf("File '%s' not found.\n", name);
        return;
    }
    if (file->size == 0 || file->blockCount == 0) {
        printf("(empty file)\n");
        return;
    }
    int bytesRead = 0;
    for (int i = 0; i < file->blockCount; i++) {
        int blk = file->blockPointers[i];
        int chunk = (file->size - bytesRead > BLOCK_SIZE) ? BLOCK_SIZE : (file->size - bytesRead);
            for(int j = 0; j < chunk; j++) {
            printf("%c", virtualDisk[blk][j]);
        }
        bytesRead += chunk;
    }
    printf("\n");
}

void deleteFile(char* name) {
    struct FileNode* file = findChild(cwd, name, 0);
    if (file == NULL) {
        printf("File '%s' not found.\n", name);
        return;
    }
    for (int i = 0; i < file->blockCount; i++) {
        releaseBlock(file->blockPointers[i]);
    }
    removeChild(cwd, file);
    free(file);
    printf("File deletion successful\n");
}

void removeDirectory(char* name) {
    struct FileNode* dir = findChild(cwd, name, 1);
    if (dir == NULL) {
        printf("This directory does not exist\n");
        return;
    }
    if (dir->childHead != NULL) {
        printf("Directory is not empty.\n");
        return;
    }
    removeChild(cwd, dir);
    free(dir);
    printf("Directory removal successful\n");
}

void listDirectory() {
    if (cwd->childHead == NULL) {
        printf("(empty)\n");
        return;
    }
    struct FileNode* curr = cwd->childHead;
    do {
        if (curr->isDirectory) {
            printf("%s/\n", curr->name);
        } else {
            printf("%s\n", curr->name);
        }
        curr = curr->next;
    } while (curr != cwd->childHead);
}

void changeDirectory(char* name) {
    if (strcmp(name, "..") == 0) {
        if (cwd->parent != NULL) {
            cwd = cwd->parent;
            printf("Moved to parent directory.\n");
        } else {
            printf("Already at root directory.\n");
        }
        return;
    }
    struct FileNode* dir = findChild(cwd, name, 1);
    if (dir != NULL) {
        cwd = dir;
        printf("Moved to /%s\n", name);
    } else {
        printf("Directory '%s' not found.\n", name);
    }
}

// recursive path printing from  root
void printPath(struct FileNode* node) {
    if (node->parent != NULL) {
        printPath(node->parent);
        if (node->parent != root) printf("/");
        printf("%s", node->name);
    } else {
        printf("/");
    }
}

void showDiskFree() {
    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", usedBlocks);
    printf("Free Blocks: %d\n", NUM_BLOCKS - usedBlocks);
    printf("Disk Usage: %.2f%%\n", ((float)usedBlocks / NUM_BLOCKS) * 100);
}

// file system memory freeing commands
void freeFileTree(struct FileNode* node) {
    if (node == NULL) return;
    if (node->childHead != NULL) {
        struct FileNode* curr = node->childHead;
        struct FileNode* last = node->childHead;
        while (last->next != node->childHead) {
            last = last->next;
        }
        last->next = NULL; // break circular link before freeing
        while (curr != NULL) {
            struct FileNode* nextChild = curr->next;
            freeFileTree(curr);
            curr = nextChild;
        }
    }
    free(node);
}

void freeFreeBlocks() {
    struct FreeBlock* curr = freeHead;
    while (curr != NULL) {
        struct FreeBlock* nextBlock = curr->next;
        free(curr);
        curr = nextBlock;
    }
    freeHead = NULL;
    freeTail = NULL;
}

void cleanupFileSystem() {
    freeFileTree(root);
    root = NULL;
    cwd = NULL;
    freeFreeBlocks();
}
int main() {
    initFileSystem();
    printf("|||| Virtual file system |||| Type 'exit' to quit ||||\n");
    char input[256];
    while (1) {
        // show the current directory
        if (cwd == root) printf("/ > ");
        else printf("%s > ", cwd->name);
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;
        char* cmd = strtok(input, " ");
        if (cmd == NULL) continue;
        char* targetName = strtok(NULL, " ");
        char* fileData = strtok(NULL, ""); 
        // comparing command to run matching command
        if (strcmp(cmd, "mkdir") == 0 && targetName) makeDirectory(targetName);
        else if (strcmp(cmd, "create") == 0 && targetName) createFile(targetName);
        else if (strcmp(cmd, "write") == 0 && targetName && fileData) {
            // remove quotes around file data
            if (fileData[0] == '"' || fileData[0] == '\'') fileData++;
            int len = strlen(fileData);
            if (len > 0 && (fileData[len-1] == '"' || fileData[len-1] == '\'')) fileData[len-1] = '\0';
            writeToFile(targetName, fileData);
        }
        else if (strcmp(cmd, "read") == 0 && targetName) readFromFile(targetName);
        else if (strcmp(cmd, "delete") == 0 && targetName) deleteFile(targetName);
        else if (strcmp(cmd, "rmdir") == 0 && targetName) removeDirectory(targetName);
        else if (strcmp(cmd, "ls") == 0) listDirectory();
        else if (strcmp(cmd, "cd") == 0 && targetName) changeDirectory(targetName);
        else if (strcmp(cmd, "pwd") == 0) {
            printPath(cwd);
            printf("\n");
        }
        else if (strcmp(cmd, "df") == 0) showDiskFree();
        else if (strcmp(cmd, "exit") == 0) {
            cleanupFileSystem();
            printf("Memory released. Exiting the program\n");
            break;
        } else {
            printf("Invalid command, try again\n");
        }
    }
    return 0;
}