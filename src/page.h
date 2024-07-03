struct ppage {
	struct ppage *next;
	struct ppage *prev;
	void *physical_addr;
	//this was dumb
	//int space [4084];
};
