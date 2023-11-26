/*
 * cdict.c
 *
 * Dictionary based on a hash table utilizing open addressing to
 * resolve collisions.
 *
 * Author: Niyomwungeri Parmenide Ishimwe <parmenin@andrew.cmu.edu>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "cdict.h"

#define DEBUG

#define DEFAULT_DICT_CAPACITY 8
#define REHASH_THRESHOLD 0.6

// Edit cdict.c and cdict.h so that the CDict type maps from char * to double. instead of char * to char *.
typedef enum
{
  SLOT_UNUSED = 0,
  SLOT_IN_USE,
  SLOT_DELETED
} CDictSlotStatus;

struct _hash_slot
{
  CDictSlotStatus status;
  CDictKeyType key;
  CDictValueType value;
};

struct _dictionary
{
  unsigned int num_stored;
  unsigned int num_deleted;
  unsigned int capacity;
  struct _hash_slot *slot;
};

// Documented in .h file
CDict CD_new()
{
  CDict dict = (CDict)malloc(sizeof(struct _dictionary));

  if(!dict)
    return NULL;

  dict->num_stored = 0;
  dict->num_deleted = 0;
  dict->capacity = DEFAULT_DICT_CAPACITY;

  dict->slot = (struct _hash_slot *)malloc(sizeof(struct _hash_slot) * dict->capacity);

  if (dict->slot == NULL)
  {
    CD_free(dict);
    return NULL;
  }

  // Initialize the slots
  for (unsigned int i = 0; i < dict->capacity; i++)
  {
    dict->slot[i].status = SLOT_UNUSED;
    dict->slot[i].key = NULL;
    dict->slot[i].value = NAN; // To check if -1 is valid initial value
  }

  return dict;
}

// Documented in .h file
void CD_free(CDict dict)
{
  if (dict)
  {
    if (dict->slot)
    {
      for (unsigned int i = 0; i < dict->capacity; i++)
      {
        if (dict->slot[i].status == SLOT_IN_USE)
          free(dict->slot[i].key);
      }
      free(dict->slot);
    }

    free(dict);
  }
}

/*
 * Return a pseudorandom hash of a key with reasonable distribution
 * properties. Based on Python's implementation before Python 3.4
 *
 * Parameters:
 *   str   The string to be hashed
 *   capacity  The capacity of the dictionary
 *
 * Returns: The hash, in the range 0-(capacity-1) inclusive
 */
static unsigned int _CD_hash(CDictKeyType str, unsigned int capacity)
{
  unsigned int x;
  unsigned int len = 0;

  for (const char *p = str; *p; p++)
    len++;

  if (len == 0)
    return 0;

  const char *p = str;
  x = (unsigned int)*p << 7;

  for (int i = 0; i < len; i++)
    x = (1000003 * x) ^ (unsigned int)*p++;

  x ^= (unsigned int)len;

  return x % capacity;
}

/*
 * Rehash the dictionary, doubling its capacity
 *
 * Parameters:
 *   dict     The dictionary to rehash
 *
 * Returns: None
 */
static void _CD_rehash(CDict dict)
{
  assert(dict != NULL);

  unsigned int new_capacity = dict->capacity * 2;
  unsigned int new_num_stored = 0;
  struct _hash_slot *new_slot = malloc(sizeof(struct _hash_slot) * new_capacity);

  if (new_slot == NULL)
  {
    CD_free(dict);
    return;
  }

  // Initialize the new slots
  for (unsigned int i = 0; i < new_capacity; i++)
    new_slot[i].status = SLOT_UNUSED;

  for (unsigned int i = 0; i < dict->capacity; i++)
  {
    if (dict->slot[i].status == SLOT_IN_USE)
    {
      unsigned int hash = _CD_hash(dict->slot[i].key, new_capacity); // avoid same hash ON NEW CAPACITY

      while (new_slot[hash].status == SLOT_IN_USE)
        hash = (hash + 1) % new_capacity;

      new_slot[hash].status = SLOT_IN_USE;
      new_slot[hash].key = dict->slot[i].key;
      new_slot[hash].value = dict->slot[i].value;
      new_num_stored++;
    }
  }

  // Free the old slot array
  free(dict->slot);

  // Update the old slot array to the new slot array
  dict->slot = new_slot;
  dict->capacity = new_capacity;
  dict->num_deleted = 0;
}

// Documented in .h file
unsigned int CD_size(CDict dict)
{
#ifdef DEBUG
  // iterate across slots, counting number of keys found
  int used = 0;
  int deleted = 0;

  for (int i = 0; i < dict->capacity; i++)
    if (dict->slot[i].status == SLOT_IN_USE)
      used++;
    else if (dict->slot[i].status == SLOT_DELETED)
      deleted++;

  assert(used == dict->num_stored);
  assert(deleted == dict->num_deleted);
#endif

  return dict->num_stored;
}

