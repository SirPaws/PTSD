#ifndef VECTOR_HEADER
#define VECTOR_HEADER

#include "general.h"

typedef struct GenericVector GenericVector;

// this macro creates a vector structure with a specific data type
// the variable name is the name of the structure and is optional
#define pCreateVectorStruct(name, data_type)\
    struct name {           \
        usize datasize;     \
        usize size;         \
        usize endofstorage; \
        data_type data[];   \
    }

//TODO: block allocation mode
//TODO: element destructor
typedef struct VectorInfo VectorInfo;
struct VectorInfo {
    usize datasize;
    usize initialsize;
};


/*
 * this function initializes a vector structure with the given VectorInfo.
 * you should call pFreeVector before the application exits.
 *
 * the return value of this function is a GenericVector that can be casted into another vector type
 * GenericVector is hidden but is equivalent to pCreateVectorStruct(GenericVector, char) 
 *
 */
GenericVector *pInitVector(VectorInfo info);
void pFreeVector(GenericVector *this_ptr);

// Expects value to be a pointer to the value and not the value it self
// ie. &(int){ 32 }
// not 32
void *pVectorInsert(GenericVector **this_ptr, void *position, void* value);

// Expects value to be a pointer to the value and not the value it self
// ie. &(int){ 32 }
// not 32
void *pVectorPushBack(GenericVector **this_ptr, void *value);

// Erases the element at position.
// this won't resize the vector but will potentionally move values around 
void *pVectorErase(GenericVector *this_ptr, void *position);

// Gets a pointer to the first element of the vector 
void *pVectorBegin(GenericVector *this_ptr);

// Gets a pointer to the element after the last valid element 
void *pVectorEnd(GenericVector *this_ptr);

// Gets the number of elements in the vector
usize pVectorSize(GenericVector *this_ptr);

#endif

