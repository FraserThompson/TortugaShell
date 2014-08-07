/* bst.h
* Header file for bst.c.
*/
#ifndef BST
#define BST

typedef struct bst
{
	wchar_t *title;
	wchar_t *description;
	int type;
	struct bst *left, *right;
}node;

extern void bst_insert(node *, node *);
extern void inorder(node *);
extern node *bst_search(node *, wchar_t *, node **);
extern node *bst_partial_search(node *, wchar_t *, node **);
extern node *init_node();
extern void bst_free(node *);

#endif