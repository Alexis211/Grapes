#include <gc/shm.h>
#include <gc/mem.h>
#include <mutex.h>

struct shm_block {
	size_t start, size;
	int is_hole;	//1 : hole, 0 : used block
	struct shm_block *prev, *next;
};

struct shm_block *blocks = 0;

static uint32_t tMutex = MUTEX_UNLOCKED;

static void shm_init() {
	struct shm_block *b = malloc(sizeof(struct shm_block));
	b->start = 0x80000000;
	b->size = 0xD0000000 - 0x80000000;
	b->is_hole = 1;
	b->prev = b->next = 0;
	blocks = b;
}

void* shm_alloc(size_t size) {
	mutex_lock(&tMutex);
	if (blocks == 0) shm_init();
	if (size & 0xFFF) size = (size & 0xFFFFF000) + 0x1000;
	//go through all blocks, get the one with the closest size
	struct shm_block *block = blocks;
	while (block) {
		if (block->size >= size) break;
		block = block->next;
	}
	if (block == 0) {
		mutex_unlock(&tMutex);
		return 0;
	}
	//if the block's size is bigger, reduce it and create a new one after
	if (block->size > size) {
		struct shm_block *newb = malloc(sizeof(struct shm_block));
		newb->start = block->start + size;
		newb->size = block->size - size;
		newb->is_hole = 1;
		newb->prev = block; newb->next = block->next;
		block->size = size;
		if (block->next != 0) block->next->prev = newb;
		block->next = newb;
	}
	//mark block as used
	block->is_hole = 0;
	//return block's address
	mutex_unlock(&tMutex);
	return (void*)block->start;
}

static void unify (struct shm_block *b) {
	if (b->next == 0 || b->is_hole == 0 || b->next->is_hole == 0) return;
	struct shm_block *n = b->next;
	b->size += n->size;
	n->next->prev = b;
	b->next = n->next;
	free(n);
}

void shm_free(void* p) {
	mutex_lock(&tMutex);
	//find block
	struct shm_block *bl = blocks;
	while (bl) {
		if (bl->start == (size_t)p) break;
		bl = bl->next;
	}
	if (bl == 0) {
		mutex_unlock(&tMutex);
		return;
	}
	//mark it as a hole
	bl->is_hole = 1;
	//unify after if possible
	if (bl->next != 0 && bl->next->is_hole == 1) unify(bl);
	//unify before if possible
	if (bl->prev != 0 && bl->prev->is_hole == 1) unify(bl->prev);
	mutex_unlock(&tMutex);
}

void* shm_allocNew(size_t size) {
	if (size & 0xFFF) size = (size & 0xFFFFF000) + 0x1000;

	void* p = shm_alloc(size);
	if (shm_create((size_t)p, size) != 0) {
		shm_free(p);
		return 0;
	}
	return p;
}

void shm_freeDel(void *p) {
	shm_delete((size_t)p);
	shm_free(p);
}
