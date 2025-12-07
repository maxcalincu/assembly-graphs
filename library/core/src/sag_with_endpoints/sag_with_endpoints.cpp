#include <array>

#include <core/sag_with_endpoints.h>

size_t SAGWithEndpoints::GetSize() const {
    return cyclic_order.size();
}

SAGWithEndpoints::SAGWithEndpoints(size_t graph_id): 
    graph_id(graph_id), vertex_counter(2),
    StartEdge(Vertex(graph_id, 0), Vertex(graph_id, 1), Tier::A, Positive),
    LastEdge(Vertex(graph_id, 0), Vertex(graph_id, 1), Tier::A, Negative) {}

TEdge<SAGWithEndpoints> SAGWithEndpoints::GetStartEdge() const {
    return StartEdge;
}

TEdge<SAGWithEndpoints> SAGWithEndpoints::GetLastEdge() const {
    return LastEdge;
}

bool SAGWithEndpoints::HasVertex(const Vertex& vertex) const {
    return cyclic_order.contains(vertex) || IsEndpoint(vertex);
}

TECyc<SAGWithEndpoints> SAGWithEndpoints::GetECyc(const Vertex& vertex) const {
    if (IsEndpoint(vertex)) {
        throw std::runtime_error("GetECyc: passed one of the endpoints");
    }
    return cyclic_order.at(vertex);
};

TEdge<SAGWithEndpoints> SAGWithEndpoints::TransversalAdvance(const Edge& edge) const {
    auto new_edge = GetECyc(edge.GetHead()).GetTransversal(edge);
    new_edge.SwitchOrientation();
    return new_edge;
}

bool SAGWithEndpoints::operator==(const SAGWithEndpoints& other) const {
    return IsIsomorphic(other) || IsIsomorphic(other, true);
}

void SAGWithEndpoints::Reverse() {
    std::swap(StartEdge, LastEdge);
}

bool SAGWithEndpoints::IsIsomorphic(const SAGWithEndpoints& other, bool reverse) const {
    if (GetSize() != other.GetSize()) {
        return false;
    }
    Edge other_edge = other.GetStartEdge(), this_edge = !reverse ?  GetStartEdge() : GetLastEdge();
    this_edge.SwitchOrientation();
    other_edge.SwitchOrientation();

    std::map<Vertex, Vertex> bijection;
    while (other_edge != other.GetLastEdge()) {
        if (!bijection.contains(this_edge.GetHead())) {
            bijection.emplace(this_edge.GetHead(), other_edge.GetHead());
        }
        if (bijection.at(this_edge.GetHead()) != other_edge.GetHead()) {
            return false;
        }
        this_edge = TransversalAdvance(this_edge);
        other_edge = other.TransversalAdvance(other_edge);
    }
    return true;
}

Tier SAGWithEndpoints::MEXTier(const Vertex& central_vertex, const Vertex& incident_vertex) const {
    if (central_vertex == StartEdge.GetHead()) {
        return (StartEdge.GetTail() == incident_vertex && StartEdge.GetTier() == Tier::A) ? Tier::B : Tier::A;
    }
    if (central_vertex == LastEdge.GetHead()) {
        return (LastEdge.GetTail() == incident_vertex && LastEdge.GetTier() == Tier::A) ? Tier::B : Tier::A;
    }
    return GetECyc(central_vertex).MEXTier(incident_vertex);
}

bool SAGWithEndpoints::IsEndpoint(const Vertex& vertex) const {
    return vertex == StartEdge.GetHead() || vertex == LastEdge.GetHead();
}

TVertex<SAGWithEndpoints> SAGWithEndpoints::CreateVertex() {
    return Vertex(graph_id, vertex_counter++);
}

TVertex<SAGWithEndpoints> SAGWithEndpoints::InsertVertex(const Edge& edge_a_, const Edge& edge_b_) {
    Edge edge_a = edge_a_, edge_b = edge_b_;
    Vertex new_vertex = CreateVertex();
    Vertex x = edge_a.p_endpoint, y = edge_a.n_endpoint, z = edge_b.p_endpoint, w = edge_b.n_endpoint;
    edge_a.OrientTowards(Positive);
    Edge xo = CreateEdge(x, new_vertex, edge_a);
    edge_a.OrientTowards(Negative);
    Edge oy = CreateEdge(y, new_vertex, edge_a);
    auto opposite_edge_b = edge_b;
    opposite_edge_b.SwitchOrientation();
    if (edge_a == edge_b || edge_a == opposite_edge_b) {
        Edge n_oo(new_vertex, new_vertex, Tier::A, Negative);
        Edge p_oo(new_vertex, new_vertex, Tier::A, Positive);
        cyclic_order.emplace(new_vertex, ECyc(new_vertex, std::make_pair(xo, p_oo), std::make_pair(n_oo, oy)));
    } else {
        edge_b.OrientTowards(Positive);
        Edge zo = CreateEdge(z, new_vertex, edge_b);
        edge_b.OrientTowards(Negative);
        Edge ow = CreateEdge(w, new_vertex, edge_b);
        cyclic_order.emplace(new_vertex, ECyc(new_vertex, std::make_pair(xo, oy), std::make_pair(zo, ow)));
    }
    return new_vertex;
}

