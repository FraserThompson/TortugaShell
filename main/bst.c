#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "bst.h"

node *init_node()
{
	node *temp;
	temp = (node *)emalloc(sizeof(node));
	temp->left = NULL;
	temp->right = NULL;
	return temp;
}

void bst_insert(node *root, node *new_node)
{
	int comparison = wcscmp(new_node->title, root->title);

	// Skip if it's already there
	if (comparison == 0){
		return;
	}

	if (comparison < 0)
	{
		if (root->left == NULL) {
			root->left = new_node;
		}
		else {
			bst_insert(root->left, new_node);
		}
	} else
	{
		if (root->right == NULL) {
			root->right = new_node;
		}
		else {
			bst_insert(root->right, new_node);
		}
	}
}

node *bst_search(node *root, wchar_t *key, node **parent)
{
	node *temp;
	temp = root;
	int comparison;

	while (temp != NULL)
	{
		comparison = wcscmp(temp->title, key);
		if (comparison == 0)
		{
			return temp;
		}
		*parent = temp;

		if (comparison > 0)
			temp = temp->left;
		else
			temp = temp->right;
	}

	return NULL;
}

void inorder(node *temp)
{
	if (temp != NULL)
	{
		inorder(temp->left);
		wprintf(L"%s\n", temp->title);
		inorder(temp->right);
	}
	else {
		wprintf(L"Empty tree!");
	}
}