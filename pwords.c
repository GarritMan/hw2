#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 4
pthread_mutex_t mutexdict;


typedef struct dict {
  char *word;
  int count;
  struct dict *next;
} dict_t;

typedef struct thread_data{
	FILE *infile;
	dict_t *dict;
	char *wordbuf;
}thread_data;

char *
make_word( char *word ) {
  return strcpy( malloc( strlen( word )+1 ), word );
}

dict_t *
make_dict(char *word) {
  dict_t *nd = (dict_t *) malloc( sizeof(dict_t) );
  nd->word = make_word( word );
  nd->count = 1;
  nd->next = NULL;
  return nd;
}

dict_t *
insert_word( dict_t *d, char *word ) {
  
  //   Insert word into dict or increment count if already there
  //   return pointer to the updated dict
  
  dict_t *nd;
  dict_t *pd = NULL;		// prior to insertion point 
  dict_t *di = d;		// following insertion point
  // Search down list to find if present or point of insertion
  while(di && ( strcmp(word, di->word ) >= 0) ) { 
    if( strcmp( word, di->word ) == 0 ) { 
      di->count++;		// increment count 
      return d;			// return head 
    }
    pd = di;			// advance ptr pair
    di = di->next;
  }
  nd = make_dict(word);		// not found, make entry 
  nd->next = di;		// entry bigger than word or tail 
  if (pd) {
    pd->next = nd;
    return d;			// insert beyond head 
  }
  return nd;
}

void print_dict(dict_t *d) {
  while (d) {
    printf("[%d] %s\n", d->count, d->word);
    d = d->next;
  }
}

int
get_word( char *buf, int n, FILE *infile) {
	//pthread_mutex_lock(&mutexword);
  int inword = 0;
  int c;  
  while( (c = fgetc(infile)) != EOF ) {
    if (inword && !isalpha(c)) {
      buf[inword] = '\0';	// terminate the word string
      pthread_mutex_unlock(&mutexword);
      return 1;
    } 
    if (isalpha(c)) {
      buf[inword++] = c;
    }
  }
 // pthread_mutex_unlock(&mutexword);
  return 0;			// no more words
}

#define MAXWORD 1024
dict_t *
words( thread_data *TData ) {
  
  int c=1;
  do {
  	pthread_mutex_lock(&mutexdict);
  	
  	c=get_word( TData->wordbuf, MAXWORD, TData->infile );
  	
    TData->dict = insert_word(TData->dict, TData->wordbuf); // add to dict
    pthread_mutex_unlock(&mutexdict);
  }while(c);
  
  return TData->dict;
}




void* worker_thread(void* arg){
	thread_data *TData;
	TData=(thread_data *) arg;
	
	TData->dict=words(TData);
	pthread_exit(NULL);
}


int
main( int argc, char *argv[] ) {
	pthread_t thread_ID[NUM_THREADS];
	void *status;
	pthread_mutex_init(&mutexdict, NULL);
	
	
	FILE *infile = stdin;
	if (argc >= 2) {
		infile = fopen (argv[1],"r");
	}
	if( !infile ) {
		printf("Unable to open %s\n",argv[1]);
		exit( EXIT_FAILURE );
	}
	
	clock_t begin, end;
	double time_spent;
	
	dict_t *wd = NULL;
	char wordbuf[MAXWORD];
	thread_data *TD;
	
	TD->infile=infile;
	
	TD->dict=wd;
	TD->wordbuf=wordbuf;
	
	int i;
	for(i=0;i<NUM_THREADS;i++){
		pthread_create(&thread_ID[i],NULL,worker_thread,(void*) TD);
	}
	
	for(i=0;i<NUM_THREADS;i++){
		pthread_join(thread_ID[i],&status);
	}
	
	
	print_dict( TD->dict );
	int sta=pthread_mutex_destroy(&mutexdict);
	
	
	
	printf("done %d\n",sta);
	pthread_mutex_destroy(&mutexword);
	fclose( infile );
}

