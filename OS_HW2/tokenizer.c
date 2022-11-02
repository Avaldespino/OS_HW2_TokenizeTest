#include <stdlib.h>
#include <stdio.h>
#include "tokenizer.h"
#include "implementation.c"
int space_char(char c){
  if(c == '\t' || c == ' ' || c == '\n'){
    // printf("Space_char = %c\n",c);
    return 1;
  }
  return 0;

}
int non_space_char(char c){
  // printf("non_space = %c\n",c);
  if(!space_char(c)){
    // printf("Returning non space true\n");
    return 1;
  }
  
  return 0;
}

char *word_start(char *str){
  if(!str) return NULL;
  while(*str){
    // printf("word_start %c\n",*str);
    if(non_space_char(*str)){
      //printf("returning a string = %c,%i\n",*str,non_space_char(*str));
      return str;}
    str++;
  }
 
  char *null = '\0'; 
  return null;
}

char *word_terminator(char *word){
  if(!word) return NULL;
  while(*word){
    if(space_char(*word)) return word;
    word++;
  }
  return 0;
}


int count_words(char *str){
  int counter = 0;
  int in_Word = 0;
  if(str){
    while(*str){
      if(space_char(*str)){
	in_Word = 0;
      }
      else if(!in_Word){
	in_Word = 1;
	++counter;
      }
      ++str;
    }
  }
    return counter;
}

char *copy_str(char *inStr, short len){

  char* new_string = (char*)__malloc_impl((len+1) *sizeof(char));
  

  int i = 0;
  while(i<len){
  new_string[i] = inStr[i];
  ++i;
  }
  
  
  
  
  new_string[i]='\0';

  
   
    
  
  return new_string;

}
 char **tokenize(char *str){
   if(!str){
     printf("Error string is null\\n");
     return NULL;
   }
   int words = count_words(str);

   char **tokens = (char**)__malloc_impl((words+1)*sizeof(char*));
    char *start_word=word_start(str);
   char *end_word;
     int i = 0;
   while(i<words){
  
    
     end_word = word_terminator(start_word);
  
     tokens[i] = copy_str(start_word,(end_word - start_word));
     start_word = word_start(end_word);
     
     ++i;
   }
   tokens[words] = '\0';
   return tokens;
 }


 void print_tokens(char **tokens){
   if(!tokens){ printf("Error: Empty poiner");return;}
   printf("-----Tokens-----\n");
   int i = 0;
   while(tokens[i]){
     printf("[%i] = %s\n",i,tokens[i]);
     ++i;
   }
   printf("End of list\n");
 }

 void free_tokens(char **tokens){
   int i = 0;
   while(tokens[i]){
     __free_impl(tokens[i]);
     ++i;
   }
   __free_impl(tokens);
 }
