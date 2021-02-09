#define TC_DATABASE_INTERN
#include "../database.h"
#include <iostream>
#include <cmath>

using namespace tobilib;
using namespace database_detail;

Database sourcedb;
Database targetdb;

void transfer_data(Member src, Member tgt)
{
    if (src.type.blockType != tgt.type.blockType)
        return;
    if (src.type.ptr_type != nullptr)
        if (src.type.ptr_type->name != tgt.type.ptr_type->name)
            return;

    int amount = std::min(src.type.amount,tgt.type.amount);
    if (amount>1)
    {
        for (int i=0;i<amount;i++)
            transfer_data(src[i],tgt[i]);
        return;
    }

    if (src.type.blockType == BlockType::t_bool)
        tgt.set(src.get<bool>());
    if (src.type.blockType == BlockType::t_char)
        tgt.set(src.get<char>());
    if (src.type.blockType == BlockType::t_int)
        tgt.set(src.get<int>());
    if (src.type.blockType == BlockType::t_double)
        tgt.set(src.get<double>());
    if (src.type.blockType == BlockType::t_ptr)
        tgt.set( Cluster(
            &targetdb,
            (ClusterFile*) &tgt.fs,
            src->index()
            ));
    if (src.type.blockType == BlockType::t_list)
        for (Member item: src)
            tgt.emplace().set( Cluster(
                &targetdb,
                (ClusterFile*) &tgt.fs,
                item->index()
            ));
    
}

void transfer_data(Cluster src, Cluster tgt)
{
    for (MemberType& memtype: src.cf->type.members)
    {
        if (!tgt.cf->type.contains(memtype.name))
            continue;
        transfer_data(
            src[memtype.name],
            tgt[memtype.name]
        );
    }
}

void transfer_data()
{
    for (ClusterFile& srcfile: sourcedb.clusters)
    {
        ClusterFile* tgtfile = targetdb.get_file(srcfile.type.name);
        if (tgtfile == nullptr)
            continue;
        unsigned int capacity = srcfile.get_data_capacity();
        for (unsigned int i=0;i<capacity;i++)
            transfer_data(
                Cluster(&sourcedb,&srcfile,i),
                Cluster(&targetdb,tgtfile,i)
            );
    }
}

void allocate_clusters(std::string name)
{
    ClusterFile* sourcefile = sourcedb.get_file(name);
    ClusterFile* targetfile = targetdb.get_file(name);
    if (targetfile==nullptr)
        return;

    unsigned int capacity = sourcefile->get_data_capacity();
    while (targetfile->get_data_capacity() < capacity)
        targetfile->extend_data_capacity();

    targetfile->set_data_capacity(capacity);
    targetfile->set_first_filled(sourcefile->get_first_filled());
    targetfile->set_last_filled(sourcefile->get_last_filled());
    targetfile->set_first_empty(sourcefile->get_first_empty());
    targetfile->set_last_empty(sourcefile->get_last_empty());

    for (unsigned int i=1;i<capacity;i++)
    {
        targetfile->set_occupied(i,sourcefile->get_occupied(i));
        targetfile->set_previous(i,sourcefile->get_previous(i));
        targetfile->set_next(i,sourcefile->get_next(i));
        targetfile->set_refcount_add(i,-targetfile->get_refcount(i));
        Cluster(&targetdb,targetfile,i).init_memory();
    }
}

void allocate_clusters()
{
    for (ClusterFile& cf: sourcedb.clusters)
        allocate_clusters(cf.type.name);
}

void transfer(std::string srcpath, std::string tarpath)
{
    sourcedb.log.prefix = "source database: ";
    targetdb.log.prefix = "target database: ";

    sourcedb.setPath(srcpath);
    if (!sourcedb.init() || !sourcedb.open())
        return;

    targetdb.setPath(tarpath);
    if (!targetdb.init() || !targetdb.open())
        return;


    std::cout << "allocate entries" << std::endl;
    targetdb.listfile.set_first_empty(0);
    targetdb.listfile.set_last_empty(0);
    targetdb.listfile.set_data_capacity(1);
    allocate_clusters();

    std::cout << "transfer data" << std::endl;
    transfer_data();

    std::cout << "finished" << std::endl;
}

int main(int argc, const char** args)
{
    if (argc != 3)
    {
        std::cout << "Erwarte Ursprung und Zielanweisung:\nbasetransfer [source] [target]" << std::endl;
        return 0;
    }
    transfer(args[1],args[2]);
}