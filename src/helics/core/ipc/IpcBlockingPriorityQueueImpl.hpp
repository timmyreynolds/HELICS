/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics_includes/optional.hpp"
#include <chrono>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

namespace helics
{
namespace ipc
{
namespace detail
{
struct dataIndex
{
    int32_t offset;
    int32_t dataSize;
};
/** class containing the raw data block implementation*/
class dataBlock
{
  private:
    unsigned char *origin = nullptr;
    unsigned char *next = nullptr;
    dataIndex *nextIndex = nullptr;
    int capacity = 0;
    int dataCount = 0;

  public:
    dataBlock (unsigned char *newBlock, int blockSize);

    void swap (dataBlock &other) noexcept;

    int getCurrentCount () const { return dataCount; }
    bool isSpaceAvaialble (int sz) const;
    bool empty () const { return (dataCount == 0); }

    bool push (const unsigned char *block, int blockSize);

    int next_data_size () const;

    int pop(unsigned char *block,int maxSize);

    /** reverse the order in which the data will be extracted*/
    void reverse ();
    /** clear all data from the dataBlock*/
	void clear ();
};

class PriorityBlock
{
  public:
    PriorityBlock (char *dataBlock, int capacity);

    int size () const { return size_; }
    int capacity () const { return capacity_; }
    // Return number of bytes written.
    int push (const unsigned char *data, int bytes);
    // Return number of bytes read.
    int pop (const unsigned char *data, int bytes);
    /** check if the block is Empty or not*/
	bool empty () const;
  private:
    int beg_index_, end_index_, size_, capacity_;
    char *data_;
};
/** class implementing a blocking queue with a priority channel
@details this class uses locks one for push and pull it can exhibit longer blocking times if the internal
operations require a swap, however in high usage the two locks will reduce contention in most cases.
*/
class IpcBlockingPriorityQueueImpl
{
  private:
    boost::interprocess::interprocess_mutex
      m_pushLock;  //!< lock for operations on the pushElements vector
    dataBlock pushData;
    boost::interprocess::interprocess_mutex m_pullLock;  //!< lock for elements on the pullData and priority structure
    dataBlock pullData;
    mutable boost::interprocess::interprocess_mutex m_conditionLock;  //!< lock for the empty and full Flag
    bool queueEmptyFlag{true};  //!< flag indicating the queue is empty
    bool queueFullFlag{false};
    // the condition variable should be keyed of the conditionLock
    boost::interprocess::interprocess_condition
      condition_empty;  //!< condition variable for notification of new data
    boost::interprocess::interprocess_condition
      condition_full;  //!< condition variable for notification of available space
    unsigned char *dataBlock = nullptr;
    size_t dataSize = 0;
    PriorityBlock priorityData;

  public:
    /** default constructor*/
    IpcBlockingPriorityQueueImpl (void *dataBlock, int blockSize);

    /** clear the queue*/
    void clear ();

    /** DISABLE_COPY_AND_ASSIGN */
    IpcBlockingPriorityQueueImpl (const IpcBlockingPriorityQueueImpl &) = delete;
    IpcBlockingPriorityQueueImpl &operator= (const IpcBlockingPriorityQueueImpl &) = delete;

    /** push a data block
    val the value to push on the queue
    */
    void push (const unsigned char *data, int size);
    /** push an element onto the queue
    val the value to push on the queue
    */
    void pushPriority (const unsigned char *data, int size);
    /** push a data block
    val the value to push on the queue
    */
    bool try_push (const unsigned char *data, int size);

    /** push an element onto the queue
    val the value to push on the queue
    */
    bool try_pushPriority (const unsigned char *data, int size);

    /** try to pop an object from the queue
    @return an optional containing the value if successful the optional will be empty if there is no
    element in the queue
    */
    int try_pop (unsigned char *data, int maxSize);

    int pop (unsigned char *data, int maxSize);

    /** blocking call to wait on an object from the stack with timeout*/
    int pop (std::chrono::milliseconds timeout, unsigned char *data, int maxSize);

    /** check whether there are any elements in the queue
because this is meant for multi-process applications this may or may not have any meaning
depending on the number of consumers
*/
    bool empty () const;
};

}  // namespace detail
}  // namespace ipc
}  // namespace helics
