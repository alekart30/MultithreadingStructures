#include <iostream>
#include <cstdlib>
#include <string>

#include "set.h"
#include "set_fgs.hpp"
#include "set_os.hpp"


Set<int>* p_set1 = nullptr;
Set<int>* p_set2 = nullptr;
Set<int>* working_set = nullptr;

size_t writers = 10;
size_t readers = 10;
size_t entries = 10;


int* shared_data = NULL;
int* complex_test_data = NULL;
size_t tests_n = 10;
size_t complex_readers_block_size = 0;


void on_error(const char* msg)
{
  std::cerr << msg << std::endl;
  if (p_set1)
    delete p_set1;
  if (p_set2)
    delete p_set2;
  exit(1);
}


void* readers_func(void* parameter)
{
  int* data = reinterpret_cast<int*>(parameter);
  for (size_t i = 0; i < entries; ++i)
    working_set->remove(data[i]);
  pthread_exit(0);
}


void* writers_func(void* parameter)
{
  int* data = reinterpret_cast<int*>(parameter);
  for (size_t i = 0; i < entries; ++i)
    working_set->add(data[i]);
  pthread_exit(0);
}


void create_and_run_threads(pthread_t* threads, pthread_attr_t* attributes, size_t threads_num, size_t thread_data_entries, void*(*thread_func)(void*))
{
  for (size_t i = 0; i < threads_num; ++i)
    pthread_attr_init(&(attributes[i]));
  for (size_t i = 0; i < threads_num; ++i)
    if (pthread_create(&(threads[i]), &(attributes[i]), thread_func, shared_data + i * thread_data_entries) != 0)
      on_error("Too many threads");
  for (size_t i = 0; i < threads_num; ++i)
    pthread_join(threads[i], NULL);
}

bool test_readers()
{
  pthread_t* threads = new pthread_t[readers];
  pthread_attr_t* attributes = new pthread_attr_t[readers];

  if (!threads || !attributes)
    on_error("Memory allocation problem");

  for (size_t i = 0; i < readers * entries; ++i)
    working_set->add(shared_data[i]);

  create_and_run_threads(threads, attributes, readers, entries, readers_func);

  for (size_t i = 0; i < readers * entries; ++i)
    if (working_set->contains(shared_data[i]))
      return false;

  delete[] threads;
  delete[] attributes;
  return true;
}

bool test_writers()
{
  pthread_t* threads = new pthread_t[writers];
  pthread_attr_t* attributes = new pthread_attr_t[writers];
  if (!threads || !attributes)
    on_error("Memory allocation problem");

  create_and_run_threads(threads, attributes, writers, entries, writers_func);

  for (size_t i = 0; i < writers * entries; ++i)
  {
    if (!working_set->contains(shared_data[i]))
      return false;
    working_set->remove(shared_data[i]);
  }

  delete[] threads;
  delete[] attributes;
  return true;
}





//Complex test
void* readers_complex_func(void* parameter)
{
  int* data = reinterpret_cast<int*>(parameter);
  for (size_t i = 0; i < complex_readers_block_size; ++i)
    if (working_set->contains(data[i]))
      ++complex_test_data[data[i]];
  pthread_exit(0);
}

void create_and_run_threads_complex_readers(pthread_t* threads, pthread_attr_t* attributes)
{
  for (size_t i = 0; i < readers; ++i)
    pthread_attr_init(&(attributes[i]));
  for (size_t i = 0; i < readers; ++i)
    if (pthread_create(&(threads[i]), &(attributes[i]), readers_complex_func, shared_data + i * complex_readers_block_size) != 0)
      on_error("Too many threads");
  for (size_t i = 0; i < readers; ++i)
    pthread_join(threads[i], NULL);
}

bool test_complex()
{
  pthread_t* threads = new pthread_t[writers];
  pthread_attr_t* attributes = new pthread_attr_t[writers];
  complex_readers_block_size = entries * ((double)writers / readers);
  complex_test_data = new int[writers * entries];
  if (!complex_test_data || !threads || !attributes)
    on_error("Memory allocation problem");

  for (size_t i = 0; i < writers * entries; ++i)
    complex_test_data[i] = 0;
  create_and_run_threads(threads, attributes, writers, entries, writers_func);

  delete[] threads;
  delete[] attributes;
  threads = new pthread_t[readers];
  attributes = new pthread_attr_t[readers];
  if (!threads || !attributes)
    on_error("Memory allocation problem");

  create_and_run_threads_complex_readers(threads, attributes);

  size_t test_size = writers * entries - (writers * entries) % complex_readers_block_size;
  for (size_t i = 0; i < test_size; ++i)
    if (complex_test_data[i] != 1)
      return false;

  for (size_t i = 0; i < writers * entries; ++i)
    working_set->remove(shared_data[i]);

  delete[] complex_test_data;
  delete[] threads;
  delete[] attributes;
  return true;
}


void prepare_shared_data(size_t threads_num, size_t thread_data_entries)
{
  shared_data = new int[threads_num * thread_data_entries];
  if (!shared_data)
    on_error("Memory allocation problem");
  for (size_t i = 0; i < threads_num * thread_data_entries; ++i)
    shared_data[i] = (int)i;
}



void test_set(Set<int>* p_set)
{
  working_set = p_set;

  prepare_shared_data(writers, entries);
  std::cout << "Test Writers...\n"; 
  if(test_writers())
    std::cout << "Success" << std::endl;
  else
    std::cout << "Fail" << std::endl;
  delete[] shared_data;

  prepare_shared_data(readers, entries);
  std::cout << "Test Readers...\n"; 
  if(test_readers())
    std::cout << "Success" << std::endl;
  else
    std::cout << "Fail" << std::endl;
  delete[] shared_data;
  
  prepare_shared_data(writers, entries);
  std::cout << "Test Complex...\n"; 
  if(test_complex())
    std::cout << "Success" << std::endl;
  else
    std::cout << "Fail" << std::endl;
  delete[] shared_data;

  
}



int main(int argc, char** argv)
{
  srand(time(NULL));
  if (argc != 1 && argc != 4)
    on_error("USAGE: app [readers, writers, entries]");

  
  if (argc == 4)
  {
    readers = atoi(argv[1]);
    writers = atoi(argv[2]);
    entries = atoi(argv[3]);
    if (!readers || !writers || !entries)
      on_error("USAGE: app [readers, writers, entries]");
  }

  
  SetFGS<int>::set_error_handler(on_error);
  SetOS<int>::set_error_handler(on_error);

  
  p_set1 = new SetFGS<int>();
  p_set2 = new SetOS<int>();
  if (!p_set1 || !p_set2)
    on_error("Memory allocation problem");

  
  std::cout << "\nFine-grained sync set:" << std::endl;
  test_set(p_set1);
  std::cout << "\nOptimistic sync set:" << std::endl;
  test_set(p_set2);
  
  
  delete p_set1;
  delete p_set2;
  return 0;
}
