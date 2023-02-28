
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
	doubly->cursor = NULL;

	return 0;
}

doublydie_t* droubly_move(doublydie_t* here, doublydie_t* from) {
	if (!from) {
		memset(here, 0, sizeof(*here));
		return NULL;
	}

	here->cursor = from->cursor;
	here->next = from->next;
	here->prev = from->prev;

	here->node_crc = from->node_crc;

	return from;
}

void* doubly_drop(doublydie_t* doubly) {
	doublydie_t* rm = doubly->cursor;
	if (!rm || rm == doubly) {
		void* user = doubly->node_data;
		doublydie_t* discard = droubly_move(doubly, doubly->next);
		if (!discard) apfree(discard);
		else memset(doubly, 0, sizeof(*doubly));
		
		return user;
	}
	
	void* user = rm->node_data;; 
	
	if (!rm->prev)
		memset(rm, 0, sizeof(*rm));
	else rm->prev->next = rm->next;
	
	if (rm->next) rm->next->prev = rm->prev;

	apfree(rm);
	return user;
}

void* doubly_next(doublydie_t* doubly) {
	volatile doublydie_t* cursor = doubly->cursor;
	
	// We're reached at end
	if (cursor == doubly) return NULL;

	if (cursor == NULL && doubly->node_data != NULL) {
		doubly->cursor = doubly;
		return doubly->node_data;
	}

	if (cursor == NULL) return NULL;

	void* ddata = cursor->node_data;
	doubly->cursor = cursor->next;

	return ddata;
}

i32 doubly_insert(void* data, doublydie_t* doubly) {
	doublydie_t* newnode = NULL;
	i32 pos = 0;

	if (doubly->node_data == NULL) {
		newnode = doubly;
		goto attribute;
	}

	while (doubly->next) {
		doubly = doubly->next;
		pos++;
	}

	newnode = (doublydie_t*)apmalloc(sizeof *doubly);
	newnode->next = NULL;

	attribute:
	if (newnode != doubly) {
		newnode->prev = doubly;
		doubly->next = newnode;
	}
		
	newnode->node_data = data;
	newnode->node_crc = cyclic32_checksum(newnode->node_data, sizeof(void*));

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

