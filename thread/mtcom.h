/*
**************************************************************************
	Wichtiger Hinweis zur Anwendung:
**************************************************************************

	Die Lebzeiten von Threadforum und TForum_access müssen
	streng eingehalten werden:
	
	Threadforum: Muss mindestens solange wie alle entsprechenden
		TForum_access existieren
		
	TForum_access: zwischen enter() und leave() sollte TForum_access
		nicht zerstört werden. Sollte TForum_access trotzdem zerstört
		werden, wird der angegebene io_context fehlerhaft und muss
		zurückgesetzt werden.
	
**************************************************************************
*/

#ifndef MULTITHREADCOMMUNICATION_H
#define MULTITHREADCOMMUNICATION_H

#include <thread>
#include <atomic>
#include <mutex>
#include <map>
#include <queue>
#include <boost/asio.hpp>
#include <boost/bind.hpp>


namespace tobilib
{
	class thread_error: public std::exception
	{
	public:
		template<class strT> thread_error(strT m): msg(m) {};
		std::string msg;
		const char* what() const noexcept {return msg.c_str();};
	};
	
	class Threadforum
	{
	public:
		Threadforum() {};
		Threadforum(const Threadforum&) = delete;
		void operator= (const Threadforum&) = delete;
		
	private:
		friend class TForum_access;
		
		std::mutex context;
		struct Thread
		{
			std::unique_ptr<std::atomic<bool>> ready;
			std::queue<std::string> eventqueue;
			
			Thread(): ready(new std::atomic<bool>(false)) {};
		};
		std::map<std::thread::id, Thread> threads;
		
		std::atomic<bool>* signup();
		void signout();
		void call(const std::string&);
		std::string nextev();
		Thread& mydata();
	};
	
	class TForum_access
	{
	public:
		typedef std::function<void()> callback;
		
		void enter(Threadforum&);
		void listen(const std::string&, callback);
		void call(const std::string&);
		void excex(callback);
		void leave(callback);
		
		TForum_access(boost::asio::io_context& _ioc): ioc(_ioc) {};
		TForum_access(const TForum_access&) = delete;
		void operator= (const TForum_access&) = delete;
		~TForum_access();
		
	private:
		void trydo();
		void check();
		void validate();
		void on_enter();
		
		bool shutdown = false;
		callback onexit;
		Threadforum* forum = NULL;
		static std::atomic<bool> dummyready;
		std::atomic<bool>* eventready = &dummyready;
		std::thread::id my_id;
		boost::asio::io_context& ioc;
		std::map<std::string,callback> handlers;
		std::queue<callback> todo;
	};
}

#ifdef TC_AS_HPP
	#include "mtcom.cpp"
#endif

#endif