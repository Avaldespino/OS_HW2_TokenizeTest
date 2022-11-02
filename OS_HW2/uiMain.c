#include <stdlib.h>
#include <stdio.h>
#include "tokenizer.h"
#include "history.h"


int main(){
  char input[256];
  char *pass = input;
  
    List *history = init_history();
    goto list;
  while(1){

  list:
    printf("\e[1;1H\e[2J");
    while(1){
   
    puts("-----History-----");
    puts(">Enter input, use !q to break, use !t to switch to Tokenize");
    puts("Enter any string for it to be added to a list. !p to print. ![number] for a specific slot");
    
    
    fgets(input,256,stdin);
    fflush(stdout);
    if(input[0] == '!' && input[1] == 'q'){
      goto done;
    }
    else if(input[0] == '!' && input[1] == 't'){
      goto token;
      
    }
    else if(input[0] == '!' && input[1] == 'p'){
      printf("\e[1;1H\e[2J");
      puts("-----History-----\n");
      print_history(history);
    }
    else if(input[0] == '!' && input[1] == '['){
      printf("\e[1;1H\e[2J");
      printf("Given item [%c]  ",input[2]);
      char *start = pass+2;
      char *end;
      while(*start){
	if(*start == ']'){
	  --start;
	  end = start;
	  break;
	}
	start++;
      }
      
      if(end == start){
	int index = atoi(start);
	printf("[%d] %s\n",index,get_history(history,index));
      
      }else{
      char *copy = copy_str(start,(end-start));
      int index = atoi(copy);
      
      printf("[%d] %s\n",index,get_history(history,index));
      free(copy);
      }
      
    }
    else{
    pass = input;
    add_history(history,pass);
    printf("Added %s\n",pass);
    }
    
    
    }

     
  }

 token:
  printf("\e[1;1H\e[2J");
  while(1){
    
    puts("-----Tokenize-----");
    puts(">Enter input, use !q to break, use !l to switch to History");
    puts("Enter any string for it to be tokenized.");

    if(input[0] == '!' && input[1] == 'q'){
      goto done;
    }
    else if(input[0] == '!' && input[1] == 'l'){
      goto list;
      
    }
    fgets(input,256,stdin);
    fflush(stdout);
    
    char **tokens = tokenize(input);
    print_tokens(tokens);
    free_tokens(tokens);

  }

 done:
  return 0;




}