std::pair<TEdge<SAGWithEndpoints>, TEdge<SAGWithEndpoints>> SAGWithEndpoints::RemoveVertex(const Vertex& vertex) {
    ECyc ecyc = GetECyc(vertex);
    Edge x_edge = ecyc.GetAnyEdge(), y_edge = ecyc.GetTransversal(x_edge);
    auto [z_edge, w_edge] = ecyc.GetNeighbours(x_edge);
    std::pair<Edge, Edge> result(x_edge, y_edge);

    auto check_loop = [&](const Vertex& central_vertex, const Edge& neighbour_a, const Edge& neighbour_b) {
        auto opposite_neighbour_b = neighbour_b;
        opposite_neighbour_b.SwitchOrientation(); 
        if (neighbour_a != opposite_neighbour_b) {
            return false;
        }
        Edge a = ecyc.GetTransversal(neighbour_a), b = ecyc.GetTransversal(neighbour_b);

        auto ab = CreateEdge(a.GetOther(central_vertex), b.GetOther(central_vertex), a);
        if (!ab.IsLoop()) {
            CreateEdge(b.GetOther(central_vertex), a.GetOther(central_vertex), b);
        } else {
            b.OrientTowards(b.GetOther(central_vertex));
            cyclic_order.at(b.GetOther(central_vertex)).ReplaceEdge(b, ab);
        }
        result = std::make_pair(ab, ab);
        return true;
    };
    if (check_loop(vertex, x_edge, z_edge) ||
        check_loop(vertex, z_edge, y_edge) ||
        check_loop(vertex, y_edge, w_edge) ||
        check_loop(vertex, w_edge, x_edge)) {
        cyclic_order.erase(vertex);
        return result;
    }
    Edge xy = CreateEdge(x_edge.GetOther(vertex), y_edge.GetOther(vertex), x_edge);
    if (!xy.IsLoop()) {
        CreateEdge(y_edge.GetOther(vertex), x_edge.GetOther(vertex), y_edge);
    } else {
        y_edge.OrientTowards(y_edge.GetOther(vertex));
        cyclic_order.at(y_edge.GetOther(vertex)).ReplaceEdge(y_edge, xy);
    }
    Edge zw = CreateEdge(z_edge.GetOther(vertex), w_edge.GetOther(vertex), z_edge);
    if (!zw.IsLoop()) {
        CreateEdge(w_edge.GetOther(vertex), z_edge.GetOther(vertex), w_edge);
    } else {
        w_edge.OrientTowards(w_edge.GetOther(vertex));
        cyclic_order.at(w_edge.GetOther(vertex)).ReplaceEdge(w_edge, zw);
    }
    cyclic_order.erase(vertex);
    return std::make_pair(xy, zw);
}

TEdge<SAGWithEndpoints> SAGWithEndpoints::CreateEdge(const Vertex& vertex, const Vertex& other_vertex, Edge original_edge) {
    if (!original_edge.IsLoop()) {
        original_edge.OrientTowards(vertex);
    }
    Tier tier = MEXTier(vertex, other_vertex);
    Edge new_edge = (vertex < other_vertex) ? Edge(vertex, other_vertex, tier, Positive)
                                            : Edge(other_vertex, vertex, tier, Negative);
    if (IsEndpoint(vertex)) {
        (vertex == StartEdge.GetHead() ? StartEdge : LastEdge) = new_edge; 
    } else {
        cyclic_order.at(vertex).ReplaceEdge(original_edge, new_edge);
    }
    new_edge.SwitchOrientation();
    return new_edge;
}

