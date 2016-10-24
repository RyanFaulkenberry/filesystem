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
	n->nodeType = nodeType;
	memset(n->name, '\0', sizeof(n->name));
	strcpy(n->name, name);
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

	while (nameCompare(n, temp) == false) {
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

bool nameCompare(node* insertee, node* existing) {
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

node* mkdir(FS* fs, char* path) {
/* Creates all missing directories along the given absolute or relative path.
 * Ensures that no files are on the path and that the path does not yet exist.
 * Caller expected to reset fs->CWD to the return value of mkdir
 */
	node* n;
	node* temp;

	if (!fileOnPath(fs, path)) {
		printf("Error, a file exists on the path\n");
		return NULL;
	}
	if (path[0] == '/')
		temp = fs->root;
	else
		temp = fs->CWD;

	if (findNode(temp, path) != NULL) {
		printf("Unable to create directory: directory already exists\n");
		return NULL;
	}
	if (path[0] == '/') {
		char* pointer = &path[1];
		return pathMaker(fs->root, pointer, 1);
	}
	else
		return pathMaker(fs->CWD, path, 1);
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

	char* l = getLocalFromPath(path);
	
	node* tempNode;

	int nodeType = 1;
	// build path node by node from n and fill in missing directories
	for (i=0; i < length; i++) {

		tempPath[i] = path[i];
		if (path[i] == '/' || i == length-1) {
			if (i == length-1) { // final case
				nodeType = makeType;
			}
			tempNode = findNode(n, tempPath);
			if (tempNode == NULL) // if no node found on path
				n = createNextNode(n, l, nodeType);
			else 
				n = tempNode;
		}
	}
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

	p++;
	for (i=0; *(p+i) != '\0'; i++)
		local[i] = *(p+i);

	char* l = local;
	return l;
}

void pwd(FS* fs) {
/* Prints the absolute path to the cwd */
	printf("%s\n", fs->CWD->name);
}

void ls(FS* fs, char* name) {
/* Prints the local name of every child of the absolute or relative directory
 * specified, or the children of CWD if no directory is specified
 */
	
	char* localName;
	char* relName;
	node* n;
	node* temp;
	node* AbsRel;
	
	if (name == NULL)
		n = fs->CWD;
	else {
		if (name[0] == '/') {
			AbsRel = fs->root;
			if (strlen(name) > 1)
				relName = &name[1];
			else
				relName = NULL;
		}
		else {
			AbsRel = fs->CWD;
			relName = &name[0];
		}
		n = findNode(AbsRel, relName);
	}

	if (n == NULL) {
		printf("Error: path not found\n");
		return;
	}
	else if (n->firstChild == NULL) {
		printf("Directory is empty\n");
		return;
	}
	else
		temp = n->firstChild;

	// Iterate through each child and print
	while (temp->sibling != NULL) {
		if (temp->nodeType == 0)
			printf("F ");
		else
			printf("D ");
		localName = getLocalName(temp);
		printf("%s\n", localName);
		temp = temp->sibling;
	}
	if (temp->nodeType == 0)
		printf("F ");
	else
		printf("D ");
	printf("%s\n", getLocalName(temp));
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

bool fileOnPath(FS* fs, char* path) {
/* Returns true iff the existing part of the input path conatains no files */

	int length = strlen(path);
	char temp[PATH_LENGTH_MAX], *t;
	memset(temp, '\0', sizeof(temp));
	t = temp;
	int i;
	node* n;
	node* tempNode;

	if (path[0] == '/')
		tempNode = fs->root;
	else
		tempNode = fs->CWD;

	// build path node-by-node from beginning (as temp) until the path leads
	// to a non-existent node. If a file is detected, return false
	for (i=0; i < length; i++) { 
		
		if (path[i] == '/' || i == length-1) { 

			if (i == length-1) // end of path
				temp[i] = path[i];

			n = findNode(tempNode, t);

			if (n == NULL) {
				return true;
			}
			else if (n->nodeType == 0) {
				return false;
			}
		}
		temp[i] = path[i];
	}		
	return true;
}
