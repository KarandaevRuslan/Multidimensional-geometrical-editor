#include "NDShape.h"
#include <QDebug>
#include <algorithm>

NDShape::NDShape(std::size_t dimension)
    : dimension_(dimension), vertexCounter_(0)
{
    if (dimension == 0) {
        QString msg = "Dimension must be greater than zero.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }
}

std::size_t NDShape::getDimension() const {
    return dimension_;
}

std::size_t NDShape::addVertex(const std::vector<double>& coords) {
    if (coords.size() != dimension_) {
        QString msg = "Coordinate count does not match shape dimension.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }
    std::size_t id = vertexCounter_++;
    vertices_[id] = coords;
    return id;
}

void NDShape::addEdge(std::size_t id1, std::size_t id2) {
    if (vertices_.find(id1) == vertices_.end() ||
        vertices_.find(id2) == vertices_.end()) {
        QString msg = "One or both vertex IDs do not exist.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }

    if (id1 == id2) {
        QString msg = "Edges with the same vertices are forbidden.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    auto it = std::find_if(edges_.begin(), edges_.end(),
                           [id1, id2](const std::pair<std::size_t, std::size_t>& edge) {
                               return (edge.first == id1 && edge.second == id2) ||
                                      (edge.first == id2 && edge.second == id1);
                           });
    if (it != edges_.end()) {
        QString msg = "Edge already exists.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    edges_.emplace_back(id1, id2);
}

std::vector<std::pair<std::size_t, std::vector<double>>> NDShape::getAllVertices() const {
    std::vector<std::pair<std::size_t, std::vector<double>>> result;
    result.reserve(vertices_.size());
    for (const auto& [id, coords] : vertices_) {
        result.emplace_back(id, coords);
    }
    return result;
}

void NDShape::setVertexCoords(std::size_t vertexId, const std::vector<double>& newCoords) {
    if (vertices_.find(vertexId) == vertices_.end()) {
        QString msg = "Vertex ID does not exist.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }
    if (newCoords.size() != dimension_) {
        QString msg = "New coordinates do not match shape dimension.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }
    vertices_[vertexId] = newCoords;
}

const std::vector<std::pair<std::size_t, std::size_t>>& NDShape::getEdges() const {
    return edges_;
}

NDShape NDShape::clone(std::size_t newDim) const {
    if (newDim == 0) {
        QString msg = "Clone dimension must be > 0.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    NDShape cloned(newDim);
    // copy over edges and counter
    cloned.edges_ = edges_;
    cloned.vertexCounter_ = vertexCounter_;

    // adjust each vertex's coords to the new dimension
    for (const auto& [id, oldCoords] : vertices_) {
        std::vector<double> newCoords;
        newCoords.reserve(newDim);

        // copy as many old coords as will fit
        std::size_t copyCount = std::min(oldCoords.size(), newDim);
        for (std::size_t i = 0; i < copyCount; ++i)
            newCoords.push_back(oldCoords[i]);

        // zero-fill any extra dimensions
        for (std::size_t i = copyCount; i < newDim; ++i)
            newCoords.push_back(0.0);

        cloned.vertices_[id] = std::move(newCoords);
    }

    return cloned;
}


void NDShape::removeVertex(std::size_t vertexId) {
    auto it = vertices_.find(vertexId);
    if (it == vertices_.end()) {
        QString msg = "Vertex ID does not exist.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }
    vertices_.erase(it);

    auto newEnd = std::remove_if(edges_.begin(), edges_.end(),
                                 [vertexId](const std::pair<std::size_t, std::size_t>& edge) {
                                     return edge.first == vertexId || edge.second == vertexId;
                                 });
    edges_.erase(newEnd, edges_.end());
}

void NDShape::removeEdge(std::size_t id1, std::size_t id2) {
    bool found = false;
    auto newEnd = std::remove_if(edges_.begin(), edges_.end(),
                                 [id1, id2, &found](const std::pair<std::size_t, std::size_t>& edge) {
                                     if ((edge.first == id1 && edge.second == id2) ||
                                         (edge.first == id2 && edge.second == id1)) {
                                         found = true;
                                         return true;
                                     }
                                     return false;
                                 });
    if (!found) {
        QString msg = "Edge between given vertices does not exist.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }
    edges_.erase(newEnd, edges_.end());
}

std::vector<std::vector<int>> NDShape::getAdjacencyMatrix() const {
    std::vector<std::size_t> vertexIds;
    for (const auto& pair : vertices_) {
        vertexIds.push_back(pair.first);
    }
    std::sort(vertexIds.begin(), vertexIds.end());

    std::size_t n = vertexIds.size();
    std::vector<std::vector<int>> matrix(n + 1, std::vector<int>(n + 1, 0));

    matrix[0][0] = -1;
    for (std::size_t i = 0; i < n; ++i) {
        matrix[0][i + 1] = static_cast<int>(vertexIds[i]);
        matrix[i + 1][0] = static_cast<int>(vertexIds[i]);
    }

    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            if (i == j) {
                matrix[i + 1][j + 1] = -1;
            } else {
                bool edgeFound = false;
                for (const auto& edge : edges_) {
                    if ((edge.first == vertexIds[i] && edge.second == vertexIds[j]) ||
                        (edge.first == vertexIds[j] && edge.second == vertexIds[i])) {
                        edgeFound = true;
                        break;
                    }
                }
                matrix[i + 1][j + 1] = edgeFound ? 1 : 0;
            }
        }
    }
    return matrix;
}


void NDShape::updateFromAdjacencyMatrix(const std::vector<std::vector<int>>& matrix) {
    std::size_t n = vertices_.size();

    if (matrix.size() != n) {
        QString msg = "Adjacency matrix row count does not match the number of vertices.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }
    for (const auto& row : matrix) {
        if (row.size() != n) {
            QString msg = "Adjacency matrix column count does not match the number of vertices.";
            qWarning() << msg;
            throw std::invalid_argument(msg.toStdString());
        }
    }

    std::vector<std::size_t> vertexIds;
    for (const auto& pair : vertices_) {
        vertexIds.push_back(pair.first);
    }
    std::sort(vertexIds.begin(), vertexIds.end());

    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            int newVal = matrix[i][j];

            if (newVal != 0 && newVal != 1) {
                QString msg = "Matrix off-diagonal cells must contain either 0 or 1.";
                qWarning() << msg;
                throw std::invalid_argument(msg.toStdString());
            }

            bool exists = false;
            for (const auto& edge : edges_) {
                if ((edge.first == vertexIds[i] && edge.second == vertexIds[j]) ||
                    (edge.first == vertexIds[j] && edge.second == vertexIds[i])) {
                    exists = true;
                    break;
                }
            }
            int currentVal = exists ? 1 : 0;

            if (newVal != currentVal) {
                if (newVal == 1) {
                    addEdge(vertexIds[i], vertexIds[j]);
                } else {
                    removeEdge(vertexIds[i], vertexIds[j]);
                }
            }
        }
    }
}

const std::vector<double>& NDShape::getVertex(std::size_t vertexId) const
{
    auto it = vertices_.find(vertexId);
    if (it == vertices_.end()) {
        throw std::out_of_range("NDShape::getVertex: vertex ID not found");
    }
    return it->second;
}

int NDShape::verticesSize() const
{
    return vertices_.size();
}

int NDShape::edgesSize() const
{
    return edges_.size();
}
