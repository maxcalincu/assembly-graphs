#pragma once
#include <array>
#include <map>
#include <set>
#include <stdexcept>


template<typename SAG>
class TVertex;

template<typename SAG>
class TEdge;

template<typename SAG>
class TECyc;

//Vertex

template<typename SAG>
class TVertex {
    using Edge = TEdge<SAG>;
    friend SAG;
    friend Edge;

    size_t vertex_id, graph_id;
public:
    TVertex(size_t graph_id, size_t vertex_id);
    bool operator<(const TVertex& other) const;
    bool operator==(const TVertex& other) const;
    size_t GetGraphId() const;
    size_t GetVertexId() const;
};

template<typename SAG>
TVertex<SAG>::TVertex(size_t graph_id, size_t vertex_id): vertex_id(vertex_id), graph_id(graph_id) {};

template<typename SAG>
bool TVertex<SAG>::operator<(const TVertex& other) const {
    return graph_id == other.graph_id ? vertex_id < other.vertex_id : graph_id < other.graph_id;
}

template<typename SAG>
bool TVertex<SAG>::operator==(const TVertex& other) const {
    return other.graph_id == graph_id && other.vertex_id == vertex_id;
}

template<typename SAG>
size_t TVertex<SAG>::GetGraphId() const { return graph_id;}

template<typename SAG>
size_t TVertex<SAG>::GetVertexId() const { return vertex_id; }

//Edge

enum Polarity{Positive, Negative};
constexpr Polarity operator!(const Polarity& p) {
    return p == Positive ? Negative : Positive;
}

enum Tier{A, B, C, D, X};
constexpr Tier& operator++(Tier& t) {
    switch (t) {
        case A: return t = B;
        case B: return t = C;
        case C: return t = D;
        case D: return t = X;
        default: return t = X;
    }
}

template<typename SAG>
class TEdge {

private:
    using Vertex = TVertex<SAG>;
    using ECyc = TECyc<SAG>;
    friend SAG;
    friend ECyc;

    Vertex p_endpoint;
    Vertex n_endpoint;
    Tier tier;
    Polarity orientation;

public:
    TEdge(const Vertex& p_endpoint, const Vertex& n_endpoint, Tier tier, Polarity orientation);
    bool operator==(const TEdge& other) const;
    bool HasEndpoint(const Vertex& vertex) const;
    Tier GetTier() const;
    Vertex GetNegative() const;
    Vertex GetPositive() const;
    Vertex GetOther(const Vertex& vertex) const;
    Vertex GetHead() const;
    Vertex GetTail() const;
    void OrientTowards(const Vertex& head);
    Polarity GetOrientation() const;
    void SwitchOrientation();
    bool IsLoop() const;
    TEdge CreateCopy(const std::map<Vertex, Vertex>& bijection) const;
};

template<typename SAG>
TVertex<SAG> TEdge<SAG>::GetPositive() const { return p_endpoint; }

template<typename SAG>
TVertex<SAG> TEdge<SAG>::GetNegative() const { return n_endpoint; }

template<typename SAG>
Tier TEdge<SAG>::GetTier() const { return tier; }

template<typename SAG>
Polarity TEdge<SAG>::GetOrientation() const { return orientation; }

template<typename SAG>
TVertex<SAG> TEdge<SAG>::GetHead() const { return orientation == Positive ? p_endpoint : n_endpoint; }

template<typename SAG>
TVertex<SAG> TEdge<SAG>::GetTail() const { return orientation == Positive ? n_endpoint : p_endpoint; }

template<typename SAG>
bool TEdge<SAG>::HasEndpoint(const Vertex& vertex) const { return p_endpoint == vertex || n_endpoint == vertex; }

template<typename SAG>
void TEdge<SAG>::SwitchOrientation() { orientation = !orientation; }

template<typename SAG>
bool TEdge<SAG>::operator==(const TEdge<SAG>& other) const {
    return (tier == other.tier) && (p_endpoint == other.p_endpoint) && (n_endpoint == other.n_endpoint) && (orientation == other.orientation);
}

template<typename SAG>
TEdge<SAG>::TEdge(const Vertex& p_endpoint, const Vertex& n_endpoint, Tier tier, Polarity orientation): 
p_endpoint(p_endpoint), n_endpoint(n_endpoint), tier(tier), orientation(orientation) {
    if (p_endpoint.GetGraphId() != n_endpoint.GetGraphId()) {
        throw std::runtime_error("TEdge constructor: vertices have different graph_id");
    }
};

