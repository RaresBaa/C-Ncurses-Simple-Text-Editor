#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Node{
    char *data;
    struct Node *next;
}Node;
void freeNode(Node *p);
void add(char *text);
int search(char *c);
void eraseList();
char* getNodeText(int line);
void setNodeText(int line, char* text);
void readFileToList(char *filename);
void writeListToFile(char *filename);
void deleteNodeAt(int n);
void insertNodeAfter(int n);
void addCharAt(int line, int x, char c);
void textEditor(char *fileName);
void init(char *fileName);

int listSize;
Node *first, *last;

int main(int argc, char* argv[]){
    if(argc != 2){//using program args for filename
        printf("\n No filename specified, exiting.\n");
        return -1;
    }
    init(argv[1]);
    textEditor(argv[1]);
	clear();
	endwin();
	return 0;
}

void init(char *fileName){
    first = last = NULL;
    listSize = 0;
    readFileToList(fileName);
    initscr(); //ncurses init
    noecho();
    cbreak();
    keypad(stdscr,TRUE);
    curs_set(2);
    start_color();//color
    init_pair(1, COLOR_MAGENTA, COLOR_CYAN);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_CYAN);
}

void textEditor(char *fileName){
    int maxX, maxY; //terminal siz
    int scrollOffset = 0;
    int reqExit = 1; //do we want to exit?
    int x = 0, y = 0; //current cursor position
    while(reqExit){
        getmaxyx(stdscr, maxY, maxX);//get Terminal size each loop in case it changes
        erase();
        //add text to buffer
        attron(COLOR_PAIR(1));//enable color
        printw("%d %d %d %d %d %d |", maxX, maxY, x, y, scrollOffset, listSize);
        attroff(COLOR_PAIR(1));//disable current color
        attron(COLOR_PAIR(4));
        printw("%s", getNodeText(y-1));
        attroff(COLOR_PAIR(4));
        for(int i=0;i < maxY;i++){
            if(i%2 == 0){
                attroff(COLOR_PAIR(3));
                attron(COLOR_PAIR(2));
            }else{
                attroff(COLOR_PAIR(2));
                attron(COLOR_PAIR(3));
            }
            char *line = strdup(getNodeText(i+scrollOffset));
            char *aux = malloc((maxX+1)*sizeof(char));
            strncpy(aux, line, maxX);
            printw("%s", aux);
            free(line);
            free(aux);
        }
        attroff(COLOR_PAIR(3));
        attroff(COLOR_PAIR(2));
        move(y,x);//show cursor position
        refresh();
        int userInput = getch();
        char userInputC;
        sprintf(&userInputC, "%c", userInput);
        switch(userInput){//process input
            case 27: //esc key
                reqExit = 0;
                break;
            case KEY_F(1): //F1 -save
                writeListToFile(fileName);
                break;
            case KEY_UP://moving y cursor pos up
                if(y>0){
                    y--;
                }else{
                    if(scrollOffset > 0){
                        scrollOffset--;
                    }
                }
                break;
            case KEY_DOWN://moving cursor pos down
                if(y<maxY-1){
                    y++;
                }else{
                    if(scrollOffset + maxY< listSize) {
                        scrollOffset++;
                    }
                }
                break;
            case KEY_LEFT: //moving cursor pos left
                if(x>0){
                    x--;
                }
                break;
            case KEY_RIGHT: //moving cursor pos right
                if(x<maxX){
                    x++;
                }
                break;
            case KEY_HOME: //moving cursor to start of line
                x=0;
                break;
            case KEY_END: //moving cursor to end of line
                x=maxX;
                break;
            case KEY_F(2): //moving cursor to start of file
                y=0;
                x=0;
                break;
            case KEY_F(3): //moving cursor to end of file
                y=maxY;
                x=0;
                break;
            case 10: //enter-add new line
                insertNodeAfter(y-1);
                y++;
                x=0;
                break;
            case 127: //backspace-delete line
                deleteNodeAt(y-2);
                break;
            default: //add character to text
                addCharAt(y, x, userInputC);
                x++;
                break;
        }
    }
    eraseList();
}

void freeNode(Node *p){
    free(p->data);
    free(p);
    listSize--;
}

void add(char *text){
    Node *p;
    if((p = (Node*)malloc(sizeof (Node)))){
        p->data = (char*) malloc(128*sizeof(char));
        strcpy(p->data, text);
        if(first == 0){ //empty list
            first = last = p;
        }else{
            last->next = p;
            last = p;
        }
        last -> next = NULL;
        listSize++;
    }
}

int search(char *c){
    Node *p = first;
    for (int i=0; i<listSize; i++){
        if(strcmp(p->data, c) == 0) {
            return i;
        }
        p = p->next;
    }
    return -1;
}

void eraseList(){
    Node *q, *q1;
    q = first;
    if(first == NULL){
        return;
    }
    while(q!=last){
        q1 = q;
        q = q->next;
    }
    if(q == first){
        first = last = NULL;
    }else{
        last = q1;
        q1 -> next = NULL;
    }
    freeNode(q);
}

char* getNodeText(int line){
    Node *p = first;
    if(line >= listSize){
        return "";
    }
    for(int i=0;i<line;i++){
        p=p->next;
    }
    return p->data;
}

void setNodeText(int line, char* text){
    Node *p = first;
    for(int i=0;i<line-1;i++){
        p=p->next;
    }
    strcpy(p->data, text);
}

void readFileToList(char *filename){
    FILE *fp = fopen(filename, "r");
    if(fp==NULL) {
        printf("\nCouldn't open file, exiting.\n");
        perror("File error");
    }
    char *line = NULL;
    size_t len = 0;
    while ((getline(&line, &len, fp)) != -1) {
        add(line);
    }
    fclose(fp);
}

void writeListToFile(char *filename){
    FILE *fp;
    fp = fopen(filename, "w");
    Node *p = first;
    for(int i=0;i<listSize;i++){
        fprintf(fp, "%s", p->data);
        p = p->next;
    }
    fclose(fp);
}

void insertNodeAfter(int n){
    Node *p = (Node*)malloc(sizeof (Node));
    p->data = (char*) malloc(128*sizeof(char));
    strcpy(p->data, "\n");
    Node *p1 = first;
    for(int i=1;i<n;i++){
        p1=p1->next;
    }
    Node *aux = p1->next;
    p1->next = p;
    p->next = aux;
    listSize++;
}

void addCharAt(int line, int x, char c){
    Node *p = first;
    for(int i=1;i<line;i++){
        p=p->next;
    }
    size_t len = strlen(p->data);
    char *aux = malloc((len + 1)*sizeof(char));
    strcpy(aux, p->data);
    free(p->data);
    for(size_t i=len+2 ; i>x ; i--) {
        aux[i] = aux[i - 1];
    }
    aux[x] = c;
    p->data = aux;
}

void deleteNodeAt(int n){
    Node *p1 = first;
    for(int i=0;i<n;i++){
        p1=p1->next;
    }
    Node *aux = p1->next;
    p1->next = aux->next;
    free(aux->data);
    listSize--;
}
