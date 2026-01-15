#pragma once
#include <core/paths.h>

#include <memory>


using SetPointer = std::shared_ptr<SetOfPolygonalPaths<SAGWithEndpoints>>;

class ISetBuilder {
public:
    virtual ~ISetBuilder() = default;
    virtual SetPointer Build(const SAGWithEndpoints& graph) const = 0;
};

class GreedySetBuilder : public ISetBuilder {
public:
    SetPointer Build(const SAGWithEndpoints& graph) const override;
};

class SymGreedySetBuilder : public ISetBuilder {
public:
    SetPointer Build(const SAGWithEndpoints& graph) const override;
};

class ExactSetBuilder : public ISetBuilder {
public:
    SetPointer Build(const SAGWithEndpoints& graph) const override;
};

size_t GetNumberOfPaths(const SAGWithEndpoints& graph, const ISetBuilder& set_builder);

size_t An(const SAGWithEndpoints& graph);
