#include "process.h"

namespace tobilib
{
    void Process::registerstep()
    {
        boost::asio::post(*this,boost::bind(&Process::step,this));
    }

    void Process::step()
    {
        on_step();
        for (auto& proc: children)
        {
            proc->poll_one();
        }
        registerstep();
    }

    Process::Process()
    {
        registerstep();
    }

    Process::Process(Process& p)
    {
        registerstep();
        attach_to(p);
    }

    Process::~Process()
    {
        detach();
        for (auto& proc: children)
        {
            proc->detach();
        }
    }

    void Process::detach()
    {
        if (parent==NULL)
            return;
        parent->children.erase(this);
        parent = NULL;
    }

    void Process::attach_to(Process& p)
    {
        if (parent!=NULL)
            throw Process_error("Dieser Prozess ist bereits untergeordnet.");
        parent = &p;
        parent->children.insert(this);
    }
}