void SAGWithEndpoints::InsertGraph(const Edge& edge, const SAGWithEndpoints& other) {
    if (other.GetSize() == 0) {
        return;
    }
    std::map<Vertex, Vertex> bijection;
    bijection.emplace(other.StartEdge.GetHead(), CreateVertex());
    bijection.emplace(other.LastEdge.GetHead(), CreateVertex());
    for (auto& [vertex, ecyc] : other.cyclic_order) {
        bijection.emplace(vertex, CreateVertex());
    }
    for (auto& [vertex, ecyc] : other.cyclic_order) {
        cyclic_order.emplace(bijection.at(vertex), other.GetECyc(vertex).CreateCopy(bijection));
    }
    Edge xy = edge;
    Vertex x = xy.GetHead(), y = xy.GetTail();
    Edge other_start = other.StartEdge.CreateCopy(bijection), other_last = other.LastEdge.CreateCopy(bijection);
    other_start.SwitchOrientation();
    other_last.SwitchOrientation();

    CreateEdge(x, other_start.GetHead(), xy);
    CreateEdge(other_start.GetHead(), x, other_start);
    xy.SwitchOrientation();
    CreateEdge(y, other_last.GetHead(), xy);
    CreateEdge(other_last.GetHead(), y, other_last);
}

std::vector<size_t> SAGWithEndpoints::ConvertToVector() const {
    std::map<Vertex, size_t> bijection;
    Edge edge = StartEdge;
    edge.SwitchOrientation();
    size_t counter = 0;
    std::vector<size_t> result;
    result.reserve(2 * GetSize());
    for (; edge != LastEdge; edge = TransversalAdvance(edge)) {
        if (!bijection.contains(edge.GetHead())) {
            bijection.emplace(edge.GetHead(), ++counter);
        }
        result.push_back(bijection.at(edge.GetHead()));
    }
    return result;
}

SAGWithEndpoints::SAGWithEndpoints(size_t graph_id, const std::vector<size_t>& two_word): SAGWithEndpoints(graph_id) {
    if (two_word.empty()) {
        return;
    }
    std::map<size_t, Vertex> bijection;
    for (auto letter : two_word) {
        if (!bijection.contains(letter)) {
            bijection.emplace(letter, CreateVertex());
        }
    }
    if (2 * bijection.size() != two_word.size()) {
        throw std::runtime_error("SAGWithEndpoints constructor: non-valid two-word");
    }
    size_t n = two_word.size()/2;
    std::map<Vertex, std::pair<Edge, Edge>> transversal_a, transversal_b;
    std::map<std::pair<Vertex, Vertex>, Tier> tiers;
    auto func = [&](const Vertex& x, const Vertex& o, const Vertex& y) {
        auto pair_xo = x < o ? std::make_pair(x, o) : std::make_pair(o, x);
        auto pair_oy = o < y ? std::make_pair(o, y) : std::make_pair(y, o);
        auto tier_xo = tiers.at(pair_xo);
        if (!tiers.contains(pair_oy)) {
            tiers.emplace(pair_oy, Tier::A);
        } else {
            ++tiers.at(pair_oy);
        }
        auto tier_oy = tiers.at(pair_oy);
        auto edge_xo = Edge(x, o, tier_xo, Negative);
        auto edge_oy = Edge(o, y, tier_oy, Positive);
        if (transversal_b.contains(o)) {
            throw std::runtime_error("SAGWithEndpoints constructor: non-valid two-word");
        }
        (transversal_a.contains(o) ? transversal_b : transversal_a).emplace( 
            o, std::make_pair(edge_xo, edge_oy)
        );
    };
    tiers.emplace(std::make_pair(StartEdge.GetHead(), bijection.at(two_word[0])), Tier::A);
    func(StartEdge.GetHead(), bijection.at(two_word[0]), bijection.at(two_word[1]));
    for (size_t i = 1; i < 2 * n - 1; ++i) {
        func(bijection.at(two_word[i - 1]), bijection.at(two_word[i]), bijection.at(two_word[i + 1]));
    }
    func(bijection.at(two_word[2 * n - 2]), bijection.at(two_word[2 * n - 1]), LastEdge.GetHead());

    StartEdge = transversal_a.at(bijection.at(two_word[0])).first;
    StartEdge.SwitchOrientation();
    
    LastEdge = transversal_b.at(bijection.at(two_word[2 * n - 1])).second;
    LastEdge.SwitchOrientation();

    for (auto [_, vertex] : bijection) {
        if (!transversal_a.contains(vertex) || !transversal_b.contains(vertex)) {
            throw std::runtime_error("SAGWithEndpoints constructor: non-valid two-word");
        }
        cyclic_order.emplace(vertex, ECyc(vertex, transversal_a.at(vertex), transversal_b.at(vertex)));
    }
}

TEdge<SAGWithEndpoints> SAGWithEndpoints::GetKthEdge(size_t k) const {
    Edge edge = GetStartEdge();
    edge.SwitchOrientation();
    for (size_t i = 0; i < std::min(k, 2 * GetSize()); ++i) {
        edge = TransversalAdvance(edge);
    }
    return edge;
}
