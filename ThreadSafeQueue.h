#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
public:
	ThreadSafeQueue() : queue_(), mutex_(), cond_var_() {}

	ThreadSafeQueue(const ThreadSafeQueue&) = delete;
	ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

	void push(const T& item) {
		{
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push(item);
		}
		cond_var_.notify_one();
	}

	bool pop(T& item) {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_var_.wait(lock, [this] { return !queue_.empty(); });
		if (queue_.empty())
			return false;

		item = queue_.front();
		queue_.pop();
		return true;
	}

	void clear() {
		std::lock_guard<std::mutex> lock(mutex_);
		std::queue<T> empty;
		std::swap(queue_, empty);
	}

	bool empty() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.empty();
	}

private:
	std::queue<T> queue_;
	mutable std::mutex mutex_;
	std::condition_variable cond_var_;
};
