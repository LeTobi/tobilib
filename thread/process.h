#ifndef TC_PROCESS_H
#define TC_PROCESS_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <set>
#include "../general/callback.hpp"

namespace tobilib
{
    class Process_error: public std::exception
	{
	public:
		std::string message;
		template<class Text> Process_error(Text txt): message(txt) {}
		const char* what() const noexcept {return message.c_str();}
	};

    /**
     * Klasse zur Bündelung von asynchronen Operationen.
     * Vorteil: Kann als Member einer Klasse gelöscht werden, ohne ungültige Aufrufe zu produzieren.
     */
    class Process: public boost::asio::io_context
    {
    private:
        Process* parent = NULL;
        std::set<Process*> children;

        void registerstep();
        void step();

    public:
        /** Erstellt einen unabhängigen Prozess
         */
        Process();

        /** Erstellt einen Unterprozess
         * @param p Übergeordneter Prozess
         */
        Process(Process& p);

        /** Destructor: Kann auch auf aktive Prozesse angewendet werden.
         */
        ~Process();

        Process(const Process&) = delete;
        Process& operator=(const Process&) = delete;

        /** wird bei jedem Prozess-Schritt aufgerufen
         */
        Callback< > on_step;

        /** Macht den Prozess unabhängig
         */
        void detach();

        /** Prozess wird untergeordnet
         * @param p Übergeordneter Prozess
         * @exception Process_error Der Prozess ist bereits untergeordnet
         */
        void attach_to(Process& p);
    };
}

#ifdef TC_AS_HPP
    #include "process.cpp"
#endif

#endif