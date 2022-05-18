#include "headers.h"

struct Tnode {
    struct Tnode* left;
    struct Tnode* right;
    int occupied;
    //int key;
    struct Tnode* parent;
    int start;
    int end;
};

void insert(struct Tnode ** binary_tree, int start, int end, struct Tnode* parent, int counter) {
    struct Tnode* tmp = NULL;
    if(!(*binary_tree)) {
        tmp = (struct Tnode *)malloc(sizeof(struct Tnode));
        tmp->left = tmp->right = NULL;
        tmp->start=start;
        tmp->end=end;
        tmp->parent=parent;
        tmp->occupied=-1;
        *binary_tree = tmp;
        //printf("here counter now is %d\n",counter);
        //return;
    }
        if(counter <= 0)
        {
            //printf("end of counter\n");
            return;
        }
        counter--;
        insert(&(*binary_tree)->left, start, start+((end-start)/2), parent,counter);
        insert(&(*binary_tree)->right, (start+(end-start)/2)+1, end, parent,counter);
}

struct Tnode* initMemory ()
{
    int size=1024;
    struct Tnode* head;
    //struct Tnode** tmphead;
    for (int i=0;i<8;i++)
    {
        if(i==0)
        {
            head= (struct Tnode *)malloc(sizeof(struct Tnode));
            head->start=0;
            head->end=(size-1);
            head->parent=NULL;
            head->occupied=-1;
            head->left=head->right=NULL;
            //tmphead=&head;
            size=size/2;
        }
        else
        {
            insert(&head,head->start,head->end,head,7);
        }
    }
    return head;
}

struct Tnode *Treesearch(struct Tnode ** binary_tree, int start, int end) {
    struct Tnode* leftSearch=NULL;
    struct Tnode* rightSearch=NULL;
    if((*binary_tree)==NULL) {
        //printf("Was NULL\n");
        return NULL;
    }
    if(start == (*binary_tree)->start && end == (*binary_tree)->end) {
        // printf("inside condition %d\n",((*binary_tree)->start));
        // printf("inside condition %d\n",((*binary_tree)->end));
        return *binary_tree;
    }
    // printf("%d\n",((*binary_tree)->start));
    // printf("%d\n",((*binary_tree)->end));

    leftSearch = Treesearch(&((*binary_tree)->left), start, end);
    rightSearch = Treesearch(&((*binary_tree)->right), start, end);

    if (rightSearch)
    {
        return rightSearch;
    }
    else
    {
        return leftSearch; 
    }
}

void display_preorder(struct Tnode * binary_tree) {
    if (binary_tree) {
        printf("%d %d\n",binary_tree->start,binary_tree->end);
        display_preorder(binary_tree->left);
        display_preorder(binary_tree->right);
    }
}

void deallocation(struct Tnode** Head,int start, int end)
{
    struct Tnode *deallocated=Treesearch(Head, start, end);
    deallocated->occupied=-1;
    struct Tnode* parent=deallocated->parent;
    while(parent != NULL)
    {
        if(parent->left->occupied==-1 && parent->right->occupied==-1)
        {
            parent->occupied=-1;
            parent=parent->parent;
        }
        else
        {
            break;
        }
    }

    
}

int main()
{
    struct Tnode* root=NULL;
    struct Tnode* search=NULL;
    root=initMemory();
    //insert(&root,0,1023,NULL,3);
    //display_preorder(root);
    search=Treesearch(&root, 768, 1023);
    //printf("%d\n",root->right==NULL);
    if(search!=NULL)
    {
        printf("%d %d\n",search->start,search->end);
    }
    else
    {
        printf("Not found\n");
    }
}

