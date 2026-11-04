#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int allocation_id;
    int start;
    int size;
    int is_free;
} Block;

Block *blocks = NULL;
int block_count = 0;
int block_capacity = 0;
char *memory_pool = NULL;
int pool_size = 0;
int allocation_strategy = 1;
int next_id = 1;
int last_failed_request_size = 0;

void insert_block(int index, Block new_block) {
    if (block_count >= block_capacity) {
        int new_capacity = block_capacity * 2;
        Block *temp_blocks = realloc(blocks, new_capacity * sizeof(Block));
        if (temp_blocks == NULL) {
            printf("realloc failed for metadata array\n");
            exit(1);
        }
        blocks = temp_blocks;
        block_capacity = new_capacity;
    }
    // shift blocks right to fit the new block
    for (int i = block_count; i > index; i--) {
        blocks[i] = blocks[i - 1];
    }
    blocks[index] = new_block;
    block_count++;
}

void merge_free_blocks() {
    for (int i = 0; i < block_count - 1; i++) {
        if (blocks[i].is_free == 1 && blocks[i + 1].is_free == 1) {
            blocks[i].size += blocks[i + 1].size;
            for (int j = i + 1; j < block_count - 1; j++) {
                blocks[j] = blocks[j + 1];
            }
            block_count--;
            i--; // i is decremented to recheck current index if one-more merge is possible
        }
    }
}

void allocate_memory(int requested_size) {
    if (requested_size <= 0 || requested_size > pool_size) {
        printf("Invalid size.\n");
        return;
    }    
    int suitable_block_index = -1;
    for (int i = 0; i < block_count; i++) {
        if (blocks[i].is_free == 1 && blocks[i].size >= requested_size) {
            // if first fit strategy is used we can use the first suitable index right away
            if (allocation_strategy == 1) {
                suitable_block_index = i;
                break;
            }
            // if best fit is useed we check if this block is a better fit than previous match 
            else if (allocation_strategy == 2) {
                if (suitable_block_index == -1 || blocks[i].size < blocks[suitable_block_index].size) {
                    suitable_block_index = i;
                }
            } 
            // if worst fit is useed we check if this block is a worse fit than previous match 
            else if (allocation_strategy == 3) {
                if (suitable_block_index == -1 || blocks[i].size > blocks[suitable_block_index].size) {
                    suitable_block_index = i;
                }
            }
        }
    }    
    if (suitable_block_index == -1) {
        printf("Allocation failed, no contiguous block is large enough.\n");
        last_failed_request_size = requested_size;
        return;
    }    
    int remaining_size = blocks[suitable_block_index].size - requested_size;
    blocks[suitable_block_index].size = requested_size;
    blocks[suitable_block_index].is_free = 0;
    blocks[suitable_block_index].allocation_id = next_id;
    // if block was larger than required size, we have to free the remaining size block
    if (remaining_size > 0) {
        Block free_block;
        free_block.allocation_id = 0;
        free_block.start = blocks[suitable_block_index].start + requested_size;
        free_block.size = remaining_size;
        free_block.is_free = 1;
        insert_block(suitable_block_index + 1, free_block);
    }    
    printf("Allocation successful. ID: %d, Start: %d, Size: %d\n", next_id, blocks[suitable_block_index].start, requested_size);
    next_id++;
    last_failed_request_size = 0; 
}

void release_memory(int allocation_id) {
    int allocation_found = 0;
    for (int i = 0; i < block_count; i++) {
        if (blocks[i].is_free == 0 && blocks[i].allocation_id == allocation_id) {
            blocks[i].is_free = 1;
            blocks[i].allocation_id = 0;
            allocation_found = 1;
            break;
        }
    }
    if (allocation_found == 1) {
        printf("Allocation released.\n");
        merge_free_blocks();
    } else {
        printf("Invalid ID\n");
    }
}

