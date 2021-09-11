#include <stdio.h>
#include <stdlib.h>
#include "inputoutput.h"
#include <string.h>
#include <ncurses.h>

void render_text_arr(char **text_arr, int **color_arr, int width, int height, int x, int y, char *name, int save, int shifting);
void main_loop(char **text_arr, int **color_arr, int width, int height, char *name);

int file_exists(char *name){
    FILE *fp;
    if(fp = fopen(name,"r")){
        fclose(fp);
        return 1;
    }
    return 0;
}

char** get_text_arr(int width, int height){
    char **text_arr = (char**)malloc(sizeof(char*) * height);
    for(int i = 0; i < height; i++){
        text_arr[i] = malloc(sizeof(char)*(width+1));
        for(int j = 0; j < width; j++){
            text_arr[i][j] = ' ';
        }
        text_arr[i][width] = '\0';
    }
    return text_arr;
}

int **get_color_arr(int width, int height){
    int **color_arr = (int**)malloc(sizeof(int*)*height);
    for(int i = 0; i < height; i++){
        color_arr[i] = malloc(sizeof(int)*width);
        for(int j = 0; j < width; j++){
            color_arr[i][j] = 0;
        }
    }
    return color_arr;
}

void save_file(char *name, char **text_arr, int **color_arr, int width, int height){
    FILE *svf = fopen(name, "wb");

    char *file_type = "cursed-ascii-file";
    fputs(file_type, svf);

    fwrite(&width, sizeof(int), 1, svf);
    fwrite(&height, sizeof(int), 1, svf);

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            char cur = text_arr[i][j];
            fwrite(&cur,sizeof(char),1,svf);
        }
    }

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            int cur = color_arr[i][j];
            fwrite(&cur,sizeof(int),1,svf);
        }
    }

    fclose(svf);
    return;
}

int is_cursed_ascii(char *name){
    FILE *svf = fopen(name, "rb");

    char expected_file_type[18] = "cursed-ascii-file";
    char actual_file_type[18];
    fgets(actual_file_type, 18, svf);

    if(strcmp(expected_file_type, actual_file_type) != 0){
        fclose(svf);
        return 0;
    }

    fclose(svf);
    return 1;
}

typedef struct TextAndColors {
    int **color_arr;
    char **text_arr;
} TextAndColors;

TextAndColors load_file(char *name, int *wp, int *hp){
    FILE *svf = fopen(name, "rb");

    char temp[18];
    fgets(temp,18,svf);

    int width, height;
    fread(&width, sizeof(int), 1, svf);
    fread(&height, sizeof(int), 1, svf);
    *wp = width;
    *hp = height;
    char **text_arr = get_text_arr(width, height);
    for(int i = 0; i < width*height; i++){
        char next;
        fread(&next, sizeof(char), 1, svf);
        int y = i / width;
        int x = i % width;
        text_arr[y][x] = next;
    }
    int **color_arr = (int**)malloc(sizeof(int*)*height);
    for(int i = 0; i < width*height; i++){
        int next;
        fread(&next, sizeof(int), 1, svf);
        int y = i / width;
        int x = i % width;
        if(x == 0){
            color_arr[y] = (int*)malloc(sizeof(int)*width);
        }
        color_arr[y][x] = next;
    }
    fclose(svf);
    TextAndColors result;
    result.text_arr = text_arr;
    result.color_arr = color_arr;
    return result;

}

void new_file(){
    int width = input_int("Width of new art: ");
    int height = input_int("Height of new art: ");
    char *name = input("Name of new file: ",100);

    if(file_exists(name)){
        if(!yes_no_input("A file by this name already exists. Are you sure you'd like to continue (y/n): ",0)){
            print_str("File creation aborted");
            return;
        }
    }

    char **text_arr = get_text_arr(width,height);
    int **color_arr = get_color_arr(width, height);
    save_file(name, text_arr, color_arr, width, height);
    main_loop(text_arr, color_arr, width, height, name);
    return;
}

void existing_file(){
    char *name = input("File name: ",100);
    if(!file_exists(name)){
        print_str("File does not exist");
        return;
    }
    if(!is_cursed_ascii(name)){
        print_str("File is not an Cursed Ascii file");
        return;
    }
    int width, height;
    TextAndColors arrs = load_file(name, &width, &height);
    char **text_arr = arrs.text_arr;
    int **color_arr = arrs.color_arr;
    main_loop(text_arr, color_arr, width, height, name);
    return;
}

int min(int a, int b){
    if(a < b)
        return a;
    return b;
}

int max(int a, int b){
    if(a > b)
        return a;
    return b;
}

void move_coor(int width, int height, int *x, int *y, int dir){
    int tx = *x;
    int ty = *y;

    tx = tx + dir;

    if(tx >= width){
        tx = 0;
        ty++;
        if(ty >= height){
            ty = height-1;
            tx = width-1;
        }
    }

    if(tx < 0){
        tx = width-1;
        ty--;
        if(ty < 0){
            ty = 0;
            tx = 0;
        }
    }

    *x = tx;
    *y = ty;
}

char *get_border(int width){
    char *border= malloc(sizeof(char) * (width+1));
    for(int i = 0; i < width; i++){
        border[i] = '#';
    }
    border[width] = '\0';
    return border;
}


void set_default_colors(){
    init_pair(1, COLOR_WHITE,COLOR_BLACK);
    init_pair(2, COLOR_BLACK,COLOR_BLACK);
    init_pair(3, COLOR_RED,COLOR_BLACK);
    init_pair(4, COLOR_GREEN,COLOR_BLACK);
    init_pair(5, COLOR_BLUE,COLOR_BLACK);
    init_pair(6, COLOR_CYAN,COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA,COLOR_BLACK);
    init_pair(8, COLOR_YELLOW,COLOR_BLACK);
    return;
}

