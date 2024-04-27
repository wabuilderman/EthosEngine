#pragma once
#include <iostream>
#include <vector>
#include <typeinfo>

#ifndef NDEBUG
inline void DebugLog(const char * msg) { std::cerr << msg << std::endl; }
#else
inline void DebugLog(const char * msg) {}
#endif

template <typename EventType, class ClassType>
struct MemberListener {
	ClassType* classPtr;
	void(ClassType::*callback)(EventType&);

	void operator()(EventType& e) {
		(this->classPtr->* this->callback)(e);
	}
};

template <typename EventType, class ClassType>
class MemberDispatcher {
public:
	static std::vector<MemberListener<EventType, ClassType>> listeners;

	static void Call(EventType& e) {
		for (auto it = listeners.cbegin(); it != listeners.cend();) {
			if (it->classPtr == nullptr) {
				it = listeners.erase(it);
				if (listeners.empty()) {
					break;
				}
			}
			else {
				it++;
			}
		}
		
		size_t numListeners = listeners.size();
		for (size_t i = 0; i < numListeners; ++i)
			listeners[i](e);
	}

	using MemberCallback = void(ClassType::*)(EventType&);

	static void Connect(MemberCallback _func, ClassType* _classPtr) {
		#ifndef NDEBUG
		for (size_t i = 0; i < listeners.size(); ++i) {
			if (listeners[i].classPtr == _classPtr && listeners[i].callback == _func) {
				DebugLog("Warning: Called Connect Twice! Second call ignored.");
				return;
			}
		}
		#endif
		listeners.push_back({ _classPtr, _func });
	}

	static void Disconnect(MemberCallback _func, ClassType* _classPtr) {
		for (size_t i = 0; i < listeners.size(); ++i) {
			if (listeners[i].classPtr == _classPtr && listeners[i].callback == _func) {
				listeners[i].classPtr = nullptr;
				listeners[i].callback = nullptr;
			}
		}
	}
};

template <typename EventType>
struct Message {
	using GenericCallback = void(*)(EventType&);

	template <class ClassType>
	using MemberCallback = void(ClassType::*)(EventType&);

	static std::vector<GenericCallback> listeners;

	static void Send(EventType& _event) {

		for (auto it = listeners.begin(); it < listeners.end();) {
			if (*it == nullptr) {
				listeners.erase(it);
				if (listeners.size() == 0) {
					break;
				}
			}
			else {
				it++;
			}
		}
		size_t numListeners = listeners.size();
		for (size_t i = 0; i < numListeners; ++i) {
			listeners[i](_event);
		}
	}

	static void Connect(GenericCallback _func) {
		#ifndef NDEBUG
		for (size_t i = 0; i < listeners.size(); ++i) {
			if (listeners[i] == _func) {
				DebugLog("Warning: Called Connect Twice! Second call ignored.");
				return;
			}
		}
		#endif

		listeners.push_back(_func);
	}

	static void Disconnect(GenericCallback _func) {
		for (size_t i = 0; i < listeners.size(); ++i) {
			if (listeners[i] == _func) {
				listeners[i] = nullptr;
			}
		}
	}

	template<class ClassType>
	static void Connect(MemberCallback<ClassType> _func, ClassType* _classPtr) {
		static bool intialized = false;
		if (!intialized) {
			listeners.push_back(MemberDispatcher<EventType, ClassType>::Call);
			intialized = true;
		}
		MemberDispatcher<EventType, ClassType>::Connect(_func, _classPtr);
	}

	template<class ClassType>
	static void Disconnect(MemberCallback<ClassType> _func, ClassType* _classPtr) {
		MemberDispatcher<EventType, ClassType>::Disconnect(_func, _classPtr);
	}
};

template <typename EventType, class ClassType>
std::vector<MemberListener<EventType, ClassType>> MemberDispatcher<EventType, ClassType>::listeners;

template <typename EventType>
std::vector<void(*)(EventType&)> Message<EventType>::listeners;
