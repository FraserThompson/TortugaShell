/* bst.h
* Header file for bst.c.
*/
#ifndef BST
#define BST

typedef struct bst
{
	wchar_t *title;
	wchar_t *description;
	struct bst *left, *right;
}node;

extern void bst_insert(node *, node *);
extern void inorder(node *);
extern void preorder(node *);
extern void postorder(node *);
extern node *bst_search(node *, wchar_t *, node **);
extern node *init_node();
extern void bst_free(node *);

#endif