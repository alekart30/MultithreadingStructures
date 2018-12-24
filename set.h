#ifndef __SET_H__
#define __SET_H__

//Error messages
/*static const char* g_msg_err_mutex_lock = "Mutex lock error";
static const char* g_msg_err_mutex_unlock = "Mutex unlock error";
static const char* g_msg_err_node_create = "Node creation error";
static const char* g_msg_err_error_handler = "Error handler was not initialized";*/


template <class T>
class Set
{
public:
  virtual ~Set() {}
  
virtual bool add(const T& item) = 0;	
virtual bool remove(const T& item) = 0;
virtual bool contains(const T& item) = 0;
};

#endif

