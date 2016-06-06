#include "Scene.hpp"


#include <chrono>
#include <iostream>
#include <deque>


std::unordered_map<uint32_t, Scene::NodeLevel> Scene::nodes_;


Scene& Scene::getInstance(void) {
    static Scene instance;
    return instance;
}
/*
void Scene::reserveNodes(unsigned nReservedNodes) {
    nodes_.reserve(nReservedNodes);
}
*/
NodeId Scene::addNode(void) {
    nTotalNodes = -1;
    auto& nl = nodes_[0];

    //  if no free invalidated nodes available, create a new one
    if (nl.firstFreeId == -1) {
        nl.nodes.emplace_back(nl.nodes.size());
        return nl.nodes.back().getId();
    }
    else {  //  else use first invalidated node
        Node& node = nl.nodes[nl.firstFreeId];
        node.valid_.reset(new bool(true));
        node.active_ = true;

        updateFirstFreeId(nl);
        return node.getId();
    }
}

NodeId Scene::addNode(const NodeId& parent) {
    printf("Adding child node for node %u,%llu\n", parent.level_, parent.id_);

    nTotalNodes = -1;
    auto& nl = nodes_[parent.level_+1];

    if (nl.firstFreeId == -1) {
        nl.nodes.emplace_back(nl.nodes.size(), parent.level_+1);

        auto newNodeId = nl.nodes.back().getId();
        (*parent).addChild(newNodeId);

        return newNodeId;
    }
    else {
        printf("Node %u,%llu revalidated\n", parent.level_+1, nl.firstFreeId);
        Node& node = nl.nodes[nl.firstFreeId];
        node.valid_.reset(new bool(true));
        node.active_ = true;

        auto newNodeId = node.getId();
        (*parent).addChild(newNodeId);

        updateFirstFreeId(nl);
        return newNodeId;
    }
}

void Scene::deleteNode(const NodeId& nodeId) {
    //  check if nodeId is valid
    if (!nodeId)
        return;

    nTotalNodes = -1;

    auto level = nodeId.level_;
    auto id = nodeId.id_;

    (*nodeId).invalidate();

    if (nodes_[level].firstFreeId == -1 || nodes_[level].firstFreeId > id) {
        nodes_[level].firstFreeId = id;
        printf("Level %u first free id: %llu\n", level, id);
    }

}

uint64_t Scene::getNodesNumber(int32_t level) {
    if (level == -1) {
        if (nTotalNodes == -1) {
            nTotalNodes = 0;
            for (auto& nl : nodes_) {
                for (auto& n : nl.second.nodes)
                    if (n.isValid())
                        nTotalNodes ++;
            }
        }
        return nTotalNodes;
    }
    else
        return nodes_[level].nodes.size();
}

void Scene::printNodes(void) {
    for (auto& node : nodes_[0].nodes)
        node.print();
}

Scene::Scene(void) :
    nTotalNodes(-1)
{}

void Scene::updateFirstFreeId(NodeLevel& nodeLevel) {
    if (nodeLevel.firstFreeId == -1)
        nodeLevel.firstFreeId = 0;

    for (auto i=nodeLevel.firstFreeId; i<(int64_t)nodeLevel.nodes.size(); ++i) {
        if (!*(nodeLevel.nodes[i].valid_)) {
            nodeLevel.firstFreeId = i;
            return;
        }
    }

    nodeLevel.firstFreeId = -1;
}

/*
void Scene::updateNodes(std::vector<Node>::iterator it) {
    //auto start = std::chrono::steady_clock::now();

    for (; it != nodes_.end(); ++it)
        *(it->it_) = it;

    //auto end = std::chrono::steady_clock::now();
    //auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    //std::cout << "node update: " << diff.count() << " microseconds" << std::endl;
}
*/
