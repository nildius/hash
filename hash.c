#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"
#define LARGO 1500

enum nodo_state {BUSY,EMPTY,DELETED};

struct hash_node {
    char * key;
    void * value;
    enum nodo_state state;
} typedef hash_node_t;

struct hash {
    hash_node_t * hash_array;
    size_t length;
    size_t node_busy;
    size_t busy_space;
    hash_destruir_dato_t destroy_f;
};

struct hash_iter {
    const hash_t *hash;
    size_t pos;
};

// La función de Hash recibe por parametro una cadena y un rango, y devuelve
// un entero dentro de ese rango correspondiente al valor de Hash de esa cadena.
int hash_function(const char *key_string, size_t tam){
    size_t aux = strlen(key_string);
    while (aux > tam) {
        aux -= tam;
    }
    return (int)aux;
}


hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t * hash = malloc(sizeof(hash_t));
    if(!hash) return NULL;
    hash->length = LARGO;
    hash->node_busy = 0;
    hash->busy_space = 0;
    hash->destroy_f = destruir_dato;
    hash->hash_array = malloc(sizeof(hash_node_t) * hash->length);

    // Inicializo todos los nodos en EMPTY.
    for(int i = 0; i < hash->length; i++){
        hash->hash_array[i].state = EMPTY;
        hash->hash_array[i].key = malloc(sizeof(char[200]));
        hash->hash_array[i].value = NULL;
    }


    return hash;
}

hash_t *hash_crear_custom(hash_destruir_dato_t destruir_dato, size_t tam) {
    hash_t * hash = malloc(sizeof(hash_t));
    if(!hash) return NULL;
    hash->length = tam;
    hash->node_busy = 0;
    hash->busy_space =0;
    hash->destroy_f = destruir_dato;
    hash->hash_array = malloc(sizeof(hash_node_t) * hash->length);

    // Inicializo todos los nodos en EMPTY.
    for(int i = 0; i < hash->length; i++){
        hash->hash_array[i].state = EMPTY;
        hash->hash_array[i].key = malloc(sizeof(char[200]));
        hash->hash_array[i].value = NULL;
    }
    return hash;
}

void hash_copy(hash_t* old_hash, hash_t* new_hash) {
    for(int i = 0; i < old_hash->length ; i++){
        if(old_hash->hash_array[i].state == BUSY) {
            hash_guardar(new_hash, old_hash->hash_array[i].key, old_hash->hash_array[i].value);
        }
    }
    return;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato) {

    if(hash->busy_space > (hash->length - hash->length / 5)) {
        printf("Redimensiono porque no tengo espacio (Ocupado: %d, Libre: %d) \n", (int)hash->busy_space, (int)hash->length);
        hash_t* new_hash = hash_crear_custom(NULL, hash->length * 2);
        hash_copy(hash, new_hash);
        for(int i = 0; i < hash->length; i++){
              free(hash->hash_array[i].key);
            }
        free(hash->hash_array);
        *hash = *new_hash;
    }

    int hashed_key = hash_function(clave, hash->length);
    for(int i = hashed_key ; i < hash->length; i++) {
        if(hash->hash_array[i].state == EMPTY) {
            strcpy(hash->hash_array[i].key,clave);
            hash->hash_array[i].value = dato;
            hash->hash_array[i].state = BUSY;
            hash->node_busy++;
            hash->busy_space++;
            return true;
        }
        if(hash->hash_array[i].state == BUSY && strcmp(clave,hash->hash_array[i].key) == 0 ) {
            if(hash->destroy_f){
            hash->destroy_f(hash->hash_array[i].value);}
            hash->hash_array[i].value = dato;
            return true;
        }
    }
    for(int i = 0; i < hashed_key; i++) {
        if(hash->hash_array[i].state == EMPTY) {
            strcpy(hash->hash_array[i].key,clave);
            hash->hash_array[i].value = dato; //*dato
            hash->hash_array[i].state = BUSY;
            hash->node_busy++;
            hash->busy_space++;
            return true;
        }
        if(hash->hash_array[i].state == BUSY && strcmp(clave,hash->hash_array[i].key) == 0 ) {
            hash->hash_array[i].value = dato;
            return true;
        }
    }
    return false;
}

size_t hash_cantidad(const hash_t *hash){
    return hash->node_busy;
}