// Documented in .h file
unsigned int CD_capacity(CDict dict)
{
  if (dict == NULL)
    return 0;

  return dict->capacity;
}

// Documented in .h file
bool CD_contains(CDict dict, CDictKeyType key)
{
  if (dict == NULL || key == NULL)
    return false;

  unsigned int hash = _CD_hash(key, dict->capacity);

  for (unsigned int i = 0; i < dict->capacity; i++)
  {
    unsigned int index = (hash + i) % dict->capacity;

    // Can't find it
    if (dict->slot[index].status == SLOT_UNUSED)
      return false;

    // Found it
    else if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0)
      return true;
  }

  return false;
}

// Documented in .h file
void CD_store(CDict dict, CDictKeyType key, CDictValueType value)
{
  if (dict == NULL)
    return;

  if (isnan(value))
    return;

  // Hash the key to get an index
  unsigned int hash = _CD_hash(key, dict->capacity);

  // Check for collisions and deleted slots
  while ((dict->slot[hash].status == SLOT_IN_USE && strcmp(dict->slot[hash].key, key) != 0) ||
         dict->slot[hash].status == SLOT_DELETED)
    hash = (hash + 1) % dict->capacity; // move to next slot

  // Found a slot with the same key, update the value
  if (dict->slot[hash].status == SLOT_IN_USE && strcmp(dict->slot[hash].key, key) == 0)
  {
    dict->slot[hash].value = value;
    return;
  }

  // Found an empty, insert here
  else if (dict->slot[hash].status == SLOT_UNUSED)
  {
    dict->slot[hash].status = SLOT_IN_USE;
    dict->slot[hash].key = strdup(key);
    dict->slot[hash].value = value;
    dict->num_stored++;

    // Check if rehashing is needed after storing new key
    if (CD_load_factor(dict) > REHASH_THRESHOLD)
      _CD_rehash(dict);

    return;
  }
}

// Documented in .h file
CDictValueType CD_retrieve(CDict dict, CDictKeyType key)
{
  if (dict == NULL || key == NULL)
    return INVALID_VALUE;

  unsigned int hash = _CD_hash(key, dict->capacity);

  for (unsigned int i = 0; i < dict->capacity; i++)
  {
    unsigned int index = (hash + i) % dict->capacity;

    if (dict->slot[index].status == SLOT_UNUSED)
      return INVALID_VALUE;

    else if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0)
      return dict->slot[index].value;
  }

  return INVALID_VALUE;
}

// Documented in .h file
void CD_delete(CDict dict, CDictKeyType key)
{
  assert(dict != NULL);
  assert(key != NULL);

  unsigned int hash = _CD_hash(key, dict->capacity);

  for (unsigned int i = 0; i < dict->capacity; i++)
  {
    unsigned int index = (hash + i) % dict->capacity;

    // Can't find it
    if (dict->slot[index].status == SLOT_UNUSED)
    {
      printf("Error: cannot delete key [%s] not found\n", key);
      return;
    }

    // Found it
    else if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0)
    {
      dict->slot[index].status = SLOT_DELETED;
      dict->num_stored--;
      dict->num_deleted++;
      dict->slot[index].key = NULL;
      dict->slot[index].value = NAN;
      return;
    }
  }
}
// Documented in .h file
double CD_load_factor(CDict dict)
{
  if (dict == NULL || dict->capacity <= 0)
    return 0;

  return (double)(dict->num_stored + dict->num_deleted) / dict->capacity;
}

// Documented in .h file
void CD_print(CDict dict)
{
  assert(dict != NULL);

  printf("*** capacity: %u stored: %u deleted: %u load_factor: %.2f\n",
         dict->capacity, dict->num_stored, dict->num_deleted, CD_load_factor(dict));

  for (unsigned int i = 0; i < dict->capacity; i++)
  {
    printf("%02u: ", i);

    if (dict->slot[i].status == SLOT_UNUSED)
      printf("unused\n");

    else if (dict->slot[i].status == SLOT_DELETED)
      printf("DELETED\n");

    else if (dict->slot[i].status == SLOT_IN_USE)
      printf("IN_USE key=%s hash=%u value=%g\n", dict->slot[i].key, _CD_hash(dict->slot[i].key, dict->capacity), dict->slot[i].value);
  }
}

void CD_foreach(CDict dict, CD_foreach_callback callback, void *cb_data)
{
  if (dict == NULL || callback == NULL)
    return;

  for (unsigned int i = 0; i < dict->capacity; i++)
    if (dict->slot[i].status == SLOT_IN_USE)
      callback(dict->slot[i].key, dict->slot[i].value, cb_data);
}