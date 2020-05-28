#include<stdio.h>
#include<stdlib.h>

#define HEAP_SIZE 1024

// I used the heap to make the heap
char heap[HEAP_SIZE];

// Structure for block of memory
struct memory_block {
    char * start;
    int size;
    enum {ALLOCATED, FREE, SPLIT} state;
};

// STructure to store blocks in tree format
struct memory_node {
    struct memory_block block;
    struct memory_node * left;
    struct memory_node * right;
};

typedef struct memory_block memory_block;
typedef struct memory_node memory_node;

memory_block new_block(char * start_addr, int size);
void print_block(memory_block m);

int next_higher_power(int n) {
    int m = 1;
    while(m < n) {
        m = m << 1;
    }
    return m;
}

char * get_state(memory_block b) {
    switch(b.state) {
        case 0:
            return "ALLOCATED";
        case 1:
            return "FREE";
        case 2:
            return "SPLIT";
    }
}

memory_node * init_tree() {
    // Set the entire heap to NULL
    int i = 0;
    for(i = 0; i < HEAP_SIZE; i++) {
        heap[i] = 0;
    }

    // Allocate memory for the root node
    memory_node * root = malloc(sizeof(memory_node));
    // Assign to it the root block
    root->block = new_block(heap, HEAP_SIZE);
    // Make it a terminal node
    root->left = NULL;
    root->right = NULL;

    return root;
};

// Print the entire tree in 'VLR' order
void print_tree(memory_node * root);

// Split the parent node into teo
void split(memory_node * parent);

// Caution: Use only on terminal nodes
void merge_children(memory_node * parent);

// collapses all free children of given node
void collapse(memory_node * tree);

int allocate(memory_node * tree, int size, memory_node ** allocated);
int allocate_memory(memory_node * tree, int size, memory_node ** allocated);
void free_node(memory_node * m);

int main() {
    // ITNA DIMAAG NA LAGAYE, KI MANN KA AAPAA KHOYE
    // AURAN KO BHI MARKS NA MILE, SWAYAM KO 'W' HOYE
    // Translation:
    // DO NOT DARE CHANGE CODE WHICH YOU MIGHT NOT UNDERSTAND
    memory_node * root = init_tree();
    memory_node * blocks[5];
    allocate_memory(root, 4, &blocks[0]);
    allocate_memory(root, 128, &blocks[1]); 
    //allocate_memory(root, 105, &blocks[2]);
    //allocate_memory(root, 509, &blocks[3]);
    //allocate_memory(root, 513, &blocks[4]);
    print_tree(root);
    blocks[0]->block.state = FREE;
    /*
    blocks[2]->block.state = FREE;
    blocks[3]->block.state = FREE;
    free_node(blocks[0]);
    free_node(blocks[1]);
    collapse(root);
    printf("\n After Freeing !\n");
    print_tree(root);
    */
   collapse(root);
   printf("After collapse:\n");
   print_tree(root);
    return 0;
}

memory_block new_block(char * start_addr, int size) {
    memory_block m;
    m.start = start_addr;
    m.size = size;
    m.state = FREE;
    return m;
}

void print_block(memory_block m) {
    printf(">>\tBlock start: %p\t Block size: %d\t Block state: %s\n", m.start, m.size, get_state(m));
}

void print_tree(memory_node * root) {
    if (root == NULL) {
        return;
    } else {
        printf(">=========\\\n");
        printf(">>\tNode Address: %p\t Left: %p\t Right: %p\n", root, root->left, root->right);
        print_block(root->block);
        printf(">=========/\n");
        printf("\n");
        print_tree(root->left);
        print_tree(root->right);
    }
}

void split(memory_node * parent) {
    if(parent->left || parent->right) {
        // If parent has left or right node you cannot split it further
        fprintf(stderr, "ERROR: Unable to split node.\n");
    } else {
        // Allocating memory
        memory_node * lc = malloc(sizeof(memory_node));
        memory_node * rc = malloc(sizeof(memory_node));

        // Copying block for easy access to specifics
        memory_block temp = parent->block;
        
        // setting block
        // Left one is basically duplicate
        lc->block = temp;
        lc->block.size = (lc->block.size / 2);
        
        // Set the right and left children to null
        lc->left = NULL;
        lc->right = NULL;

        // The block on the right starts after half a parent block
        rc->block = temp;
        rc->block.size = (rc->block.size / 2);
        rc->block.start = lc->block.start + lc->block.size;
        
        // Set children to null
        rc->left = NULL;
        rc->right = NULL;

        // Set the left and right children pointers
        parent->left = lc;
        parent->right = rc;

        // Set the parent block state to split
        parent->block.state = SPLIT;
    }
}

void merge_children(memory_node * parent) {
    // Free memory allocated to the children
    free(parent->left);
    free(parent->right);

    // Set children to null
    parent->left = NULL;
    parent->right = NULL;
}

void collapse(memory_node * tree) {
    // If you are at a terminal node
    if (tree->left == NULL && tree->right == NULL) {
        // Go back
        return;
    } else {
        // Collapse all to the left and right
        collapse(tree->left);
        collapse(tree->right);
        // If the left child is free and the right child is feee
        if (tree->left->block.state == FREE && tree->right->block.state == FREE) {
            // And the current block is marked as split
            if (tree->block.state == SPLIT) {
                // Mark current block as free
                tree->block.state = FREE;
            }
            // Merge both of its children
            merge_children(tree);
        }
    }
}

int allocate_memory(memory_node * tree, int size, memory_node ** allocated) {
    int status = allocate(tree, size, allocated);
    if (status == 0) {
        printf("+------------------------------------+\n");
        printf("|ERROR: Insufficient memory          |\n");
        printf("|Unable to allocate block of size %4d|\n", size);
        printf("+------------------------------------+\n");
    } else {
        printf("+---------------------+");
        printf("|Allocation Successful|\n");
        printf("+---------------------+");
    }
    return status;
}

int allocate(memory_node * tree, int size, memory_node ** allocated) {
    // get the precise block size to allocate
    int sz = next_higher_power(size);
    // If the given block is smaller than size
    if (tree->block.size < sz) {
        // Return 0 because it cannot be allocated
        return 0;
    } else {
        if (tree->block.size == sz) {
            // If the best fir block is found
            if (tree->block.state == FREE) {
                // Set its state to ALLOCATED and return 1
                *allocated = tree;
                tree->block.state = ALLOCATED;
                return 1;
            } else {
                // Return 0 if a block is found but not free
                return 0;
            }
        } else {
            // If you reach an end node of larger size
            if (tree->left == NULL && tree->right == NULL) {
                // Split the given node in two
                split(tree);
            }
            // If allocation succeeds on the left branch    
            if (allocate(tree->left, size, allocated)) {
                // Return 1
                return 1;
            } else {
                // Try allocating on the right
                return allocate(tree->right, size, allocated);
            }
        }
    }
}

void free_node(memory_node * m) {
    // Set the node status to free
    m->block.state = FREE;
}