template<typename SAG>
TVertex<SAG> TEdge<SAG>::GetOther(const Vertex& vertex) const {
    if (vertex == p_endpoint) { return n_endpoint; }
    if (vertex == n_endpoint) { return p_endpoint; }
    throw std::runtime_error("GetOther: no such endpoint");
}

template<typename SAG>
bool TEdge<SAG>::IsLoop() const { return p_endpoint == n_endpoint; }

template<typename SAG>
void TEdge<SAG>::OrientTowards(const Vertex& head) {
    if (IsLoop()) {
        throw std::runtime_error("OrientTowards: loop passed");
    }
    if (head == GetHead()) {
        return;
    }
    if (head == GetTail()) {
        SwitchOrientation();
        return;
    }
    throw std::runtime_error("OrientTowards: no such endpoint");
}

template<typename SAG>
TEdge<SAG> TEdge<SAG>::CreateCopy(const std::map<Vertex, Vertex>& bijection) const {
    return TEdge<SAG>(bijection.at(p_endpoint), bijection.at(n_endpoint), tier, orientation);
}

//ECyc

template<typename SAG>
class TECyc {
    using Vertex = TVertex<SAG>;
    using Edge = TEdge<SAG>;
    friend SAG;

    Vertex central_vertex;
    std::array<Edge, 4> incident_edges;

    bool GetIndex(const Edge& incident_edge, size_t& index) const;
public:
    void ReplaceEdge(const Edge& before, const Edge& after);
    TECyc(const Vertex& central_vertex, const std::pair<Edge, Edge>& transversal_a, const std::pair<Edge, Edge>&  transversal_b);
    TECyc(const Vertex& central_vertex, const std::array<Edge, 4> incident_edges);

    bool operator==(const TECyc& other) const;
    bool HasEdge(const Edge& edge) const;
    Edge GetTransversal(const Edge& edge) const;
    Edge GetAnyEdge() const;
    std::pair<Edge, Edge> GetNeighbours(const Edge& edge) const;
    Vertex GetCentralVertex() const;
    Tier MEXTier(const Vertex& incident_vertex) const;
    TECyc CreateCopy(const std::map<Vertex, Vertex>& bijection) const;
};

template<typename SAG>
bool TECyc<SAG>::operator==(const TECyc& other) const {
    size_t index_0;
    if (central_vertex != other.central_vertex || !GetIndex(other.incident_edges[0], index_0)) {
        return false;
    }
    return (other.incident_edges[2] == incident_edges[index_0 ^ 2]) && (
        (other.incident_edges[1] == incident_edges[index_0 ^ 1] &&
        other.incident_edges[3] == incident_edges[index_0 ^ 3]) ||
        (other.incident_edges[1] == incident_edges[index_0 ^ 3] &&
        other.incident_edges[3] == incident_edges[index_0 ^ 1])
    );
}

template<typename SAG>
TVertex<SAG> TECyc<SAG>::GetCentralVertex() const {
    return central_vertex;
}

template<typename SAG>
bool TECyc<SAG>::HasEdge(const Edge& incident_edge) const {
    size_t index;
    return GetIndex(incident_edge,index);
}

template<typename SAG>
TEdge<SAG> TECyc<SAG>::GetTransversal(const Edge& incident_edge) const {
    size_t index;
    if (!GetIndex(incident_edge, index)) {
        throw std::runtime_error("GetTransversal: edge does not exist");
    }
    return incident_edges[index ^ 2];
}

template<typename SAG>
std::pair<TEdge<SAG>, TEdge<SAG>> TECyc<SAG>::GetNeighbours(const Edge& incident_edge) const {
    size_t index;
    if (!GetIndex(incident_edge, index)) {
        throw std::runtime_error("GetNeighbours: edge does not exist");
    }
    return std::make_pair(incident_edges[index ^ 1], incident_edges[index ^ 3]);
}

template<typename SAG>
TEdge<SAG> TECyc<SAG>::GetAnyEdge() const {
    return incident_edges[0];
}

template<typename SAG>
TECyc<SAG>::TECyc(const Vertex& central_vertex, const std::pair<Edge, Edge>& transversal_a, const std::pair<Edge, Edge>&  transversal_b):
    TECyc(central_vertex, {transversal_a.first, transversal_b.first, transversal_a.second, transversal_b.second}) {}

