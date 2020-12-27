#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"


void copy_state(state_t* dst, state_t* src){
	
	//Copy field
	memcpy( dst->field, src->field, SIZE*SIZE*sizeof(int8_t) );

	dst->cursor = src->cursor;
	dst->selected = src->selected;
}

/**
 * Saves the path up to the node as the best solution found so far
*/
void save_solution( node_t* solution_node ){
	node_t* n = solution_node;
	while( n->parent != NULL ){
		copy_state( &(solution[n->depth]), &(n->state) );
		solution_moves[n->depth-1] = n->move;

		n = n->parent;
	}
	solution_size = solution_node->depth;
}


node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n->parent = NULL;	
	new_n->depth = 0;
	copy_state(&(new_n->state), init_state);
	return new_n;
}

/**
 * Creates, initialises and returns a new board state node
*/
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action ){
    node_t* new_node = malloc(sizeof(node_t));
	assert(new_node != NULL);
	new_node->parent = n;
	new_node->depth = n->depth + 1;
	new_node->move = action;
	copy_state(&new_node->state, &n->state);
	new_node->state.cursor.x = selected_peg->x;
	new_node->state.cursor.y = selected_peg->y;
    execute_move_t( &(new_node->state), &(new_node->state.cursor), action );
	return new_node;
}

/**
 * Find a solution path as per algorithm description in the handout
 */
void find_solution( state_t* init_state  ){

	HashTable table;

	// Choose initial capacity of PRIME NUMBER 
	// Specify the size of the keys and values you want to store once 
	ht_setup( &table, sizeof(int8_t) * SIZE * SIZE, sizeof(int8_t) * SIZE * SIZE, 16769023);

	// Initialize Stack
	initialize_stack();

	// Add the initial node
	node_t* n = create_init_node( init_state );

	// Initialise free arr, add inital node
	initialise_free_arr();
	add_to_free_arr(n);

	stack_push(n);

	int remaining_pegs = num_pegs(&n->state);

	// explore all possible board states until game is won or budget is exhausted
	while(!is_stack_empty()) {
		n = stack_top();
		stack_pop();
		expanded_nodes += 1;
		if(num_pegs(&n->state) < remaining_pegs) {
			// found a better solution
			save_solution(n);
			remaining_pegs = num_pegs(&n->state);
		}
		if(generate_states(n, &table, &remaining_pegs) == 1) {
			// game is won
			ht_destroy(&table);
			return;
		}
		if(expanded_nodes >= budget) {
			// budget exhausted
			ht_destroy(&table);
			return;
		}
	}
}

/*
 * Generates all next possible board states for given node n, checking for win
 */
int generate_states(node_t *n, HashTable* table, int* remaining_pegs) {
	// x and y for loops to iterate over entire board
	for(int x = 0; x < SIZE; x++) {
		for(int y = 0; y < SIZE; y++) {
			position_s peg;
			peg.x = x;
			peg.y = y;
			// for each board space, check for valid move in all four directions
			for(int d = left; d <= down; d++) {
				if(can_apply(&n->state, &peg, d)) {
					// move is valid, generate new board state
					node_t* new_node = applyAction(n, &peg, d);
					generated_nodes += 1;
					add_to_free_arr(new_node);
					// check if new state is a win
					if(won(&new_node->state)) {
						save_solution(new_node);
						*remaining_pegs = num_pegs(&new_node->state);
						return 1;
					}
					// if new state has not been seen before, push to stack
					if(ht_contains(table, new_node->state.field) == HT_NOT_FOUND) {
						stack_push(new_node);
						ht_insert(table, new_node->state.field, new_node->state.field);
					}
				}
			} 
		}
	}
	return 0;	
}