#ifndef DEF_OBJ_H
#define DEF_OBJ_H

// generic method error codes
#define ME_UNHANDLED -32767
#define ME_INTERRUPTED -32766

struct object_cli {
	int id;
};

typedef struct object_cli Object;

#endif