template<typename SAG>
TECyc<SAG>::TECyc(const Vertex& central_vertex_, const std::array<Edge, 4> incident_edges_): central_vertex(central_vertex_), incident_edges(incident_edges_) {
    if (incident_edges[0].GetHead() != central_vertex ||
        incident_edges[1].GetHead() != central_vertex ||
        incident_edges[2].GetHead() != central_vertex ||
        incident_edges[3].GetHead() != central_vertex) {
            throw std::runtime_error("TECyc construct: some vertices do not point to the central_vertex");
    }
    if (incident_edges[0] == incident_edges[1] ||
        incident_edges[0] == incident_edges[2] ||
        incident_edges[0] == incident_edges[3] ||
        incident_edges[1] == incident_edges[2] ||
        incident_edges[1] == incident_edges[3] ||
        incident_edges[2] == incident_edges[3]
    ) {
        throw std::runtime_error("TECyc construct: equal edges with same tiers passed");
    }
    if ((incident_edges[0].GetTail() == incident_edges[1].GetTail() && incident_edges[0].GetTier() == incident_edges[1].GetTier() && !incident_edges[0].IsLoop()) ||
        (incident_edges[0].GetTail() == incident_edges[2].GetTail() && incident_edges[0].GetTier() == incident_edges[2].GetTier() && !incident_edges[0].IsLoop()) ||
        (incident_edges[0].GetTail() == incident_edges[3].GetTail() && incident_edges[0].GetTier() == incident_edges[3].GetTier() && !incident_edges[0].IsLoop()) ||
        (incident_edges[1].GetTail() == incident_edges[2].GetTail() && incident_edges[1].GetTier() == incident_edges[2].GetTier() && !incident_edges[1].IsLoop()) ||
        (incident_edges[1].GetTail() == incident_edges[3].GetTail() && incident_edges[1].GetTier() == incident_edges[3].GetTier() && !incident_edges[1].IsLoop()) ||
        (incident_edges[2].GetTail() == incident_edges[3].GetTail() && incident_edges[2].GetTier() == incident_edges[3].GetTier() && !incident_edges[2].IsLoop())
    ) {
        throw std::runtime_error("TECyc construct: equal edges with same tiers passed");
    }
};

template<typename SAG>
Tier TECyc<SAG>::MEXTier(const Vertex& incident_vertex) const {
    std::set<Tier> existing_tiers;
    for (auto& edge : incident_edges) {
        if (edge.GetOther(central_vertex) == incident_vertex) {
            existing_tiers.insert(edge.tier);
        }
    }
    if (!existing_tiers.contains(Tier::A)) { return Tier::A; }
    if (!existing_tiers.contains(Tier::B)) { return Tier::B; }
    if (!existing_tiers.contains(Tier::C)) { return Tier::C; }
    if (!existing_tiers.contains(Tier::D)) { return Tier::D; }
    return Tier::X;
}


template<typename SAG>
void TECyc<SAG>::ReplaceEdge(const Edge& before, const Edge& after) {
    if (MEXTier(after.GetOther(central_vertex)) != after.tier) {
        throw std::runtime_error("ReplaceEdge: after is invalid");
    }
    size_t index;
    if (!GetIndex(before, index)) {
        throw std::runtime_error("ReplaceEdge: before edge does not exist");
    }
    incident_edges[index] = after;
}

template<typename SAG>
bool TECyc<SAG>::GetIndex(const Edge& incident_edge, size_t& index) const {
    if (incident_edge == incident_edges[0]) { index = 0; return true; }
    if (incident_edge == incident_edges[1]) { index = 1; return true; }
    if (incident_edge == incident_edges[2]) { index = 2; return true; }
    if (incident_edge == incident_edges[3]) { index = 3; return true; }
    return false;
}

template<typename SAG>
TECyc<SAG> TECyc<SAG>::CreateCopy(const std::map<Vertex, Vertex>& bijection) const{
    return TECyc<SAG>(bijection.at(central_vertex), {
        incident_edges[0].CreateCopy(bijection), 
        incident_edges[1].CreateCopy(bijection),
        incident_edges[2].CreateCopy(bijection),
        incident_edges[3].CreateCopy(bijection),
    });
}
