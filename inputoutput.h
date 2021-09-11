#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * input(char prompt[],int string_len){
    char* result = malloc(sizeof(char)*string_len);
    printf("%s",prompt);
    scanf("%s", result);
    return result;
}

int input_int(char prompt[]){
    int result_int;
    printf("%s",prompt);
    int input_result = scanf("%d",&result_int);
    if(input_result == EOF)
        return 0;
    return result_int;
}

void print_str(char *str){
    printf("%s\n",str);
    return;
}

void print_int(int i){
    printf("%d\n",i);
    return;
}

int yes_no_input(char *prompt, int def){
    char *result = input(prompt,2);
    if(strlen(result) <= 0)
        return def;
    if(result[0] == 'y' || result[0] == 'Y')
        return 1;
    return 0;
}