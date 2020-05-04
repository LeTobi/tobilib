#include "typeinfo.h"

using namespace tobilib;
using namespace database;

const BlockInfo* ClusterInfo::find(const std::string& name) const
{
    for (auto& block: segmentation)
        if (block.name==name)
            return &block;
    return nullptr;
}

BlockInfo* ClusterInfo::find(const std::string& name)
{
    for (auto& block: segmentation)
        if (block.name==name)
            return &block;
    return nullptr;
}

const ClusterInfo* DatabaseInfo::find(const std::string& name) const
{
    for (auto& cluster: types)
        if (cluster.name==name)
            return &cluster;
    return nullptr;
}

ClusterInfo* DatabaseInfo::find(const std::string& name)
{
    for (auto& cluster: types)
        if (cluster.name==name)
            return &cluster;
    return nullptr;
}