void resize_allocation(int allocation_id, int new_size) {
    if (new_size <= 0 || new_size > pool_size) {
        printf("Invalid size.\n");
        return;
    }
    
    int allocation_block_index = -1;
    for (int i = 0; i < block_count; i++) {
        if (blocks[i].is_free == 0 && blocks[i].allocation_id == allocation_id) {
            allocation_block_index = i;
            break;
        }
    }
    if (allocation_block_index == -1) {
        printf("Invalid ID.\n");
        return;
    }
    int old_size = blocks[allocation_block_index].size;
    if (new_size == old_size) {
        printf("Allocation resize done\n");
        return;
    }
    // case 1st: shrinking of old block
    if (new_size < old_size) {
        int diff = old_size - new_size;
        blocks[allocation_block_index].size = new_size;
        Block free_block;
        free_block.allocation_id = 0;
        free_block.start = blocks[allocation_block_index].start + new_size;
        free_block.size = diff;
        free_block.is_free = 1;
        insert_block(allocation_block_index + 1, free_block);
        merge_free_blocks();
        printf("Allocation resize done\n");
        return;
    }    
    // case 2nd: check if block can be expanded in-place
    int diff = new_size - old_size;
    if (allocation_block_index + 1 < block_count && blocks[allocation_block_index + 1].is_free == 1 && blocks[allocation_block_index + 1].size >= diff) {
        blocks[allocation_block_index].size = new_size;
        blocks[allocation_block_index + 1].size -= diff;
        blocks[allocation_block_index + 1].start += diff;
        if (blocks[allocation_block_index + 1].size == 0) {
            for (int i = allocation_block_index + 1; i < block_count - 1; i++) {
                blocks[i] = blocks[i + 1];
            }
            block_count--;
        }
        printf("Allocation resize done\n");
        return;
    }
    // case 3rd: block has to be relocated to a new block
    int suitable_block_index = -1;
    for (int i = 0; i < block_count; i++) {
        if (blocks[i].is_free == 1 && blocks[i].size >= new_size) {
            if (allocation_strategy == 1) {
                suitable_block_index = i;
                break;
            } else if (allocation_strategy == 2) {
                if (suitable_block_index == -1 || blocks[i].size < blocks[suitable_block_index].size) {
                    suitable_block_index = i;
                }
            } else if (allocation_strategy == 3) {
                if (suitable_block_index == -1 || blocks[i].size > blocks[suitable_block_index].size) {
                    suitable_block_index = i;
                }
            }
        }
    }
    if (suitable_block_index == -1) {
        printf("Resize failed: no contiguous block is large enough.\n");
        last_failed_request_size = new_size; 
        return;
    }
    int new_start = blocks[suitable_block_index].start;
    int old_start = blocks[allocation_block_index].start;
    // copying previous block into new block
    for (int i = 0; i < old_size; i++) {
        memory_pool[new_start + i] = memory_pool[old_start + i];
    }
    int remaining_size = blocks[suitable_block_index].size - new_size;
    blocks[suitable_block_index].size = new_size;
    blocks[suitable_block_index].is_free = 0;
    blocks[suitable_block_index].allocation_id = allocation_id;
    if (remaining_size > 0) {
        Block free_block;
        free_block.allocation_id = 0;
        free_block.start = blocks[suitable_block_index].start + new_size;
        free_block.size = remaining_size;
        free_block.is_free = 1;
        insert_block(suitable_block_index + 1, free_block);
    }
    for (int i = 0; i < block_count; i++) {
        if (blocks[i].start == old_start && blocks[i].is_free == 0 && blocks[i].allocation_id == allocation_id) {
            blocks[i].is_free = 1;
            blocks[i].allocation_id = 0;
            break;
        }
    }
    merge_free_blocks();
    printf("Allocation resize done\n");
}

void display_map() {
    printf("\nMemory Map (%d bytes):\n", pool_size);
    for (int i = 0; i < block_count; i++) {
        if (blocks[i].is_free == 1) {
            printf("[Free: %d bytes, Start: %d]\n", blocks[i].size, blocks[i].start);
        } else {
            printf("[Allocated #%d: %d bytes, Start: %d]\n", blocks[i].allocation_id, blocks[i].size, blocks[i].start);
        }
    }
}

