#pragma once

#include <core/interfaces.h>

template<SimpleAssemblyGraphImpl SAG>
class SetOfSimplePaths {
    using Vertex = SAG::Vertex;
    using Edge = SAG::Edge;
    using ECyc = SAG::ECyc;
public:
    SetOfSimplePaths(const SAG& Graph);
    void AddEdge(Edge);
    void RemoveEdge(Edge);
    size_t getNumberOfPaths() const;
    size_t getNumberOfVertices() const;
private:
    // TBA
};