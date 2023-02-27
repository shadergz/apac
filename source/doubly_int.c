
#include <string.h>

#include <memctrlext.h>
#include <doubly_int.h>

#include <cyclic_hw.h>

i32 doubly_init(doublydie_t* doubly) {
	if (doubly == NULL) return -1;
	memset(doubly, 0, sizeof *doubly);

	doubly->prev = NULL;

	return 0;
}

i32 doubly_reset(doublydie_t* doubly) {
	if (doubly == NULL) return -1;
	doubly->cursor = doubly;

	return 0;
}

void* doubly_next(doublydie_t* doubly) {
	doublydie_t* cursor = doubly->cursor;

	void* ddata = cursor->node_data;
	doubly->cursor = cursor->next;

	return ddata;
}

i32 doubly_insert(void* data, doublydie_t* doubly) {
	doublydie_t* new_node = apmalloc(sizeof *doubly);
	if (new_node == NULL || doubly->prev != NULL) return -1;

	if (doubly->node_data == NULL) {
		doubly->node_data = data;
		return 0;
	} else if (doubly->next) doubly = doubly->next;

	new_node->next = NULL;
	new_node->node_data = data;

	i32 pos = 1; 
	while (doubly->next) {
		doubly = doubly->next;
		pos++;
	}

	new_node->prev = doubly;
	
	doubly->next = new_node;
	doubly->node_crc = cyclic32_checksum(new_node->node_data, sizeof(void*));

	return pos;
}

i32 doubly_deinit(doublydie_t* doubly) {
	if (doubly == NULL) return -1;
	if (doubly->prev != NULL) return -1;

	i32 destroyed = 0;
	doubly = doubly->next;
	while (doubly) {
		doublydie_t* next = NULL;
		next = doubly->next;
		apfree(doubly);
		doubly = next;
		destroyed++;
	}

	return destroyed;
}

i32 doubly_rm(void* data, doublydie_t* doubly) {
	if (doubly->node_data == data && doubly->prev == NULL) {
		doubly->node_data = NULL;
		// We're inside the root node, we can't deallocate it!
		if (doubly->next == NULL) return 0;
		// Removing the next node
		doubly->node_data = doubly->next->node_data;
		doublydie_t* fast_delete = doubly->next;

		doubly->next = doubly->next->next;
		doubly->next->prev = doubly;
		apfree(fast_delete);
	}

	i32 pos = 0;
	while ((doubly = doubly->next)) {
		if (doubly->node_data == data) break;
		pos++;
	}
	if (doubly == NULL) return -1;
	if (doubly->prev)
		doubly->prev->next = doubly->next;
	if (doubly->next)
		doubly->next->prev = doubly->prev;

	return pos;
}