void display_stats() {
    int total_free_memory = 0;
    int total_allocated_memory = 0;
    int free_block_count = 0;
    int allocated_block_count = 0;
    int largest_free_block_size = 0;
    for (int i = 0; i < block_count; i++) {
        if (blocks[i].is_free == 1) {
            total_free_memory += blocks[i].size;
            free_block_count++;
            if (blocks[i].size > largest_free_block_size) {
                largest_free_block_size = blocks[i].size;
            }
        } else {
            total_allocated_memory += blocks[i].size;
            allocated_block_count++;
        }
    }
    printf("\nTotal memory: %d bytes\n", pool_size);
    printf("Allocated: %d bytes\n", total_allocated_memory);
    printf("Total allocated blocks: %d\n",allocated_block_count);
    printf("Free: %d bytes\n", total_free_memory);
    printf("Free blocks: %d\n", free_block_count);
    printf("Largest free block: %d bytes\n", largest_free_block_size);
    // fragmentation check
    if (last_failed_request_size > 0 && last_failed_request_size <= total_free_memory) {
        printf("Note: The request for %d bytes failed even though %d bytes are free, because the memory is fragmented as:- ", last_failed_request_size, total_free_memory);
        
        int first = 1;
        for (int i = 0; i < block_count; i++) {
            if (blocks[i].is_free == 1) {
                if (!first) printf(", ");
                printf("%d bytes", blocks[i].size);
                first = 0;
            }
        }
        printf(".\n");
    }
}

int main() {
    printf("Enter memory pool size: ");
    if (scanf("%d", &pool_size) != 1 || pool_size < 1 || pool_size > 1000000) {
        printf("Invalid pool size.\n");
        return 1;
    }
    memory_pool = (char *)malloc(pool_size);
    if (memory_pool == NULL) {
        printf("Failed to allocate memory pool.\n");
        return 1;
    }
    block_capacity = 10;
    blocks = malloc(block_capacity * sizeof(Block));
    if (blocks == NULL) {
        printf("Failed to allocate metadata array.\n");
        free(memory_pool);
        return 1;
    }    
    blocks[0].allocation_id = 0;
    blocks[0].start = 0;
    blocks[0].size = pool_size;
    blocks[0].is_free = 1;
    block_count = 1;
    printf("Select allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ");
    if (scanf("%d", &allocation_strategy) != 1) {
        allocation_strategy = 1;
    }
    if (allocation_strategy == 1) {
        printf("First Fit selected ...\n");
    } else if (allocation_strategy == 2) {
        printf("Best Fit selected ...\n");
    } else if (allocation_strategy == 3) {
        printf("Worst Fit selected ...\n");
    } else {
        printf("Invalid strategy. Defaulting to First Fit.\n");
        allocation_strategy = 1;
    }
    int choice;
    while (1) {
        printf("\nMenu:\n");
        printf("1. Allocate memory\n");
        printf("2. Release memory\n");
        printf("3. Resize allocation\n");
        printf("4. Display memory map\n");
        printf("5. Display memory statistics\n");
        printf("6. Change allocation strategy\n");
        printf("7. Exit\n");
        printf("Choice: ");
        
        if (scanf("%d", &choice) != 1) {
            break;
        }
        
        switch (choice) {
            case 1: {
                int requested_size;
                printf("Requested size: ");
                scanf("%d", &requested_size);
                allocate_memory(requested_size);
                break;
            }
            case 2: {
                int allocation_id;
                printf("Allocation ID: ");
                scanf("%d", &allocation_id);
                release_memory(allocation_id);
                break;
            }
            case 3: {
                int allocation_id, new_size;
                printf("Allocation ID: ");
                scanf("%d", &allocation_id);
                printf("New size: ");
                scanf("%d", &new_size);
                resize_allocation(allocation_id, new_size);
                break;
            }
            case 4: {
                display_map();
                break;
            }
            case 5: {
                display_stats();
                break;
            }
            case 6: {
                printf("Select allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ");
                scanf("%d", &allocation_strategy);
                if (allocation_strategy == 1) {
                    printf("First Fit selected ...\n");
                } else if (allocation_strategy == 2) {
                    printf("Best Fit selected ...\n");
                } else if (allocation_strategy == 3) {
                    printf("Worst Fit selected ...\n");
                } else {
                    printf("Invalid strategy. Selecting First Fit ...\n");
                    allocation_strategy = 1;
                }
                break;
            }
            case 7: {
                free(memory_pool);
                free(blocks);
                exit(0);
            }
            default: {
                printf("Invalid choice.\n");
                break;
            }
        }
    }
    free(memory_pool);
    free(blocks);
    return 0;
}