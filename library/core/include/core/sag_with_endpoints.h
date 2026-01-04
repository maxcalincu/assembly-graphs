#pragma once

#include <core/interfaces.h>

class SAGWithEndpoints: public ISimpleAssemblyGraph<SAGWithEndpoints>, public ITwoWord<SAGWithEndpoints> {
public:
    ECyc GetECyc(const Vertex& vertex) const override;
    size_t GetSize() const override;
    Edge GetStartEdge() const;
    Edge GetLastEdge() const;
    Polarity GetOrientation() const;
    std::vector<size_t> ConvertToVector() const override;
    
    bool operator==(const SAGWithEndpoints& other) const override;
    bool IsIsomorphic(const SAGWithEndpoints& other, bool reverse = false) const;
    bool IsEndpoint(const Vertex& vertex) const;
    bool HasVertex(const Vertex& vertex) const override;

    void Reverse();
    std::pair<Edge, Edge> RemoveVertex(const Vertex& vertex) override;
    Vertex InsertVertex(const Edge& edge_a, const Edge& edge_b) override;
    std::pair<Edge, Edge> InsertGraph(const Edge& edge, const SAGWithEndpoints& graph) override;
    SAGWithEndpoints& Saturate(const SAGWithEndpoints subgraph);
    SAGWithEndpoints& InteriorSaturate(const SAGWithEndpoints subgraph);

    SAGWithEndpoints(size_t graph_id);
    SAGWithEndpoints(size_t graph_id, const std::vector<size_t>& two_word);

    Edge GetKthEdge(size_t k) const;
    Edge TransversalAdvance(const Edge& edge) const;

private:
    Tier MEXTier(const Vertex& central_vertex, const Vertex& incident_vertex) const;
    Edge CreateEdge(const Vertex& vertex, const Vertex& other_vertex, Edge original_edge, Tier tier, Polarity orientation);
    Vertex CreateVertex();

    std::map<Vertex, ECyc> cyclic_order;
    size_t graph_id;
    size_t vertex_counter;

    Edge StartEdge;
    Edge LastEdge;
public:
    friend std::ostream& operator<<(std::ostream& os, const SAGWithEndpoints& graph) {
        auto vector = graph.ConvertToVector();
        os << "SAGWithEndpoints(" << graph.graph_id << ", {";
        for (size_t i = 0; i < vector.size(); ++i) {
            os << vector[i] << (i + 1 == vector.size() ? "" : " ");
        }
        os << "})";
        return os;
    }
};

SAGWithEndpoints operator*(int lhs, const SAGWithEndpoints& graph);

SAGWithEndpoints operator+(const SAGWithEndpoints& lhs, const SAGWithEndpoints& rhs);
