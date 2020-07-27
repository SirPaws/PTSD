#include "allocator.h"
#include "vector.h"

extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

pCreateVectorStruct(GenericVector, u8);

GenericVector *pInitVector(VectorInfo info) {
    usize size = (info.datasize * info.initialsize) + sizeof(GenericVector);
    GenericVector *vector = (GenericVector *)pCurrentAllocatorFunc(NULL, size, 0, MALLOC, pCurrentAllocatorUserData);
    memset(vector, 0, size);
    vector->datasize = info.datasize;
    vector->endofstorage = info.initialsize * info.datasize;
    vector->size = 0; 
    vector->data = (u8 *)(vector + 1);
    return vector;
}

void pFreeVector(GenericVector *this_ptr) {
    pCurrentAllocatorFunc(this_ptr, 0, 0, FREE, pCurrentAllocatorUserData);
}

#include <string.h>

#define EOS_ZERO_TEST (this->endofstorage ? this->endofstorage * 2 : this->datasize * 2)

static void *pInsertBeginning(GenericVector **this_ptr, const void *const value, usize size, bool resize){
	GenericVector * this_vector = *this_ptr;
    if (resize){
        this_vector = (GenericVector *)pCurrentAllocatorFunc(
                this_vector, size, 0, REALLOC, pCurrentAllocatorUserData);
        *this_ptr = this_vector;
        this_vector->data = (u8 *)(this_vector + 1);

        if (this_vector->endofstorage > 0){

            void *old_data = pCurrentAllocatorFunc(NULL, this_vector->endofstorage, 0, MALLOC, pCurrentAllocatorUserData);
            memmove(old_data, this_vector->data, this_vector->endofstorage);
            memmove(&this_vector->data[this_vector->datasize], old_data, this_vector->endofstorage);
            pCurrentAllocatorFunc(old_data, 0, 0, FREE, pCurrentAllocatorUserData);
        }
        this_vector->endofstorage += 2 * this_vector->datasize;
        this_vector->size += this_vector->datasize;
        memcpy(&this_vector->data[0], value, this_vector->datasize);
        return &this_vector->data[0];
    }else{
        if (this_vector->endofstorage > 0){
            void *old_data = pCurrentAllocatorFunc(NULL, this_vector->endofstorage, 0, MALLOC, pCurrentAllocatorUserData);
            memmove(old_data, this_vector->data, this_vector->size);
            memmove(&this_vector->data[this_vector->datasize], old_data, this_vector->size);
            pCurrentAllocatorFunc(old_data, 0, 0, FREE, pCurrentAllocatorUserData);
        }
        this_vector->size += this_vector->datasize;
        memcpy(&this_vector->data[0], value, this_vector->datasize);
        return &this_vector->data[0];
    }
}

#define VectorSet(position) \
    memcpy(position, value, this_vector->datasize), this_vector->size += this_vector->datasize;\
    return position

void *pVectorInsert(GenericVector **this_ptr, const void *const pos, void *value){
    const u8 *const position = pos;

	GenericVector *this_vector = *this_ptr; 
	const usize n = (unsigned)(position - this_vector->data);
    size_t size = sizeof *this_vector + this_vector->endofstorage + (this_vector->datasize * 2);
    if (this_vector->size != this_vector->endofstorage){
		if (n == 0) { return pInsertBeginning(this_ptr, value, size, false); }
		if (n == this_vector->size){ 
            VectorSet(&this_vector->data[this_vector->size]) - this_vector->datasize;
        }
        if (n > this_vector->size && n < this_vector->endofstorage){
			size_t diff = n - this_vector->size;
            memset(&this_vector->data[this_vector->size], 0, this_vector->datasize * diff);
            VectorSet(&this_vector->data[n * this_vector->datasize]);
		}
        if (n < this_vector->size){
            size_t diff = this_vector->size - n;
            
            // create a new block of data 
            // this block holds the data 
            void *old_data = pCurrentAllocatorFunc(NULL, diff, 0, MALLOC, pCurrentAllocatorUserData);
		    memmove(old_data, &this_vector->data[n], diff);
		    memmove(&this_vector->data[(n + this_vector->datasize)], old_data, diff);
		    pCurrentAllocatorFunc(old_data, 0, 0, FREE, pCurrentAllocatorUserData); 

            old_data = NULL;
            VectorSet(&this_vector->data[n]);
        }
	}
	if (n == 0){
		return pInsertBeginning(this_ptr, value, size, true);
	}
    if (n < this_vector->size){
		size_t diff = this_vector->size - n;
        void *old_data = pCurrentAllocatorFunc(NULL, diff, 0, MALLOC, pCurrentAllocatorUserData);
		memmove(old_data, &this_vector->data[n], diff);
        this_vector = (GenericVector *)pCurrentAllocatorFunc(this_vector, size, 0, REALLOC, pCurrentAllocatorUserData);
		memmove(&this_vector->data[(n + this_vector->datasize)], old_data, diff);
		pCurrentAllocatorFunc(old_data, 0, 0, FREE, pCurrentAllocatorUserData); 
        this_vector->endofstorage += this_vector->datasize * 2;
        *this_ptr = this_vector;
        VectorSet(&this_vector->data[n]);
    }
    if (n == this_vector->endofstorage){
        this_vector = (GenericVector *)pCurrentAllocatorFunc(this_vector, size, 0, REALLOC, pCurrentAllocatorUserData);
        *this_ptr = this_vector;
        this_vector->endofstorage += this_vector->datasize * 2;
	    this_vector->size += this_vector->datasize;
        memcpy(&this_vector->data[n], value, this_vector->datasize);
        return &this_vector->data[n];
    }

	return NULL;
}

void *pVectorPushBack(GenericVector * *this_ptr, void *value){
    GenericVector * this_vector = *this_ptr;
	size_t size = sizeof *this_vector + this_vector->endofstorage + (this_vector->datasize * 2);
    if (this_vector->size == this_vector->endofstorage){
        this_vector = (GenericVector *)pCurrentAllocatorFunc(this_vector, size, 0, REALLOC, pCurrentAllocatorUserData);
        *this_ptr = this_vector;
        this_vector->endofstorage += this_vector->datasize * 2;
    }
    memcpy(&this_vector->data[this_vector->size], value, this_vector->datasize);
    this_vector->size += this_vector->datasize;
    size = 0;
    return (char *)this_vector->data + this_vector->size;
}

void *pVectorErase(GenericVector *this_vector, const void *const position){
    isize offset = (const char *const)position - (const char *const)pVectorBegin(this_vector);
    char *pos = (char *)pVectorBegin(this_vector) + offset;
    if (pos + (this_vector->datasize) != pVectorEnd(this_vector)){
        isize size = ((char *)pVectorEnd(this_vector)) - ((char *)pos + this_vector->datasize);
        memcpy(pos, (char *)pos + this_vector->datasize, (unsigned)size);
    }
    this_vector->size -= this_vector->datasize;
    return pos;
}

void *pVectorBegin(GenericVector *this_vector){
    return &this_vector->data[0];
}

void *pVectorEnd(GenericVector *this_vector){
    return &this_vector->data[this_vector->size];
}

usize pVectorSize(GenericVector *this_ptr) {
    return this_ptr->size / this_ptr->datasize;
}

