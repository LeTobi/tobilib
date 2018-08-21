#ifndef TC_CALLBACK_HPP
#define TC_CALLBACK_HPP

#include <memory>
#include <map>
#include <functional>

namespace tobilib
{
	class CBLink
	{
	public:
		std::function<void(unsigned int)> remove;
		std::function<bool(unsigned int)> is_active;
		bool valid = true;
	};
	
	class Callback_Ticket
	{
	private:
		unsigned int index;
		std::shared_ptr<CBLink> link;
		
	public:
		Callback_Ticket(const std::shared_ptr<CBLink>& l, unsigned int i): link(l), index(i) {};
		
		bool active() const
		{
			return link->valid && link->is_active(index);
		}
		
		void disable()
		{
			if (link->valid)
				link->remove(index);
		}
	};
	
	enum class callback_position{
		early,
		late
	};
	
	template <class... Args>
	class Callback
	{
	private:
		std::shared_ptr<CBLink> link;
		std::map<unsigned int,std::function<void(Args...)>> first;
		std::map<unsigned int,std::function<void(Args...)>> last;
		
		bool is_active(unsigned int i) const
		{
			return (first.count(i)+last.count(i))>0;
		};
		
		void remove(unsigned int i)
		{
			first.erase(i);
			last.erase(i);
		};
		
	public:
		
		Callback(): link(new CBLink())
		{
			link->is_active = std::bind(&Callback<Args...>::is_active,this,std::placeholders::_1);
			link->remove = std::bind(&Callback<Args...>::remove,this,std::placeholders::_1);
		};
		
		Callback_Ticket notify(std::function<void(Args...)> f, callback_position p=callback_position::early)
		{
			unsigned int i;
			while (is_active(i))
				i++;
			if (p==callback_position::early)
				first.insert(std::make_pair(i,f));
			else if (p==callback_position::late)
				last.insert(std::make_pair(i,f));
			return Callback_Ticket(link,i);
		};
		
		void operator()(Args... a) const
		{
			for (auto f=first.begin();f!=first.end();f++)
			{
				f->second(a...);
			}
			for (auto f=last.rbegin();f!=last.rend();f++)
			{
				f->second(a...);
			}
		};
		
		operator bool()
		{
			return (first.size()+last.size())>0;
		};
		
		~Callback()
		{
			link->valid = false;
		};
		
		Callback(const Callback<Args...>&) = delete;
		void operator=(const Callback<Args...>&) = delete;
	};
	
	
}

#endif