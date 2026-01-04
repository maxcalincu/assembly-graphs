#include <array>

#include <core/sag_with_endpoints.h>
#include <stdexcept>
#include <utility>

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

Polarity SAGWithEndpoints::GetOrientation() const {
    return LastEdge.GetOrientation();
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

TVertex<SAGWithEndpoints> SAGWithEndpoints::InsertVertex(const Edge& edge_a, const Edge& edge_b) {
    Edge xy = edge_a, zw = edge_b;
    Vertex o = CreateVertex();
    
    xy.OrientTowards(GetOrientation());
    zw.OrientTowards(GetOrientation());

    Vertex x = xy.GetTail(), y = xy.GetHead(), z = zw.GetTail(), w = zw.GetHead();

    Edge oy = CreateEdge(y, o, xy, MEXTier(y, o), GetOrientation());
    auto yx = xy; yx.SwitchOrientation();
    Edge xo = CreateEdge(x, o, yx, MEXTier(x, o), !GetOrientation());
    if (xy == zw) {
        Edge in_oo(o, o, Tier::A, !GetOrientation());
        Edge out_oo(o, o, Tier::A, GetOrientation());
        cyclic_order.emplace(o, ECyc(o, std::make_pair(xo, in_oo), std::make_pair(out_oo, oy)));
    } else {
        Edge ow = CreateEdge(w, o, zw, MEXTier(w, o), GetOrientation());
        Edge wz = zw; wz.SwitchOrientation();
        Edge zo = CreateEdge(z, o, wz, MEXTier(z, o), !GetOrientation());
        cyclic_order.emplace(o, ECyc(o, std::make_pair(xo, oy), std::make_pair(zo, ow)));
    }
    return o;
}

std::pair<TEdge<SAGWithEndpoints>, TEdge<SAGWithEndpoints>> SAGWithEndpoints::RemoveVertex(const Vertex& vertex) {
    ECyc ecyc = GetECyc(vertex);
    Edge x_edge = ecyc.GetAnyEdge(), y_edge = ecyc.GetTransversal(x_edge);
    auto [z_edge, w_edge] = ecyc.GetNeighbours(x_edge);

    if (x_edge.GetOrientation() != GetOrientation()) {
        std::swap(x_edge, y_edge);
    }
    if (z_edge.GetOrientation() != GetOrientation()) {
        std::swap(z_edge, w_edge);
    }

    std::pair<Edge, Edge> result(x_edge, x_edge);

    auto check_loop = [&](const Vertex& central_vertex, const Edge& neighbour_a, const Edge& neighbour_b) {
        auto opposite_neighbour_b = neighbour_b;
        opposite_neighbour_b.SwitchOrientation(); 
        if (neighbour_a != opposite_neighbour_b) {
            return false;
        }
        Edge out_edge = ecyc.GetTransversal(neighbour_a), in_edge = ecyc.GetTransversal(neighbour_b);
        out_edge.SwitchOrientation(); in_edge.SwitchOrientation();

        auto in_out_tier = MEXTier(in_edge.GetHead(), out_edge.GetHead());
        auto in_out = CreateEdge(
            in_edge.GetHead(), 
            out_edge.GetHead(), 
            in_edge,
            in_out_tier,
            !GetOrientation()
        );
        CreateEdge(
            out_edge.GetHead(), 
            in_edge.GetHead(), 
            out_edge,
            in_out_tier,
            GetOrientation()
        );
        result = std::make_pair(in_out, in_out);
        return true;
    };
    if (check_loop(vertex, z_edge, y_edge) ||
        check_loop(vertex, x_edge, w_edge)) {
        cyclic_order.erase(vertex);
        return result;
    }
    x_edge.SwitchOrientation(); y_edge.SwitchOrientation();
    auto xy_tier = MEXTier(x_edge.GetHead(), y_edge.GetHead());
    Edge xy = CreateEdge(
        x_edge.GetHead(), 
        y_edge.GetHead(), 
        x_edge, 
        xy_tier,
        !GetOrientation()
    );
    CreateEdge(
        y_edge.GetHead(),
        x_edge.GetHead(),
        y_edge,
        xy_tier,
        GetOrientation()
    );
    z_edge.SwitchOrientation(), w_edge.SwitchOrientation();
    auto zw_tier = MEXTier(z_edge.GetHead(), w_edge.GetHead());
    Edge zw = CreateEdge(
        z_edge.GetHead(),
        w_edge.GetHead(),
        z_edge,
        zw_tier,
        !GetOrientation()
    );
    CreateEdge(
        w_edge.GetOther(vertex),
        z_edge.GetOther(vertex),
        w_edge,
        zw_tier,
        GetOrientation()
    );
    cyclic_order.erase(vertex);
    return std::make_pair(xy, zw);
}

TEdge<SAGWithEndpoints> SAGWithEndpoints::CreateEdge(const Vertex& vertex, const Vertex& other_vertex, Edge original_edge, Tier tier, Polarity orientation) {
    Edge new_edge = orientation == Positive ? Edge(vertex, other_vertex, tier, Positive)
                                            : Edge(other_vertex, vertex, tier, Negative);
    if (IsEndpoint(vertex)) {
        (vertex == StartEdge.GetHead() ? StartEdge : LastEdge) = new_edge; 
    } else {
        cyclic_order.at(vertex).ReplaceEdge(original_edge, new_edge);
    }
    new_edge.SwitchOrientation();
    return new_edge;
}

std::pair<TEdge<SAGWithEndpoints>, TEdge<SAGWithEndpoints>> SAGWithEndpoints::InsertGraph(const Edge& edge, const SAGWithEndpoints& other) {
    if (other.GetSize() == 0) {
        auto oriented_edge = edge; oriented_edge.OrientTowards(GetOrientation());
        return std::make_pair(oriented_edge, oriented_edge);
    }
    std::map<Vertex, Vertex> bijection;
    bijection.emplace(other.StartEdge.GetHead(), CreateVertex());
    bijection.emplace(other.LastEdge.GetHead(), CreateVertex());
    for (auto& [vertex, ecyc] : other.cyclic_order) {
        bijection.emplace(vertex, CreateVertex());
    }
    bool switch_polarity = (GetOrientation() != other.GetOrientation());
    for (auto& [vertex, ecyc] : other.cyclic_order) {
        cyclic_order.emplace(bijection.at(vertex), other.GetECyc(vertex).CreateCopy(bijection, switch_polarity));
    }
    Edge xy = edge; xy.OrientTowards(GetOrientation());
    Vertex x = xy.GetTail(), y = xy.GetHead();
    auto yx = xy; yx.SwitchOrientation();

    Edge other_start = other.StartEdge.CreateCopy(bijection, switch_polarity);
    Edge other_last = other.LastEdge.CreateCopy(bijection, switch_polarity);
    other_start.SwitchOrientation();
    other_last.SwitchOrientation();

    auto tier_x_start = MEXTier(x, other_start.GetHead());
    auto begin = CreateEdge(x, other_start.GetHead(), yx, tier_x_start, !GetOrientation());
    CreateEdge(other_start.GetHead(), x, other_start, tier_x_start, GetOrientation());

    auto tier_last_y = MEXTier(y, other_last.GetHead());    
    auto end = CreateEdge(y, other_last.GetHead(), xy, tier_last_y, GetOrientation());
    CreateEdge(other_last.GetHead(), y, other_last, tier_last_y, !GetOrientation());

    begin.OrientTowards(GetOrientation());
    end.OrientTowards(GetOrientation());
    return std::make_pair(begin, end);
}

std::vector<size_t> SAGWithEndpoints::ConvertToVector() const {
    std::map<Vertex, size_t> bijection;
    Edge edge = GetKthEdge(0);
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
    auto process = [&](const Vertex& x, const Vertex& o, const Vertex& y) {
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
    process(StartEdge.GetHead(), bijection.at(two_word[0]), bijection.at(two_word[1]));
    for (size_t i = 1; i < 2 * n - 1; ++i) {
        process(bijection.at(two_word[i - 1]), bijection.at(two_word[i]), bijection.at(two_word[i + 1]));
    }
    process(bijection.at(two_word[2 * n - 2]), bijection.at(two_word[2 * n - 1]), LastEdge.GetHead());

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

SAGWithEndpoints& SAGWithEndpoints::InteriorSaturate(const SAGWithEndpoints subgraph) {
    if (GetSize() == 0) {
        return *this;
    }
    for (auto edge = GetKthEdge(1); edge != GetLastEdge(); edge = TransversalAdvance(edge)) {
        edge = InsertGraph(edge, subgraph).second;
    }
    return *this;
}

SAGWithEndpoints& SAGWithEndpoints::Saturate(const SAGWithEndpoints subgraph) {
    if (GetSize() == 0) {
        InsertGraph(GetStartEdge(), subgraph);
        return *this;
    }
    InteriorSaturate(subgraph);
    InsertGraph(GetStartEdge(), subgraph);
    InsertGraph(GetLastEdge(), subgraph);
    return *this;
}

SAGWithEndpoints operator*(int lhs, const SAGWithEndpoints& graph) {
    auto result = SAGWithEndpoints(0, {});
    for (size_t i = 0; i < lhs; ++i) {
        result.InsertGraph(result.GetLastEdge(), graph);
    }
    return result;
}

SAGWithEndpoints operator+(const SAGWithEndpoints& lhs, const SAGWithEndpoints& rhs) {
    auto result = lhs; result.InsertGraph(result.GetLastEdge(), rhs);
    return result;
}
