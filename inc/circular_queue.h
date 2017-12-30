#ifndef AMAZOOM_CIRCULAR_QUEUE_H
#define AMAZOOM_CIRCULAR_QUEUE_H

#include "shared.h"

class CircularQueue {
	cpen333::process::shared_object<CircularBuffer> cbuff_;
	cpen333::process::semaphore producer_;
	cpen333::process::semaphore consumer_;
	cpen333::process::mutex pmutex_;
	cpen333::process::mutex cmutex_;

public:
	CircularQueue() : cbuff_(AMAZOOM_BUFFER_MEMORY_NAME), producer_(AMAZOOM_PSEMAPHORE_NAME, CIRCULAR_BUFFER_SIZE), consumer_(AMAZOOM_CSEMAPHORE_NAME, 0), pmutex_(AMAZOOM_PMUTEX_NAME), cmutex_(AMAZOOM_CMUTEX_NAME){}

	void add(const Order& order)
	{
		int pidx;

		producer_.wait();
		std::unique_lock<cpen333::process::mutex> plock(pmutex_);
		pidx = cbuff_->pidx;
		cbuff_->pidx = (cbuff_->pidx+1)%CIRCULAR_BUFFER_SIZE;
		cbuff_->buffer[pidx] = order;
		plock.unlock();
		consumer_.notify();
	}

	Order get(void)
	{
		Order order;
		int cidx;

		consumer_.wait();
		std::unique_lock<cpen333::process::mutex> clock(cmutex_);
		cidx = cbuff_->cidx;
		cbuff_->cidx = (cbuff_->cidx+1)%CIRCULAR_BUFFER_SIZE;
		order = cbuff_->buffer[cidx];
		clock.unlock();
		producer_.notify();

		return order;
	}
};

#endif //AMAZOOM_CIRCULAR_QUEUE_H
