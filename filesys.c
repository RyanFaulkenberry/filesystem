#include "filesys.h"

FS* createFS() {
/* initializes the FS, initializes the root node and CWD pointer, returns
 * a pointer to the initialized FS
 */ 
	FS* fs = (FS*)malloc(sizeof(FS));

	//init
	fs->root = createNode(NULL, 1, "/");
	fs->CWD = fs->root;
	return fs;
}

node* createNode(node* parent, int nodeType, char* name) {
/* Initializes a node and sets it's parent pointer to the parent input. Sets
 * nodeType to 0 for file, 1 for directory. Sibling and parent pointers set to
 * NULL. Returns pointer to created node.
 */
	if (strlen(name) > PATH_LENGTH_MAX) {
		printf("name Error in createNode\n");
		return NULL;
	}	
	if (nodeType != 0 && nodeType != 1) {
		printf("nodeType Error in createNode\n");
		return NULL;
	}

	//init
	node* n = (node*)malloc(sizeof(node));
	memset(n->name, '\0', sizeof(n->name));
	strcpy(n->name, name);
	n->nodeType = nodeType;
	n->parent = parent;
	n->sibling = NULL;
	childListInsert(n, n->parent);
	return n;
}

void childListInsert(node* n, node* parent) {
/* Inserts node n into the proper sorted point in node parent's child list, 
 * adjusting sibling pointers as necessary*/	

	if (parent == NULL) //root case
		return;
	if (parent->firstChild == NULL) {
		parent->firstChild = n;
		return;
	}

	node* temp = parent->firstChild;
	node* last;

	while (nodeCompare(n, temp) == false) {
		last = temp;
		temp = temp->sibling;
		if (temp == NULL)
			break;
	}
	if (temp == parent->firstChild) {
		n->sibling = temp;
		parent->firstChild = n;
	}
	else {
		n->sibling = temp;
		last->sibling = n;
	}
	return;
}

bool nodeCompare(node* insertee, node* existing) {
/* True iff insertee should be inserted before existing in the child list */

	if (insertee->nodeType == 1) {
		if (existing->nodeType == 0)
			return true;
	}
	else {
		if (existing->nodeType == 1) 
			return false;
	}
	
	char newName[PATH_LENGTH_MAX];
	memset(newName, '\0', sizeof(newName));
	strcpy(newName, insertee->name);
	char listName[PATH_LENGTH_MAX];
	memset(listName, '\0', sizeof(listName));
	strcpy(listName, existing->name);
	
	int i;
	for (i=0;; i++) {
		if (!(newName[i] == '\0' && listName[i] == '\0')) {
			if (newName[i] != listName[i]) {
				if (newName[i] == '\0')
					return true;
				if (listName[i] == '\0')
					return false;
				if (newName[i] == '.')
					return true;
				if (listName[i] == '.')
					return false;
				if (newName[i] == '-')
					return true;
				if (listName[i] == '-')
					return false;
				if (newName[i] == '_')
					return true;
				if (listName[i] == '_')
					return false;
		// Remaining cases are in ascending ascii order by precedence
				if (newName[i] < listName[i])
					return true;
				if (newName[i] > listName[i])
					return false;
			}
		}
		else
			return true;
	}
	// equal case
	return true;
}

node* mkdir(node* n, char* path, int nodeType) {
/* Creates all missing directories along the relative path starting from node n,
 * ending with a node of type nodeType.
 * Ensures that no files are on the path and that the path does not yet exist.
 * Caller expected to reset fs->CWD to the return value of mkdir
 */
	if (path[0] == '/')	// ensure path is relative
		path++;

	node* temp = findNode(n, path);

	if (temp != NULL) {
		printf("Unable to create directory: directory already exists\n");
		return NULL;
	}
	return pathMaker(n, path, nodeType);
}

node* pathMaker(node* n, char* path, int makeType) {
/* Input path is expected NOT to begin with '/', that is, all paths are 
 * treated as relative paths from input node n. Pathmaker will create
 * all missing directories along the input path and create the last node
 * on the path as a file (makeType=0) or directory (makeType=1) */
	int i;
	int length = strlen(path);

	char tempPath[PATH_LENGTH_MAX+1], *t;
	memset(tempPath, '\0', sizeof(tempPath));
	t = tempPath;

	char* l;
	node* tempNode;

	int nodeType = 1;

	// build path node by node from n and fill in missing directories
	for (i=0; i < length; i++) {

		if (path[i] == '/' || i == length-1) {

			if (i == length-1) { // final case
				nodeType = makeType;
				tempPath[i] = path[i];
			}
			tempNode = findNode(n, tempPath);
			if (tempNode == NULL) { // if no node found on path
				l = getLocalFromPath(tempPath);
				n = createNextNode(n, l, nodeType);
			}
			else 
				n = tempNode;
		}
		tempPath[i] = path[i];

	}
	if (makeType == 0)
		return n->parent;
	return n;
}

