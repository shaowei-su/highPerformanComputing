
typedef struct node {
	int children;
	int count;
	struct node *parent;
} NODE;

typedef struct barrier {
	int sense;
	NODE *node[n];
} BARRIER;

int mysense = 1;

void Build(BARRIER *B, NODE *parent, int height) {
	static int nodes = 0;
	NODE *nd;
	nd = (*nd)malloc(sizeof(nd)); //NODE *nd = newcell(NODE);
	nd->parent = parent;
	if (height == 0) {
		nd->count = nd->children = 0;
		B->node[nodes++] = nd;
	}
	else {
		nd->count = nd->children = r;
		B->node[nodes++] = nd;
		for (j=0; j<r; j++){
			Build(B, nd, height-1);
		}
	}
}

void InitializeBarrier(BARRIER *B) {
	int height = 0;
	while (n > 1) {
		height++;
		n = n / r;
	}
	Build(B, NULL, height);
	B->sense = 0;
}

void await(BARRIER *B) {
	NODE *nd = B->node[i];
	wait(nd);
}

void wait(NODE *nd) {
	while (nd->count > 0) ;
	nd->count = nd->children;
	if (nd->parent == NULL)
		B->sense = 1 - B->sense;
	else {
		faa(nd->parent->count, -1);
		while (B->sense != mysense) ;
	}
	mysense = 1 - mysense;
}