void main_loop(char **text_arr, int **color_arr, int width, int height, char *name){

    WINDOW *win = initscr();
    start_color();
    set_default_colors();
    noecho();
    keypad(win, TRUE);
    curs_set(0);

    int x = 0;
    int y = 0;
    int save = 0;
    int current_color = -1;
    int shifting = 0;

    int should_continue = 1;
    while(should_continue){
        render_text_arr(text_arr,color_arr,width,height,x,y,name,save,shifting);
        int ch = getch();
        switch(shifting){
            case (1):
                switch(ch){
                    case ('s'):
                        save_file(name, text_arr, color_arr, width, height);
                        save = 0;
                        break;
                    case ('q'):
                        should_continue = 0;
                        break;
                    case ('z'):
                        shifting = 0;
                        break;
                    case ('?'):
                        text_arr[y][x] = ch;
                        color_arr[y][x] = current_color;
                        save = 1;
                        move_coor(width,height,&x,&y,1);
                        shifting = 0;
                        break;
                    default:
                        if(ch >= 48 && ch <= 56){
                            current_color = (int)ch-48;
                        }
                        break;
                }
                break;
            default:
                switch (ch)
                {
                    case (KEY_UP):
                        y--;
                        y = max(0, y);
                        break;
                    case (KEY_DOWN):
                        y++;
                        y = min(height-1,y);
                        break;
                    case (KEY_ENTER):
                        y++;
                        y = min(height-1,y);
                        break;
                    case (KEY_LEFT):
                        x--;
                        x = max(0, x);
                        break;
                    case (KEY_RIGHT):
                        x++;
                        x = min(width-1,x);
                        break;
                    case (KEY_DC):
                        text_arr[y][x] = ' ';
                        save = 1;
                        move_coor(width,height,&x,&y,-1);
                        break;
                    case (KEY_BACKSPACE):
                        text_arr[y][x] = ' ';
                        save = 1;
                        move_coor(width,height,&x,&y,-1);
                        break;
                    case ('?'):
                        shifting = 1;
                        break;
                    default:
                        if(!(32 <= ch && ch <= 126))
                            break;
                        text_arr[y][x] = ch;
                        color_arr[y][x] = current_color;
                        save = 1;
                        move_coor(width,height,&x,&y,1);
                        break;
                }
                break;
        }
    }

    endwin();
}

void render_text_arr(char **text_arr, int **color_arr, int width, int height,int x, int y, char *name, int save, int shifting){
    clear();
    char *border = get_border(width);

    //colors
    if(shifting){
        for(int i = 1; i < 9; i++){
            char c = (char)(i+48);
            addch(c | COLOR_PAIR(i));
            addch(' ');
        }
    }
    addstr("\n");

    //title
    addstr(name);
    if(shifting)
        addstr(" (alt mode) ");
    if(save)
        addstr(" * ");
    
    addstr("\n\n");
    addstr(border);
    addstr("\n");
    for(int i = 0; i < height; i++){
        addstr("|");
        for(int j = 0; j < width; j++){
            char added_char = text_arr[i][j];
            if(x == j && y == i){
                if(color_arr[i][j] > -1){
                    addch(added_char | A_BOLD | A_UNDERLINE | COLOR_PAIR(color_arr[i][j]));
                }else{
                    addch(added_char | A_BOLD | A_UNDERLINE);
                }
            }else{
                if(color_arr[i][j] > 0){
                    addch(added_char | COLOR_PAIR(color_arr[i][j]));
                }else{
                    addch(added_char);
                }
            }
        }
        addch('|');
        addch('\n');
    }
    addstr(border);
    addstr("\n\nPress Home to quit...\nUse Shift+? to enter Alt mode\nIn Alt Mode...\nPress q to quit\nPress z to save\nPress z to exit alt mode\nPress 0-8 to change colors");
    refresh();
}

void convert_to_text(char *existing_name, char *new_name){
    int width, height;
    TextAndColors arrs = load_file(existing_name, &width, &height);
    char **text_arr = arrs.text_arr;
    int **color_arr = arrs.color_arr;
    FILE *cf = fopen(new_name, "w");
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            char str[2] = {text_arr[i][j],'\0'};
            fputs(str,cf);
        }
        fputs("\n",cf);
    }
    fclose(cf);
    return;
}

void convert_file(){
    char *exist_name = input("Existing Cursed Ascii Art file: ",100);

    if(!file_exists(exist_name)){
        print_str("File does not exist");
        return;
    }

    if(!is_cursed_ascii(exist_name)){
        print_str("File is not an Cursed Ascii file");
        return;
    }

    char *new_name = input("Convert to file: ",100);

    if(file_exists(new_name)){
        if(!yes_no_input("A file by this name already exists. Are you sure you'd like to continue (y/n): ",0)){
            print_str("File creation aborted");
            return;
        }
    }

    convert_to_text(exist_name,new_name);
    return;
}

void stop_signal(int sig){
    return;
}

int main(int argc, char *argv[]){

    char instruct_flag[3] = "-n";
    if(argc > 1){
        int flag_len = strlen(argv[1]);
        if(flag_len == 2){
            instruct_flag[1] = argv[1][1];
        }
    }
    
    switch (instruct_flag[1])
    {
        case ('n'):
            new_file();
            break;
        case ('l'):
            existing_file();
            break;
        case ('c'):
            convert_file();
            break;
        default:
            new_file();
            break;
    }


    return 0;
}