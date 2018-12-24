#ifndef __SETOS_H__
#define __SETOS_H__

#include <functional>
#include <pthread.h>
#include <bits/stdc++.h>

#include "set.h"

template <class T>
class SetOS : public Set<T>
{
public:
  SetOS()
  {
    head = new Node(0);
    if (!head)
      error_handler("Problem with node creation");
    head->next = new Node(INT_MAX);
    if (!head->next)
      error_handler("Problem with node creation");
  }

  ~SetOS()
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
    while (true)
    {
      Node* previous = head;
      Node* current = head->next;
      while (current->key < key)
      {
        previous = current;
        current = current->next;
      }

      
      previous->lock();
      current->lock();
      
      if (validate(previous, current))
      {
        if (current->key == key)
        {
          previous->unlock();
          current->unlock();
          return false;
        }
        else
        {
         
          Node* to_insert = new Node(item);
          if (!to_insert)
            error_handler("Problem with node creation");
          to_insert->next = current;
          previous->next = to_insert;
          previous->unlock();
          current->unlock();
          return true;
        }
      }
      previous->unlock();
      current->unlock();
    }
  }

  bool remove(const T& item)
  {
    size_t key = generate_hash(item);
    while (true)
    {
      
      Node* previous = head;
      Node* current = head->next;
      while (current->key < key)
      {
        previous = current;
        current = current->next;
      }

      previous->lock();
      current->lock();
      
      if (validate(previous, current))
      {
        if (current->key == key)
        {
          previous->next = current->next;
          delete current;
          previous->unlock();
          current->unlock();
          return true;
        }
        else
        {
          previous->unlock();
          current->unlock();
          return false;
        }
      }
      previous->unlock();
      current->unlock();
    }
  }

  bool contains(const T& item)
  {
    size_t key = generate_hash(item);
    while (true)
    {
      Node* previous = head;
      Node* current = head->next;
      while (current->key < key)
      {
        previous = current;
        current = current->next;
      }

      previous->lock();
      current->lock();
      if (validate(previous, current))
      {
        previous->unlock();
        current->unlock();
        return current->key == key;
      }
      previous->unlock();
      current->unlock();
    }
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

                                            
  bool validate(Node* previous, Node* current)
  {
    Node* node = head;
    while (node->key <= previous->key)
    {
      if (node == previous)
        return previous->next == current;
      node = node->next;
    }
    return false;
  }

  size_t generate_hash(const T& item)
  {
    return std::hash<T>()(item);
  }
};

template <class T>
void(*SetOS<T>::error_handler)(const char*) = nullptr;

#endif

