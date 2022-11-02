
/*  

    Copyright 2018-21 by

    University of Alaska Anchorage, College of Engineering.

    Copyright 2022 by

    University of Texas at El Paso, Department of Computer Science.

    All rights reserved.

    Contributors:  ...
                   ...
		   ...                 and
		   Christoph Lauter

    See file memory.c on how to compile this code.

    Implement the functions __malloc_impl, __calloc_impl,
    __realloc_impl and __free_impl below. The functions must behave
    like malloc, calloc, realloc and free but your implementation must
    of course not be based on malloc, calloc, realloc and free.

    Use the mmap and munmap system calls to create private anonymous
    memory mappings and hence to get basic access to memory, as the
    kernel provides it. Implement your memory management functions
    based on that "raw" access to user space memory.

    As the mmap and munmap system calls are slow, you have to find a
    way to reduce the number of system calls, by "grouping" them into
    larger blocks of memory accesses. As a matter of course, this needs
    to be done sensibly, i.e. without wasting too much memory.

    You must not use any functions provided by the system besides mmap
    and munmap. If you need memset and memcpy, use the naive
    implementations below. If they are too slow for your purpose,
    rewrite them in order to improve them!

    Catch all errors that may occur for mmap and munmap. In these cases
    make malloc/calloc/realloc/free just fail. Do not print out any 
    debug messages as this might get you into an infinite recursion!

    Your __calloc_impl will probably just call your __malloc_impl, check
    if that allocation worked and then set the fresh allocated memory
    to all zeros. Be aware that calloc comes with two size_t arguments
    and that malloc has only one. The classical multiplication of the two
    size_t arguments of calloc is wrong! Read this to convince yourself:

    https://bugzilla.redhat.com/show_bug.cgi?id=853906

    In order to allow you to properly refuse to perform the calloc instead
    of allocating too little memory, the __try_size_t_multiply function is
    provided below for your convenience.
    
*/

#include <stddef.h>
#include <sys/mman.h>
#define MEMORY_MAP_MIN_SIZE (4194304)

static void add_free_memory_block();
static void prune_memory_map();
static void coalesce_memory_blocks();
static int __try_size_t_multiply();

struct memory_block_struct_t{
  size_t size;
  void *mmap_start;
  size_t mmap_size;
  struct memory_block_struct_t *next;
}typedef memory_block_t;

static memory_block_t *free_memory_blocks = NULL;


static void add_free_memory_block(memory_block_t *ptr, int prune){
  memory_block_t *curr, *prev;
  if(ptr == NULL) return;
  for(curr = free_memory_blocks, prev = NULL;
      curr != NULL;
      curr = (prev = curr)->next){
    if(((void *) ptr) < ((void *) curr)){
      break;
    }
  }
  if(prev == NULL){
    ptr->next = free_memory_blocks;
    free_memory_blocks = ptr;
    coalesce_memory_blocks(ptr, prune);
  } else {
    ptr->next = curr;
    prev->next = ptr;
    coalesce_memory_blocks(ptr,prune);
  }
}
//May need to check how may times segment of code repeats
static void coalesce_memory_blocks(memory_block_t *ptr, int prune){
  memory_block_t *clobbered;
  int did_coalesce;

  if((ptr == NULL) || (ptr->next == NULL)){
    if (prune) prune_memory_map();
    return;
  }

  did_coalesce = 0;
  if((ptr->mmap_start = ptr->next->mmap_start) && ((((void *) ptr) + ptr->size) == ((void *)(ptr->next)))){
    clobbered = ptr->next;
    ptr->next = clobbered->next;
    ptr->size += clobbered->size;
    did_coalesce =1;
  }
  if(ptr->next == NULL){
    if(did_coalesce && prune){
      prune_memory_map();
    }
    return;
  }

  if((ptr->mmap_start = ptr->next->mmap_start) && ((((void *) ptr) + ptr->size) == ((void *)(ptr->next)))){
    clobbered = ptr->next;
    ptr->next = clobbered->next;
    ptr->size += clobbered->size;
    did_coalesce = 1;
  }
  if(did_coalesce && prune) prune_memory_map();
  
  if(ptr->next == NULL){
    if(did_coalesce && prune){
      prune_memory_map();
    }
    return;
  }

  if(ptr->next->next == NULL){
    if(did_coalesce && prune){
      prune_memory_map();
    }
    return;
  }
  if((ptr->next->mmap_start = ptr->next->next->mmap_start) &&
     ((((void *) ptr->next) + ptr->next->size) == ((void *)(ptr->next->next)))){
    clobbered = ptr->next->next;
    ptr->next->next = clobbered->next;
    ptr->next->size += clobbered->size;
    did_coalesce =1;
  }
  if(did_coalesce && prune) prune_memory_map();
}

