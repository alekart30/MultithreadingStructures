#ifndef __SETFGS_H__
#define __SETFGS_H__

#include <functional>
#include <pthread.h>
#include <bits/stdc++.h>

#include "set.h"


template <class T>
class SetFGS : public Set<T>
{
public:
  SetFGS()
  {
    
    head = new Node(0);
    if (!head)
      error_handler("Problem with node creation");
    head->next = new Node(INT_MAX);
    if (!head->next)
      error_handler("Problem with node creation");
  }

  ~SetFGS()
  {
	  Node *next = nullptr;
    for (Node *current = head; current != nullptr; current = next)
    {
      next = current->next;
      delete current;
    }
  }

  bool add(const T& item)
  {
    
    size_t key = generate_hash(item);
    
    head->lock();
    Node* previous = head;
    Node* current = head->next;
    current->lock();
    while (current->key < key)
    {
      previous->unlock();
      previous = current;
      current = current->next;
      current->lock();
    }
     
    if (current->key == key)
    {
      current->unlock();
      previous->unlock();
      return false;
    }

    
    Node* to_insert = new Node(item);
    if (!to_insert)
      error_handler("Problem with node creation");
    to_insert->next = current;
    previous->next = to_insert;
    current->unlock();
    previous->unlock();
    return true;
  }

  bool remove(const T& item)
  {
    
    size_t key = generate_hash(item);
    
    head->lock();
    Node* previous = head;
    Node* current = head->next;
    current->lock();
    while (current->key < key)
    {
      previous->unlock();
      previous = current;
      current = current->next;
      current->lock();
    }

    if (current->key == key)
    {
      previous->next = current->next;
      current->unlock();
      previous->unlock();
      delete current;
      return true;
    }
    current->unlock();
    previous->unlock();
    return false;
  }

  bool contains(const T& item)
  {
    
    size_t key = generate_hash(item);
    head->lock();
    
    Node* previous = head;
    Node* current = head->next;
    current->lock();
    while (current->key < key)
    {
      previous->unlock();
      previous = current;
      current = current->next;
      current->lock();
    }

    current->unlock();
    previous->unlock();
    
    return current->key == key;
  }

  static void set_error_handler(void(*handler)(const char*))
  {
    error_handler = handler;
  }

private:
  class Node
  {
  public:
    Node(T init_value) : item(init_value), key(std::hash<T>()(init_value)), next(nullptr), mutex(PTHREAD_MUTEX_INITIALIZER) {}

    T item; 
    size_t key; 
    Node* next; 

                
    void lock()
    {
      if (pthread_mutex_lock(&mutex) != 0)
        error_handler("Problem with node lock");
    }

    
    void unlock()
    {
      if (pthread_mutex_unlock(&mutex) != 0)
        error_handler("Problem with node unlock");
    }

  private:
    pthread_mutex_t mutex; 
  };

  Node* head; 
  static void(*error_handler)(const char*); 

  size_t generate_hash(const T& item)
  {
    return std::hash<T>()(item);
  }
};

template <class T>
void(*SetFGS<T>::error_handler)(const char*) = nullptr;

#endif

