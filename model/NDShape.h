#ifndef NDSHAPE_H
#define NDSHAPE_H

#include <vector>
#include <map>
#include <utility>
#include <cstddef>

/**
 * @brief NDShape represents a multidimensional figure using a simple B-rep model.
 */
class NDShape {
public:
    NDShape() = default;
    ~NDShape() = default;
    /**
     * @brief Constructs an NDShape with the specified dimension.
     * @param dimension The dimension of the figure (e.g., 2 for 2D, 3 for 3D).
     * @throws std::invalid_argument If dimension is zero.
     */
    NDShape(std::size_t dimension);

    /**
     * @brief Retrieves the dimension of the shape.
     * @return The dimension as a size_t.
     */
    std::size_t getDimension() const;

    /**
     * @brief Adds a vertex to the shape.
     *
     * Validates the size of the coordinates against the shape's dimension.
     * Automatically assigns a new unique vertex ID.
     *
     * @param coords The coordinates for the vertex.
     * @return The newly assigned vertex ID.
     * @throws std::invalid_argument If coords.size() != getDimension().
     */
    std::size_t addVertex(const std::vector<double>& coords);

    /**
     * @brief Adds an edge connecting two existing vertices by their IDs.
     *
     * Checks that both vertices exist, ensures the edge does not already exist
     * and vertices are different.
     *
     * @param id1 The ID of the first vertex.
     * @param id2 The ID of the second vertex.
     * @throws std::out_of_range If either vertex ID does not exist.
     * @throws std::invalid_argument If the edge already exists,
     * or if vertices the same.
     */
    void addEdge(std::size_t id1, std::size_t id2);

    /**
     * @brief Returns a complete list of vertex data.
     *
     * Each element of the returned vector is a pair where:
     *  - first = vertex ID
     *  - second = coordinate vector of that vertex
     *
     * @return A vector of (ID, coordinates) pairs.
     */
    std::vector<std::pair<std::size_t, std::vector<double>>> getAllVertices() const;

    /**
     * @brief Updates the coordinates of an existing vertex.
     *
     * The size of newCoords must match the shape's dimension.
     *
     * @param vertexId The ID of the vertex to update.
     * @param newCoords The new coordinates for the vertex.
     * @throws std::out_of_range If vertexId does not exist.
     * @throws std::invalid_argument If newCoords.size() != getDimension().
     */
    void setVertexCoords(std::size_t vertexId, const std::vector<double>& newCoords);

    /**
     * @brief Retrieves the edges of this shape as pairs of vertex IDs.
     *
     * @return A const reference to the vector of edges.
     */
    const std::vector<std::pair<std::size_t, std::size_t>>& getEdges() const;

    /**
     * @brief Clones the current shape into a new NDShape with the specified dimension.
     *
     * The new shape will:
     *  - Have the same vertex IDs.
     *  - Have the same edges.
     *  - Have newly allocated coordinate storage for each vertex (all zeroed),
     *    or left uninitialized so that they can be set by the caller.
     *  - Copy the same vertexCounter_ so the next added vertex picks up where this left off.
     *
     * @param newDim The dimension of the clone. Must be > 0.
     * @return A new NDShape instance with the requested dimension,
     *         containing the same IDs and edges, but new coordinate arrays.
     * @throws std::invalid_argument If newDim == 0.
     */
    NDShape clone(std::size_t newDim) const;

    /**
     * @brief Removes the vertex with the given ID.
     *
     * Also removes any edges incident to this vertex.
     *
     * @param vertexId The ID of the vertex to remove.
     * @throws std::out_of_range If the vertex does not exist.
     */
    void removeVertex(std::size_t vertexId);

    /**
     * @brief Removes an edge connecting two vertices.
     *
     * The function removes all occurrences of an edge between the specified vertices.
     * If the edge does not exist, an exception is thrown.
     *
     * @param id1 The ID of the first vertex.
     * @param id2 The ID of the second vertex.
     * @throws std::out_of_range If no such edge exists.
     */
    void removeEdge(std::size_t id1, std::size_t id2);

    /**
     * @brief Returns the vertices adjacency matrix with their IDs as headers.
     *
     * The returned matrix is a two-dimensional vector of integers where the first row
     * and first column contain the vertex IDs as headers. The top-left cell is set to -1,
     * as well as main diagonal cells. Each cell contains 1 if an edge exists between
     * the corresponding vertices and 0 otherwise.
     *
     * @return A 2D vector of integers representing the adjacency matrix.
     */
    std::vector<std::vector<int>> getAdjacencyMatrix() const;

    /**
     * @brief Updates the NDShape's edges based on the provided adjacency matrix data.
     *
     * The provided matrix is expected to be an n x n matrix (where n is the number of vertices),
     * with all off-main-diagonal cells containing 0 or 1.
     * For each off-diagonal cell, if the matrix indicates 1 (and no edge exists), the edge is added;
     * if it indicates 0 (and an edge exists), the edge is removed.
     *
     * @param matrix A 2D vector of integers representing the new adjacency matrix data (without headers).
     * @throws std::invalid_argument If the matrix dimensions are invalid,
     * or if any cell contains an invalid value.
     */
    void updateFromAdjacencyMatrix(const std::vector<std::vector<int>>& matrix);

private:
    std::size_t dimension_;
    std::map<std::size_t, std::vector<double>> vertices_;
    std::vector<std::pair<std::size_t, std::size_t>> edges_;
    std::size_t vertexCounter_;
};

#endif // NDSHAPE_H
