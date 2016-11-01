#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define PATH_LENGTH_MAX 100
#define COMMAND_LENGTH_MAX 10

/* A node is a file or directory in the filesys (indicated respectively by a 
 * 0 or 1). Every node's name is the absolute path leading to it.
 *  Every node can have one or zero parents, and any number of siblings 
 *  or children.
 *
 *  POINTERS:
 *  Every parent node P has one firstChild F, which points to the first child in
 *  A chain of siblings. If there exists another child, the first child's
 *  sibling pointer will point to the next sibling, and so on, for every
 *  child C, such that for one parent with many children, the pointer
 *  chain may be visualized as: P->F->C->C-> ... ->C->null
 *  Note also that any F or C in this chain may have children of their own
 *
 *  PATHS AND NAMES:
 *  Paths follow normal filesystem conventions. An absolute path to a 
 *  file/directory is the path from the root directory (a single slash), 
 *  to the file/directory in question, where all directory names are separated
 *  by slashes. A relative path is a path from a certain file/directory (not
 *  beginning with a slash) to another file/directory. Though paths are
 *  separated by slashes, they do not end in slashes.
 *
 *  A file/directory's name is the absolute path to it. A file/directory's
 *  local name is the string after the last '/' in the path. Note that
 *  this filesystem does not currently support slashes at the end of paths 
 */

struct _NODE_ {
    int nodeType; //0 indicates file, 1 indicates directory
    char name[PATH_LENGTH_MAX];
    struct _NODE_* parent;
    struct _NODE_* sibling;
    struct _NODE_* firstChild;
};
typedef struct _NODE_ node;

/* A FS is a filesystem containing a pointer to the root node of the filesys
 * tree, and a pointer to the current working directory node.
 * Creating a FS creates a root node and initialized CWD to the root.
 */

struct _FILESYS_ {
    node* root;
    node* CWD;
};
typedef struct _FILESYS_ FS;

///////////////////////////////////////////////////////////////////////////////
FS* createFS();
node* createNode(node*, int, char*);
node* createNextNode(node*, char*, int);
void pwd(FS*);
void ls(FS*, char*);
void childListInsert(node*, node*);
node* cd(FS*, char*);
node* mkdir(FS*, char*, int);
node* pathMaker(node*, char*, int);
node* pathMakerCreate(node*, char*, int);
char* getLocalName(node*);
char* getLocalFromPath(char*);
char* allocatePath();
node* findNode(node*, char*);
node* findNodeAbsolute(FS*, char*);
node* findNodeRelative(FS*, char*);
node* findNextNode(node*, char*);
bool fileOnPath(FS*, char*);
bool nodeCompare(node*, node*);