static void prune_memory_map(){
  memory_block_t *curr, *prev, *next;

  for(curr = free_memory_blocks, prev = NULL;
      curr != NULL;
      curr = (prev = curr->next)){
    if((curr->size == curr->mmap_size) &&
       (curr->mmap_start = ((void *) curr))){
      next = curr->next;
      if(munmap(curr->mmap_start, curr->mmap_size) == 0){
	if(prev == NULL){
	  free_memory_blocks = next;
	} else {
	  prev->next = next;
	}
	return;
      }
    }
  }
}

static memory_block_t *get_memory_block(size_t raw_size){
  size_t new_memory_block, size;
  memory_block_t *curr, *prev, *new;

  if(raw_size == ((size_t) 0)) return NULL;
  size = raw_size - ((size_t) 1);
  new_memory_block = size + sizeof(memory_block_t);
  if(new_memory_block < size) return NULL;
  new_memory_block /= sizeof(memory_block_t);
  if(!__try_size_t_multiply(&size, new_memory_block, sizeof(memory_block_t))) return NULL;

  for(curr = free_memory_blocks, prev = NULL;
      curr != NULL;
      curr = (prev = curr)->next){
    if(curr->size >= size){
      if((curr->size - size) < sizeof(memory_block_t)){
	if(prev == NULL){
	  free_memory_blocks = curr->next;
	} else {
	  prev->next = curr->next;
	}
	return curr;
      } else {
	new = (memory_block_t*)(((void*) curr) + size);
	new->size = curr->size - size;
	new->mmap_start = curr->mmap_start;
	new->mmap_size = curr->mmap_size;
	new->next = curr->next;
	if(prev == NULL){
	  free_memory_blocks = new;
	} else {
	  prev->next = new;
	}
	curr->size = size;
	return curr;
      }
    }
  }
  return NULL;
}

static void new_memory_map(size_t raw_size){
  size_t size, min_memory_block, new_memory_block;
  void *ptr;
  memory_block_t *new;

  if(raw_size == ((size_t) 0)) return;
  size = raw_size - ((size_t) 1);
  new_memory_block = size + sizeof(memory_block_t);
  if(new_memory_block < size) return;
  new_memory_block /= sizeof(memory_block_t);
  min_memory_block = MEMORY_MAP_MIN_SIZE / sizeof(memory_block_t);//Not sure what MEMORY_MAP_MIN_SIZE is
  if(new_memory_block < min_memory_block) new_memory_block = min_memory_block;
  if((!__try_size_t_multiply(&size, new_memory_block, sizeof(memory_block_t)))) return;

  ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if(ptr == MAP_FAILED) return;
  new = (memory_block_t*)ptr;
  new->size = size;
  new->mmap_start = ptr;
  new->mmap_size = size;
  new->next = NULL;
  add_free_memory_block(new, 0);
}

/* Predefined helper functions */

static void *__memset(void *s, int c, size_t n) {
  unsigned char *p;
  size_t i;

  if (n == ((size_t) 0)) return s;
  for (i=(size_t) 0,p=(unsigned char *)s;
       i<=(n-((size_t) 1));
       i++,p++) {
    *p = (unsigned char) c;
  }
  return s;
}

static void *__memcpy(void *dest, const void *src, size_t n) {
  unsigned char *pd;
  const unsigned char *ps;
  size_t i;

  if (n == ((size_t) 0)) return dest;
  for (i=(size_t) 0,pd=(unsigned char *)dest,ps=(const unsigned char *)src;
       i<=(n-((size_t) 1));
       i++,pd++,ps++) {
    *pd = *ps;
  }
  return dest;
}

