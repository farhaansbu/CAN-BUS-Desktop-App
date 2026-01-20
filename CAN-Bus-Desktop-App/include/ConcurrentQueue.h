#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H


#include <mutex>
#include <optional>
#include <deque>
#include <functional>

template <typename T>
class ConcurrentQueue {

public:
	void push(const T& item)
	{
		std::lock_guard<std::mutex> lock{ queue_mut_ };	//take mutex
		queue_.push_back(item);
	}

	std::optional<T> try_pop()
	{
		std::lock_guard<std::mutex> lock{ queue_mut_ };
		if (queue_.empty())
			return std::nullopt;
		T out = queue_.front();
		queue_.pop_front();
		return out;

	}

	
	size_t drain(size_t max_items, std::function<void(const T&)> fn) {
		size_t count = 0;
		while (count < max_items) 
		{
			auto item = try_pop();
			if (!item) 
				break;

			fn(*item);
			++count;
		}
		return count;
	}

	size_t size() const
	{
		std::lock_guard<std::mutex> lock{ queue_mut_ };
		return queue_.size();
	}


private:
	mutable std::mutex queue_mut_;
	std::deque<T> queue_;


};





#endif 

