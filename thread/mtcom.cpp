#include "mtcom.h"

#include <iostream>

namespace tobilib
{
	std::atomic<bool>* Threadforum::signup()
	{
		if (threads.count(std::this_thread::get_id())>0)
			throw thread_error("Der Thread ist bereits Teil des Forums");
		threads.emplace(std::this_thread::get_id(),Thread());
		return mydata().ready.get();
	}
	
	void Threadforum::signout()
	{
		threads.erase(std::this_thread::get_id());
	}
	
	void Threadforum::call(const std::string& name)
	{
		for (auto& thr: threads)
		{
			if (thr.first == std::this_thread::get_id())
				continue;
			thr.second.eventqueue.push(name);
			*thr.second.ready = true;
		}
	}
	
	std::string Threadforum::nextev()
	{
		std::string out = mydata().eventqueue.front();
		mydata().eventqueue.pop();
		*mydata().ready = mydata().eventqueue.size()>0;
		return out;
	}
	
	Threadforum::Thread& Threadforum::mydata()
	{
		return threads.at(std::this_thread::get_id());
	}
	
	std::atomic<bool> TForum_access::dummyready (false);
	
	TForum_access::~TForum_access()
	{
		if (forum!=NULL)
		{
			forum->context.lock();
			forum->signout();
			forum->context.unlock();
		}
	}
	
	void TForum_access::enter(Threadforum& f)
	{
		if (forum != NULL)
			throw thread_error("Der TForum_access ist bereits verbunden");
		forum = &f;
		shutdown = false;
		my_id = std::this_thread::get_id();
		eventready = &dummyready;
		excex(std::bind(&TForum_access::on_enter,this));
		ioc.post(std::bind(&TForum_access::trydo,this));
	}
	
	void TForum_access::on_enter()
	{
		eventready = forum->signup();
	}
	
	void TForum_access::listen(const std::string& name, callback cb)
	{
		validate();
		handlers.emplace(name,cb);
	}
	
	void TForum_access::call(const std::string& name)
	{
		excex(std::bind(&Threadforum::call,forum,name));
	}
	
	void TForum_access::excex(callback cb)
	{
		if (shutdown)
			return;
		todo.push(cb);
	}
	
	void TForum_access::leave(callback cb)
	{
		validate();
		onexit = cb;
		excex(std::bind(&Threadforum::signout,forum));
		shutdown = true;
		eventready = &dummyready;
	}
	
	void TForum_access::trydo()
	{
		validate();
		check();
		if (todo.size()>0 && forum->context.try_lock())
		{
			todo.front()();
			todo.pop();
			forum->context.unlock();
		}
		if (shutdown && todo.size()==0)
		{
			forum = NULL;
			if (onexit)
				onexit();
			return;
		}
		ioc.post(std::bind(&TForum_access::trydo,this));
	}
	
	void TForum_access::check()
	{
		if (! *eventready)
			return;
		if (forum->context.try_lock())
		{
			while (*eventready)
			{
				std::string ev = forum->nextev();
				if (handlers.count(ev)>0)
					handlers.at(ev)();
			}
			*eventready = false;
			forum->context.unlock();
		}
	}
	
	void TForum_access::validate()
	{
		if (forum == NULL)
			throw thread_error("Der Zugriff ist nicht mit einem Threadforum verbunden");
		if (std::this_thread::get_id() != my_id)
			throw thread_error("Unerlaubter zugriff auf TForum_access");
	}
	
}