/* Tries to multiply the two size_t arguments a and b.

   If the product holds on a size_t variable, sets the 
   variable pointed to by c to that product and returns a 
   non-zero value.
   
   Otherwise, does not touch the variable pointed to by c and 
   returns zero.

   This implementation is kind of naive as it uses a division.
   If performance is an issue, try to speed it up by avoiding 
   the division while making sure that it still does the right 
   thing (which is hard to prove).

*/
static int __try_size_t_multiply(size_t *c, size_t a, size_t b) {
  size_t t, r, q;

  /* If any of the arguments a and b is zero, everthing works just fine. */
  if ((a == ((size_t) 0)) ||
      (b == ((size_t) 0))) {
    *c = a * b;
    return 1;
  }

  /* Here, neither a nor b is zero. 

     We perform the multiplication, which may overflow, i.e. present
     some modulo-behavior.

  */
  t = a * b;

  /* Perform Euclidian division on t by a:

     t = a * q + r

     As we are sure that a is non-zero, we are sure
     that we will not divide by zero.

  */
  q = t / a;
  r = t % a;

  /* If the rest r is non-zero, the multiplication overflowed. */
  if (r != ((size_t) 0)) return 0;

  /* Here the rest r is zero, so we are sure that t = a * q.

     If q is different from b, the multiplication overflowed.
     Otherwise we are sure that t = a * b.

  */
  if (q != b) return 0;
  *c = t;
  return 1;
}

/* End of predefined helper functions */

/* Your helper functions 

   You may also put some struct definitions, typedefs and global
   variables here. Typically, the instructor's solution starts with
   defining a certain struct, a typedef and a global variable holding
   the start of a linked list of currently free memory blocks. That 
   list probably needs to be kept ordered by ascending addresses.

*/


/* End of your helper functions */

/* Start of the actual malloc/calloc/realloc/free functions */

static void __free_impl();
static void * __malloc_impl();
static void *__realloc_impl();
static void * __calloc_impl();


void *__malloc_impl(size_t size) {
  /* STUB */
  size_t s;
  void* ptr;
  if (size < ((size_t) 0 )){
    return NULL;
  }
  s = size + sizeof(memory_block_t);
  if(s<size){
    return NULL;
  }
  ptr = (void*) get_memory_block(s);
  if(ptr != NULL){
    return ptr + sizeof(memory_block_t);
  }
  new_memory_map(s);
  ptr = (void*) get_memory_block(s);

  if(ptr != NULL){
    return ptr + sizeof(memory_block_t);
  }
  
  /*so we allocate by size bytes and return a pointer to allocated memory, 
    memory is not initialized
    
    if size is zero return null or a unique pointer to be later passed on to free
    
   */
  
  
  return NULL;
}

void *__calloc_impl(size_t nmemb, size_t size) {
  /* STUB */
  size_t s;
  void *ptr;
  /* so we will allocate memory for an array of nmemb elemets of size bytes
     and return a pointer to the allocated memory

     if nmemb or size is zero then we return null or a unique pointer to the allocated memory
     
     if nmemb and size multiplied results in over flow then return an error 

   */
  if(!__try_size_t_multiply(&s, nmemb, size)) return NULL;
  ptr = __malloc_impl(s);
  if(ptr != NULL){
    __memset(ptr,0,s);
  }
  return ptr;  
}

void *__realloc_impl(void *ptr, size_t size) {
  /* STUB */
  /* changes size of memory block pointed to by pointer by size

     if new size is larger than old size then added memory not initialized

     if ptr is NULL then call is equivelent to malloc(size) 
     for all values of size; if size equals zero and ptr is not null then the call is equivelent to
     free(ptr) unless ptr is Null it must be returned by an earlier call to malloc() calloc() or 
     realloc
     if area pointed to was moved free is done

   */
  void *new_ptr;
  memory_block_t *old_memory_block;
  size_t s = 0;

  if(ptr == NULL){
    return __malloc_impl(size);
  }
  if(size == ((size_t) 0)){
    __free_impl(ptr);
    return NULL;
  }
  new_ptr = __malloc_impl(size);
  if(new_ptr == NULL) return NULL;
  old_memory_block = (memory_block_t*)(ptr - sizeof(memory_block_t));
  if(old_memory_block->size < s) s = old_memory_block->size;
  __memcpy(new_ptr, ptr, s);
  __free_impl(ptr);
  return new_ptr;  
}

void __free_impl(void *ptr) {
  /* STUB */

  /*
    frees emmory space pointed to by ptr which must have been returned by a previous call malloc()
    calloc() or realloc()
    Otherwise or if free(ptr) has been called before undefined behavior occurs.
   */
  if(ptr ==NULL){
    return;
  }
  add_free_memory_block((ptr-sizeof(memory_block_t)), 1);
}
/* End of the actual malloc/calloc/realloc/free functions */
