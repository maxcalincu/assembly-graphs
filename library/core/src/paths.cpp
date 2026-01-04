#include <core/paths.h>

size_t SetOfSimplePaths<SAGWithEndpoints>::GetNumberOfPaths() const {
    return (endpoint_edges.size()/2) + GetNumberOfDots();
}

size_t SetOfSimplePaths<SAGWithEndpoints>::GetNumberOfVertices() const {
    return covered_vertices.size();
}

size_t SetOfSimplePaths<SAGWithEndpoints>::GetNumberOfDots() const {
    return graph.GetSize() - GetNumberOfVertices();
}

SetOfSimplePaths<SAGWithEndpoints>::SetOfSimplePaths(const SAGWithEndpoints& graph): graph(graph) {};


size_t SetOfSimplePaths<SAGWithEndpoints>::GetNumberOfEdges() const {
    return next_edge.size()/2 + endpoint_edges.size()/2;
}

bool SetOfSimplePaths<SAGWithEndpoints>::IsPathEndpoint(const Vertex& vertex) const {
    return other_endpoint.contains(vertex);
}

bool SetOfSimplePaths<SAGWithEndpoints>::IsCovered(const Vertex& vertex) const {
    return covered_vertices.contains(vertex);
}

TEdge<SAGWithEndpoints> SetOfSimplePaths<SAGWithEndpoints>::GetEndpointEdge(const Vertex& vertex) const {
    if (!IsPathEndpoint(vertex)) {
        throw std::runtime_error("GetEndpointEdge: vertex isn't an endpoint");
    }
    return endpoint_edges.at(vertex);
}

TVertex<SAGWithEndpoints> SetOfSimplePaths<SAGWithEndpoints>::GetOtherEndpoint(const Vertex& vertex) const {
    if (!IsPathEndpoint(vertex)) {
        throw std::runtime_error("GetOtherEndpoint: vertex isn't an endpoint");
    }
    return other_endpoint.at(vertex);
}

TEdge<SAGWithEndpoints> SetOfSimplePaths<SAGWithEndpoints>::GetNextPathEdge(const Edge& edge) const {
    if (!next_edge.contains(edge)) {
        throw std::runtime_error("GetNextPathEdge: vertex isn't an endpoint");
    }
    auto next = next_edge.at(edge);
    next.SwitchOrientation();
    return next;
}

bool SetOfSimplePaths<SAGWithEndpoints>::Contains(const Edge& edge) const {
    auto xy = edge, yx = edge; yx.SwitchOrientation();
    auto x = xy.GetTail(), y = xy.GetHead();
    if (!IsPathEndpoint(x)) {
        return next_edge.contains(yx);
    }
    if (!IsPathEndpoint(y)) {
        return next_edge.contains(xy);
    }
    return (GetEndpointEdge(x) == yx) || (GetEndpointEdge(x) == xy);
} 

bool SetOfSimplePaths<SAGWithEndpoints>::IsInsertionValid(Edge xy) const {
    auto check = [&](const Vertex& x_, const Vertex y_){
        if (!IsPathEndpoint(x_)) {
            return IsCovered(x_);
        }
        auto edge_x = GetEndpointEdge(x_);
        ECyc ecyc_x = graph.GetECyc(x_);
        edge_x.OrientTowards(x_);
        xy.OrientTowards(x_);
        return  (   ecyc_x.GetTransversal(edge_x) == xy || 
                    !ecyc_x.HasEdge(xy) || 
                    GetOtherEndpoint(x_) == y_
                );
    };
    auto x = xy.GetTail(), y = xy.GetHead();
    return !(   xy.IsLoop() ||
                graph.IsEndpoint(x) ||
                graph.IsEndpoint(y) ||
                check(x, y) ||
                check(y, x)
            );
}

void SetOfSimplePaths<SAGWithEndpoints>::InsertEdge(Edge xy) {
    if (!IsInsertionValid(xy)) {
        throw std::runtime_error("InsertEdge: insertion isn't valid");
    }
    auto x = xy.GetTail(), y = xy.GetHead();
    if (!IsCovered(x) && !IsCovered(y)) {
        covered_vertices.insert(x);
        covered_vertices.insert(y);

        other_endpoint.emplace(x, y);
        other_endpoint.emplace(y, x);

        endpoint_edges.emplace(x, xy);
        endpoint_edges.emplace(y, xy);
        return;
    }
    if (IsCovered(x)) {
        std::swap(x, y);
    }
    auto y_edge = endpoint_edges.at(y);
    auto other_y = other_endpoint.at(y);
    endpoint_edges.erase(y);
    other_endpoint.erase(y);
    other_endpoint.erase(other_y);
    
    xy.OrientTowards(y);
    y_edge.OrientTowards(y);
    next_edge.emplace(xy, y_edge);
    next_edge.emplace(y_edge, xy);

    if (!IsCovered(x)) {
        covered_vertices.insert(x);
    
        other_endpoint.emplace(other_y, x);
        other_endpoint.emplace(x, other_y);
    
        endpoint_edges.emplace(x, xy);

        return;
    }
    auto x_edge = endpoint_edges.at(x);
    auto other_x = other_endpoint.at(x);
    endpoint_edges.erase(x);
    other_endpoint.erase(x);
    other_endpoint.erase(other_x);

    xy.OrientTowards(x);
    x_edge.OrientTowards(x);
    next_edge.emplace(xy, x_edge);
    next_edge.emplace(x_edge, xy);

    other_endpoint.emplace(other_x, other_y);
    other_endpoint.emplace(other_y, other_x);
}

void SetOfSimplePaths<SAGWithEndpoints>::RemoveEdge(Edge xy) {
    if (!Contains(xy)) {
        throw std::runtime_error("RemoveEdge: set doesn't have xy");
    }
    auto get_other = [&](Edge xy){
        while (!IsPathEndpoint(xy.GetHead())) {
            xy = GetNextPathEdge(xy);
        }
        return xy.GetHead();
    };
    
    auto x = xy.GetTail(), y = xy.GetHead();
    auto yx = xy; yx.SwitchOrientation();
    Vertex other_x = get_other(yx), other_y = get_other(xy);
    other_endpoint.erase(other_x);
    other_endpoint.erase(other_y);

    if (other_x == x && other_y == y) {
        covered_vertices.erase(x);
        covered_vertices.erase(y);

        endpoint_edges.erase(x);
        endpoint_edges.erase(y);
        return;
    }
    if (other_x != x) {
        std::swap(x, y);
        std::swap(xy, yx);
        std::swap(other_x, other_y);
    }
    auto endpoint_edge_y = next_edge.at(xy);
    next_edge.erase(xy);
    next_edge.erase(endpoint_edge_y);
    endpoint_edges.emplace(y, endpoint_edge_y);
    other_endpoint.emplace(y, other_y);
    other_endpoint.emplace(other_y, y);

    if (other_x == x) {
        covered_vertices.erase(x);
        endpoint_edges.erase(x);
        return;
    }
    
    auto endpoint_edge_x = next_edge.at(yx);
    next_edge.erase(yx);
    next_edge.erase(endpoint_edge_x);
    endpoint_edges.emplace(x, endpoint_edge_x);
    other_endpoint.emplace(x, other_x);
    other_endpoint.emplace(other_x, x);
}