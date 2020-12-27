#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "utils.h"
#include "hashtable.h"

void initialize_ai();

void find_solution( state_t* init_state );
int generate_states(node_t *n, HashTable* table, int* remaining_pegs);

#endif
