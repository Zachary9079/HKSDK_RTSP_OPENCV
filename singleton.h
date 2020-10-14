#ifndef SINGLETON_H
#define SINGLETON_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

template <class T>
class Queue {
protected:
	// Data
	std::queue<T> _queue;
	typename std::queue<T>::size_type _size_max;

	// Thread gubbins
	std::mutex _mutex;
	std::condition_variable _fullQue;
	std::condition_variable _empty;

	// Exit
	// 原子操作
	std::atomic_bool _quit; //{ false };
	std::atomic_bool _finished; // { false };

public:
	Queue() :_size_max(100) {
		_quit = ATOMIC_VAR_INIT(false);
		_finished = ATOMIC_VAR_INIT(false);
	}
	Queue(const size_t size_max) :_size_max(size_max) {
		_quit = ATOMIC_VAR_INIT(false);
		_finished = ATOMIC_VAR_INIT(false);
	}

	bool push(T& data)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		while (!_quit && !_finished)
		{
			if (_queue.size() < _size_max)
			{
				_queue.push(std::move(data));
				//_queue.push(data);
				_empty.notify_all();
				return true;
			}
			else
			{
				// wait的时候自动释放锁，如果wait到了会获取锁
				_fullQue.wait(lock);
			}
		}

		return false;
	}


	bool pop(T &data)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		while (!_quit)
		{
			if (!_queue.empty())
			{
				//data = std::move(_queue.front());
				data = _queue.front();
				_queue.pop();

				_fullQue.notify_all();
				return true;
			}
			else if (_queue.empty() && _finished)
			{
				return false;
			}
			else
			{
				_empty.wait(lock);
			}
		}
		return false;
	}

	// The queue has finished accepting input
	void finished()
	{
		_finished = true;
		_empty.notify_all();
	}

	void quit()
	{
		_quit = true;
		_empty.notify_all();
		_fullQue.notify_all();
	}

	int length()
	{
		return static_cast<int>(_queue.size());
	}
};

//--------------------------------------------------------------

template <class T>
class Singleton
{
private:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton&)=delete;
    Singleton& operator=(const Singleton&)=delete;

public:
    template <typename... Args>
    static T* instance(Args&&... args)
    {
        std::call_once(_flag, [&](){
            _instance = new T(std::forward<Args>(args)...);
        });
        return _instance;
    }

    static void destroy()
    {
        if (_instance)
        {
            delete _instance;
            _instance = NULL;
        }
    }

private:
    static T* _instance;
    static std::once_flag _flag;
};

template <class T>
T* Singleton<T>::_instance = NULL;

template <class T>
std::once_flag Singleton<T>::_flag;

//--------------------------------------------------------------
template <class T>
class Singleton_HW
{
private:
	Singleton_HW() = default;
	~Singleton_HW() = default;
	Singleton_HW(const Singleton_HW&) = delete;
	Singleton_HW& operator=(const Singleton_HW&) = delete;

public:
	template <typename... Args>
	static T* instance(Args&&... args)
	{
		std::call_once(_flag, [&]() {
			_instance = new T(std::forward<Args>(args)...);
		});
		return _instance;
	}

	static void destroy()
	{
		if (_instance)
		{
			delete _instance;
			_instance = NULL;
		}
	}

private:
	static T* _instance;
	static std::once_flag _flag;
};

template <class T>
T* Singleton_HW<T>::_instance = NULL;

template <class T>
std::once_flag Singleton_HW<T>::_flag;
#endif // SINGLETON_H