void *hash_obtener(const hash_t *hash, const char *clave) {
    int hashed_key = hash_function(clave, hash->length);

    if(hash->node_busy == 0)
        return NULL;

    for(int i = hashed_key ; i < hash->length; i++) {
        if(hash->hash_array[i].state == EMPTY) {
            return NULL;
        }
        if(hash->hash_array[i].state == BUSY && strcmp(clave,hash->hash_array[i].key) == 0 ) {
            return hash->hash_array[i].value;
        }
    }
    for(int i = 0; i < hashed_key; i++) {
        if(hash->hash_array[i].state == EMPTY) {
            return NULL;
        }
        if(hash->hash_array[i].state == BUSY && strcmp(clave,hash->hash_array[i].key) == 0 ) {
            return hash->hash_array[i].value;
        }
    }
    return NULL;
}

void *hash_borrar(hash_t *hash, const char *clave){

    int hashed_key = hash_function(clave, (int)hash->length);
    if(hash->node_busy == 0)
        return NULL;

    if(hash->node_busy == hash->length / 5 ) {
        hash_t* new_hash = hash_crear_custom(NULL, hash->length / 2);
        hash_copy(hash, new_hash);
        for(int i = 0; i < hash->length; i++){
              free(hash->hash_array[i].key);
            }
          free(hash->hash_array);

        //hash_destruir(hash);
        *hash = *new_hash;
    }

    for(int i = hashed_key ; i < hash->length; i++) {
        if(hash->hash_array[i].state == BUSY && strcmp(clave,hash->hash_array[i].key) == 0 ) {
            void* aux = hash->hash_array[i].value;
            free(hash->hash_array[i].key);
            hash->hash_array[i].key = NULL;
            hash->hash_array[i].value = NULL;
            hash->hash_array[i].state = DELETED;
            hash->node_busy--;
            return aux;
        }
    }
    for(int i = 0; i < hashed_key; i++) {
        if(hash->hash_array[i].state == BUSY && strcmp(clave,hash->hash_array[i].key) == 0 ) {
            void* aux = hash->hash_array[i].value;
            free(hash->hash_array[i].key);
            hash->hash_array[i].key = NULL;
            hash->hash_array[i].value = NULL;
            hash->hash_array[i].state = DELETED;
            hash->node_busy--;
            return aux;
        }
    }
    return NULL;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
    int hashed_key = hash_function(clave, (int)hash->length);

    if(hash->node_busy == 0)
        return false;

    for(int i = hashed_key ; hashed_key < hash->length; i++) {
        if(hash->hash_array[i].state == EMPTY) {
            return false;
        }
        if(hash->hash_array[i].state == BUSY && strcmp(clave,hash->hash_array[i].key) == 0 ) {
            return true;
        }
    }
    for(int i = 0; i < hashed_key; i++) {
        if(hash->hash_array[i].state == EMPTY) {
            return false;
        }
        if(hash->hash_array[i].state == BUSY && strcmp(clave,hash->hash_array[i].key) == 0 ) {
            return true;
        }
    }
    return false;
}

void hash_destruir(hash_t *hash){
    for(int i = 0; i < hash->length; i++){
      if(hash->destroy_f){
          hash->destroy_f(hash->hash_array[i].value);
      }
      free(hash->hash_array[i].key);
    }
  free(hash->hash_array);
  free(hash);
}

// AUXILIAR
void hash_mostrar(hash_t* hash) {
    for(int x = 0; x < hash->length; x++) {
        if(hash->hash_array[x].state == BUSY)
            printf("Indice %d (colision %d) - [%s]\n", x, hash_function(hash->hash_array[x].key, hash->length), hash->hash_array[x].key);
        else
            printf("Indice %d - BORRADO / VACIO\n", x);
    }
}

// COMIENZO FUNCIONES DE ITERADOR

hash_iter_t *hash_iter_crear(const hash_t *hash) {
    hash_iter_t *iter = malloc(sizeof(hash_iter_t));
    if(!iter)
        return NULL;

    iter->hash = hash;
    iter->pos = 0;

    if(iter->hash->hash_array[iter->pos].state != BUSY)
        hash_iter_avanzar(iter);
    return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){

    if(!hash_iter_al_final(iter)) {
        iter->pos++;
        if(hash_iter_al_final(iter)) return true;
        if(iter->hash->hash_array[iter->pos].state != BUSY){
            return hash_iter_avanzar(iter);}

        return true;
    }
    return false;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
    if(hash_iter_al_final(iter))
        return NULL;

    size_t pos = iter->pos;
    return iter->hash->hash_array[pos].key;
}

bool hash_iter_al_final(const hash_iter_t *iter){
    return iter->pos == iter->hash->length;
}

void hash_iter_destruir(hash_iter_t* iter){
    free(iter);
}
