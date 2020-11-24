#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct __counter_t {
    int value;
    pthread_mutex_t lock;
} counter_t;

void init(counter_t *c) {
    c->value = 0;
    pthread_mutex_init(&c->lock, NULL);
}

void increment(counter_t *c) {
    pthread_mutex_lock(&c->lock);
    c->value++;
    pthread_mutex_unlock(&c->lock);
}

int get(counter_t *c) {
    pthread_mutex_lock(&c->lock);
    int rc = c->value;
    pthread_mutex_unlock(&c->lock);
    return rc;
}

typedef struct __node_t{
    char *key;
    struct __node_t *next;
} node_t;

typedef struct __list_t {
    node_t *head;
    pthread_mutex_t lock;
} list_t;

void List_Init(list_t *L) {
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);
}

void List_Insert(list_t *L, char *key) {
    //printf("aaa1\n");
    node_t *new = malloc(sizeof(node_t));
    //printf("aaa2\n");
    if (new == NULL) {
        perror("malloc");
        return; }
    //printf("aaa3\n");
    new->key = malloc(sizeof (strlen(key) + 1));
    //printf("aaa4\n");
    strcpy(new->key, key);
    //printf("aaa5\n");
    //new->key = key;
    pthread_mutex_lock(&L->lock);
    new->next = L->head;
    L->head   = new;
    pthread_mutex_unlock(&L->lock);
}

int List_Lookup(list_t *L, char *key) {
    int rv = -1;
    pthread_mutex_lock(&L->lock);
    node_t *curr = L->head;
    //if(L->head == NULL) return -1;
    while (curr) {
        if (strcmp(curr->key, key)==0) {
            rv = 0;
            break; }
        curr = curr->next;
    }
    pthread_mutex_unlock(&L->lock);
    return rv;
}

#define BUCKETS 256

typedef struct __hash_t {
    list_t lists[BUCKETS];
} hash_t;

/* hash: form hash value for string s */
unsigned getHash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return hashval % BUCKETS;
}

void Hash_Init(hash_t *H) {
    int i;
    for (i = 0; i < BUCKETS; i++)
        List_Init(&H->lists[i]);
}

void Hash_Insert(hash_t *H, char *key) {
    int k = getHash(key);
    return List_Insert(&H->lists[k % BUCKETS], key);
}

int Hash_Lookup(hash_t *H, char *key) {
    int k = getHash(key);
    return List_Lookup(&H->lists[k % BUCKETS], key);
}

void *countWords(hash_t *table, counter_t *count, char *arg) {

    printf("countWords for %s\n", arg);
    FILE *file;
    file = fopen(arg, "r");
    if(file == NULL){
        printf("file is null\n");
    }
    char* ptr;
    //fscanf(file, "%ms", &ptr);
    while (fscanf(file, "%ms", &ptr)>0){
        //strcpy(word, ptr);
        printf("inside while\n");
        if(Hash_Lookup(table, ptr)==-1){
            Hash_Insert(table, ptr);
            increment(count);
        }
        //printf("word is %s\n", ptr);
        //fscanf(file, "%ms", &ptr);
        //break;
    }
    free(ptr);
    fclose(file);
    return NULL;

}

int main(int argc, char **argv) {
    printf("starting\n");
    counter_t *count = malloc(sizeof(counter_t));
    init(count);
    printf("good till here1\n");
    hash_t *table = malloc(sizeof(hash_t));
    Hash_Init(table);
    printf("good till here2\n");


    printf("starting loop\n");
    for(int i=1; i<argc; i++){
        printf("i=%d\n", i);
        pthread_t p;
        pthread_create(&p, NULL, countWords(table, count, argv[i]), NULL);
        pthread_join(p, NULL);
    }

    int result = get(count);
    free(table);
    free(count);
    printf("number of words in the table: %d\n", result);
    printf("done\n");
}