node* createNextNode(node* n, char* path, int nodeType) {
/* Input node n is the parent of the node to be created. "path" is the local
 * name of the new node. Formats paramaters to pass to createNode and
 * returns the created node */

	char formattedPath[PATH_LENGTH_MAX+1], *f, *l;
	memset(formattedPath, '\0', sizeof(formattedPath));
	strcpy(formattedPath, n->name);

	if (n->parent != NULL)
		f = strcat(formattedPath, "/");
	f = strcat(formattedPath, path);
		
	return createNode(n, nodeType, f);
}

char* getLocalFromPath(char* path) {
/* From string path, finds the last occurence of '/' and returns the string
 * after that char. If no '/' returns the whole string. If no strig return NULL*/

	int i;
	char local[PATH_LENGTH_MAX+1];
	memset(local, '\0', sizeof(local));
	int length = strlen(path);

	if (path == NULL)
		return NULL;
	char* p = strchr(path, '/');
	if (p == NULL)
		return path;
	return ++p;
}

void pwd(node* n) {
/* Prints the absolute path to the cwd */
	printf("%s\n", n->name);
}

void ls(node* n, char* path) {
/* Prints the local name of every child of the directory
 * specified, or the children of CWD if no directory is specified
 */
	char* localName;
	node* temp;
	
	n = findNode(n, path);
	if (n == NULL) {
		printf("Path not found\n"); 
		return;
	}
	else if (n->nodeType == 0) {
		printf("Path leads to a file\n");
		return;
	}
	else if (n->firstChild == NULL) {
		printf("Directory is empty\n");
		return;
	}
	else
		temp = n->firstChild;

	// Iterate through each child and print
	do {
		if (temp->nodeType == 0)
			printf("F ");
		else
			printf("D ");
		localName = getLocalName(temp);
		printf("%s\n", localName);
		temp = temp->sibling;

	} while (temp != NULL);
}

node* cd(node* n, char* path) {
/* Returns the node pointed to by the specified relative path. If any 
 * error occurs, returs n */
	
	node* temp;

	if (fileOnPath(n, path)) {
		printf("Error, the path leads to a file\n");
		return n;
	}

	temp = findNode(n, path);
	if (temp == NULL) {
		printf("Error, path not found\n");
		return n;
	}
	return temp;
}

char* getLocalName(node* n) {
/* Returns the portion of n's name after the final '/' char, or if n is the root,
 * returns '/' */ 

	char localName[PATH_LENGTH_MAX], *l;
	memset(localName, '\0', sizeof(localName));
	
	char* a = n->name;
	if (n->parent == NULL) {
		localName[0] = '/';
		l = localName;
		return l;
	}

	while ((l = strchr(a, '/')) != NULL) // get string after final '/'
		a = l+1;
	l = a;

	int i;
	for (i=0; *(l+i) != '\0'; i++)
		localName[i] = *(l+i);
	
	l = &localName[0];
	return l;
}

node* findNode(node* n, char* name) {
/* Returns the node that relative path "name" leads to. input node n is 
 * considered the parent directory of the relative path. If no node exists
 * on the path, returns NULL */

	
	if (name == NULL)
		return n;
	int i;
	char tempName[PATH_LENGTH_MAX], *t;
	memset(tempName, '\0', sizeof(tempName));
	t = tempName;
	int length = strlen(name);

	if (t[0] == '/')
		t++;

	for (i=0; i < length; i++) {
	// Build path name from CWD and find according nodes	

		if (name[i] == '/' || i == length-1) {
			if (i == length-1)
				tempName[i] = name[i];
			n = findNextNode(n, t);
			if (n == NULL)
				return NULL;
			if (strcmp(n->name, name) == 0)
				return n;
		}
		tempName[i] = name[i];
	}
	return n;
}

node* findNextNode(node* n, char* name) {
/* Searches node n's children and returns a node with local name "name", else
 * returns NULL */

	char k[PATH_LENGTH_MAX], *l;
	memset(k ,'\0', sizeof(k));
	l = getLocalFromPath(name);

	node* temp;
	if (!n)
		return NULL;
	if (n->firstChild)
		temp = n->firstChild;
	else
		return NULL;

	while (temp != NULL) {
		if (strcmp(getLocalName(temp), l) == 0)		
			return temp;
		else
			temp = temp->sibling;
	}
	return NULL;
}

bool fileOnPath(node* n, char* path) {
/* Returns false iff the existing part of the input path conatains no files */

	int length = strlen(path);
	char temp[PATH_LENGTH_MAX], *t;
	memset(temp, '\0', sizeof(temp));
	t = temp;
	int i;
	node* tempNode;

	// build path node-by-node from beginning (as temp) until the path leads
	// to a non-existent node. If a file is detected, return false
	for (i=0; i < length; i++) { 
		
		if (path[i] == '/' || i == length-1) { 

			if (i == length-1) // end of path
				temp[i] = path[i];

			tempNode = findNode(n, t);

			if (tempNode == NULL) {
				return false;
			}
			else if (tempNode->nodeType == 0) {
				return true;
			}
		}
		temp[i] = path[i];
	}		
	return false;
}

char* allocatePath() {
	
	char path[PATH_LENGTH_MAX+1], *t;
	memset(path, '\0', sizeof(path));
	t = path;
	return t;
}

