#pragma once
#include <vector>

#include <core/basics.h>

template<typename Impl>
class ISimpleAssemblyGraph;

template<typename SAG>
concept SimpleAssemblyGraphImpl = requires {
    requires std::derived_from<SAG, ISimpleAssemblyGraph<SAG>>;
    typename SAG::Vertex;
    typename SAG::Edge;
};

template<typename SAG>
class ISimpleAssemblyGraph {
public:
    using Vertex = TVertex<SAG>; 
    using Edge = TEdge<SAG>;
    using ECyc = TECyc<SAG>;

    virtual ~ISimpleAssemblyGraph() = default;
    virtual bool operator==(const SAG& other) const = 0;
    virtual std::pair<Edge, Edge> RemoveVertex(const Vertex& vertex) = 0;
    virtual Vertex InsertVertex(const Edge& edge_a, const Edge& edge_b) = 0;
    virtual void InsertGraph(const Edge& edge, const SAG& graph) = 0;
    virtual ECyc GetECyc(const Vertex& vertex) const = 0;
    virtual std::size_t GetSize() const = 0;
    virtual bool HasVertex(const Vertex& vertex) const = 0;
};

template<typename TW>
class ITwoWord {
public:
    virtual ~ITwoWord() = default;
    virtual bool operator==(const TW& other) const = 0;
    virtual std::size_t GetSize() const = 0;
    virtual std::vector<size_t> ConvertToVector() const = 0;
};
