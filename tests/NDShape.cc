#include <gtest/gtest.h>
#include "../model/NDShape.h"
#include <stdexcept>

class NDShapeTest : public ::testing::Test {
protected:
    void SetUp() override {
        shape3D = new NDShape(3);
    }

    void TearDown() override {
        delete shape3D;
    }

    NDShape* shape3D;
};

/**
 * @test Construct a shape with valid dimension.
 */
TEST_F(NDShapeTest, ConstructWithValidDimension) {
    // dimension must be > 0
    EXPECT_NO_THROW({
        NDShape shape(2);
    });
}

/**
 * @test Attempt to construct a shape with zero dimension -> throws exception.
 */
TEST_F(NDShapeTest, ConstructWithZeroDimension) {
    // dimension == 0 should throw invalid_argument
    EXPECT_THROW({
        NDShape shape(0);
    }, std::invalid_argument);
}

/**
 * @test Verify getDimension returns the expected dimension.
 */
TEST_F(NDShapeTest, GetDimension) {
    // We initialized shape3D with dimension = 3
    EXPECT_EQ(shape3D->getDimension(), static_cast<std::size_t>(3));
}

/**
 * @test Add valid vertices, ensuring the correct dimension is enforced and IDs are assigned incrementally.
 */
TEST_F(NDShapeTest, AddVertexValid) {
    // Add a 3D vertex
    std::vector<double> coords = {1.0, 2.0, 3.0};
    std::size_t id = shape3D->addVertex(coords);

    // Should store exactly one vertex with ID=0 (first insertion),
    // because vertexCounter_ starts at 0
    EXPECT_EQ(id, static_cast<std::size_t>(0));

    auto allVertices = shape3D->getAllVertices();
    ASSERT_EQ(allVertices.size(), 1U);
    EXPECT_EQ(allVertices[0].first, id);
    EXPECT_EQ(allVertices[0].second, coords);
}

/**
 * @test Add vertex with invalid dimension -> throws exception.
 */
TEST_F(NDShapeTest, AddVertexInvalidDimension) {
    // For a 3D shape, coords.size() must be 3
    std::vector<double> wrongSize = {1.0, 2.0};
    EXPECT_THROW({
        shape3D->addVertex(wrongSize);
    }, std::invalid_argument);
}

/**
 * @test Verify adding multiple vertices increments IDs.
 */
TEST_F(NDShapeTest, AddMultipleVertices) {
    std::vector<double> coords1 = {0.0, 0.0, 0.0};
    std::vector<double> coords2 = {1.0, 1.0, 1.0};

    std::size_t id1 = shape3D->addVertex(coords1);
    std::size_t id2 = shape3D->addVertex(coords2);

    // Expect consecutive IDs
    EXPECT_EQ(id1, 0U);
    EXPECT_EQ(id2, 1U);

    auto allVertices = shape3D->getAllVertices();
    EXPECT_EQ(allVertices.size(), 2U);

    // Check that each ID is present with correct coordinates
    bool foundId1 = false, foundId2 = false;
    for (auto& pair : allVertices) {
        if (pair.first == id1) {
            foundId1 = true;
            EXPECT_EQ(pair.second, coords1);
        } else if (pair.first == id2) {
            foundId2 = true;
            EXPECT_EQ(pair.second, coords2);
        }
    }

    EXPECT_TRUE(foundId1);
    EXPECT_TRUE(foundId2);
}

/**
 * @test Add edge between two valid vertices.
 */
TEST_F(NDShapeTest, AddEdgeValid) {
    // Add two vertices
    std::size_t v1 = shape3D->addVertex({1.0, 2.0, 3.0});
    std::size_t v2 = shape3D->addVertex({4.0, 5.0, 6.0});

    // Add an edge between them
    EXPECT_NO_THROW({
        shape3D->addEdge(v1, v2);
    });

    // Check that the edge is present
    const auto& edges = shape3D->getEdges();
    ASSERT_EQ(edges.size(), 1U);
    // Because addEdge stores it as (v1, v2)
    EXPECT_TRUE((edges[0].first == v1 && edges[0].second == v2) ||
                (edges[0].first == v2 && edges[0].second == v1));
}

/**
 * @test Add edge with invalid (non-existent) vertex IDs -> throws exception.
 */
TEST_F(NDShapeTest, AddEdgeWithInvalidVertexId) {
    // shape3D has no vertices added; so ID=0 or ID=1 do not exist
    EXPECT_THROW({
        shape3D->addEdge(0, 1);
    }, std::out_of_range);

    // Add only one vertex => second ID is invalid
    std::size_t v1 = shape3D->addVertex({0,0,0});
    EXPECT_THROW({
        shape3D->addEdge(v1, 999);
    }, std::out_of_range);
}

/**
 * @test Add edge where both vertices are the same -> throws exception.
 */
TEST_F(NDShapeTest, AddEdgeSameVertex) {
    std::size_t v1 = shape3D->addVertex({0,0,0});
    EXPECT_THROW({
        shape3D->addEdge(v1, v1);
    }, std::invalid_argument);
}

/**
 * @test Add edge that already exists -> throws exception.
 */
TEST_F(NDShapeTest, AddEdgeDuplicate) {
    std::size_t v1 = shape3D->addVertex({0,0,0});
    std::size_t v2 = shape3D->addVertex({1,1,1});
    shape3D->addEdge(v1, v2);

    // Attempting to add the same edge again
    EXPECT_THROW({
        shape3D->addEdge(v2, v1);
    }, std::invalid_argument);
}

/**
 * @test Check setVertexCoords for a valid update.
 */
TEST_F(NDShapeTest, SetVertexCoordsValid) {
    std::size_t v1 = shape3D->addVertex({1.0, 2.0, 3.0});
    std::vector<double> newCoords = {7.0, 8.0, 9.0};

    shape3D->setVertexCoords(v1, newCoords);

    // Verify the vertex was updated
    auto allVertices = shape3D->getAllVertices();
    EXPECT_EQ(allVertices.size(), 1U);
    EXPECT_EQ(allVertices[0].second, newCoords);
}

/**
 * @test setVertexCoords with invalid vertex ID -> throws exception.
 */
TEST_F(NDShapeTest, SetVertexCoordsInvalidVertexId) {
    // No vertices => ID does not exist
    EXPECT_THROW({
        shape3D->setVertexCoords(0, {1.0, 2.0, 3.0});
    }, std::out_of_range);
}

/**
 * @test setVertexCoords with newCoords that do not match dimension -> throws exception.
 */
TEST_F(NDShapeTest, SetVertexCoordsInvalidDimension) {
    std::size_t v1 = shape3D->addVertex({1.0, 2.0, 3.0});
    // shape dimension = 3, but we're giving 2 coordinates
    std::vector<double> wrongSize = {9.0, 9.0};
    EXPECT_THROW({
        shape3D->setVertexCoords(v1, wrongSize);
    }, std::invalid_argument);
}

/**
 * @test Remove an existing vertex.
 */
TEST_F(NDShapeTest, RemoveVertexValid) {
    std::size_t v1 = shape3D->addVertex({1,2,3});
    std::size_t v2 = shape3D->addVertex({4,5,6});
    shape3D->addEdge(v1, v2);

    // Removing the vertex that has an edge
    shape3D->removeVertex(v1);

    // Check that the vertex is removed
    auto allVertices = shape3D->getAllVertices();
    EXPECT_EQ(allVertices.size(), 1U);
    EXPECT_EQ(allVertices[0].first, v2);  // only v2 remains

    // Also check that the edge was removed automatically
    const auto& edges = shape3D->getEdges();
    EXPECT_EQ(edges.size(), 0U);
}

/**
 * @test Remove a non-existing vertex -> throws exception.
 */
TEST_F(NDShapeTest, RemoveVertexInvalidId) {
    // shape3D is empty
    EXPECT_THROW({
        shape3D->removeVertex(42);
    }, std::out_of_range);
}

/**
 * @test Remove an existing edge.
 */
TEST_F(NDShapeTest, RemoveEdgeValid) {
    std::size_t v1 = shape3D->addVertex({0,0,0});
    std::size_t v2 = shape3D->addVertex({1,1,1});
    shape3D->addEdge(v1, v2);

    shape3D->removeEdge(v1, v2);

    // No edges remain
    const auto& edges = shape3D->getEdges();
    EXPECT_EQ(edges.size(), 0U);
}

/**
 * @test Remove edge that does not exist -> throws exception.
 */
TEST_F(NDShapeTest, RemoveEdgeNonExisting) {
    std::size_t v1 = shape3D->addVertex({0,0,0});
    std::size_t v2 = shape3D->addVertex({1,1,1});

    // No edge between them yet
    EXPECT_THROW({
        shape3D->removeEdge(v1, v2);
    }, std::out_of_range);

    // Add an edge, remove it, then removing again should fail
    shape3D->addEdge(v1, v2);
    shape3D->removeEdge(v1, v2);
    EXPECT_THROW({
        shape3D->removeEdge(v1, v2);
    }, std::out_of_range);
}

/**
 * @test Retrieve the adjacency matrix.
 */
TEST_F(NDShapeTest, GetAdjacencyMatrix) {
    std::size_t v1 = shape3D->addVertex({0,0,0}); // ID=0
    std::size_t v2 = shape3D->addVertex({1,1,1}); // ID=1
    std::size_t v3 = shape3D->addVertex({2,2,2}); // ID=2

    // Add edges (0,1) and (1,2)
    shape3D->addEdge(v1, v2);
    shape3D->addEdge(v2, v3);

    auto matrix = shape3D->getAdjacencyMatrix();

    // matrix should be (n+1) x (n+1) = 4x4 for 3 vertices
    ASSERT_EQ(matrix.size(), 4U);
    for (auto& row : matrix) {
        ASSERT_EQ(row.size(), 4U);
    }

    // First row/column are headers: top-left = -1
    EXPECT_EQ(matrix[0][0], -1);

    // The row/column headers for i=1..3 should be vertex IDs in sorted order:
    // sorted IDs = [0, 1, 2]
    EXPECT_EQ(matrix[0][1], 0);
    EXPECT_EQ(matrix[0][2], 1);
    EXPECT_EQ(matrix[0][3], 2);

    EXPECT_EQ(matrix[1][0], 0);
    EXPECT_EQ(matrix[2][0], 1);
    EXPECT_EQ(matrix[3][0], 2);

    // Diagonal cells = -1, off-diagonal = 1 if edge exists, else 0
    // row i=1 -> vertex 0, col j=2 -> vertex 1 => edge(0,1)=1
    // row i=1 -> vertex 0, col j=3 -> vertex 2 => edge(0,2)=0
    EXPECT_EQ(matrix[1][2], 1);
    EXPECT_EQ(matrix[1][3], 0);

    // row i=2 -> vertex 1, col j=3 -> vertex 2 => edge(1,2)=1
    EXPECT_EQ(matrix[2][3], 1);
}

/**
 * @test Clone the shape with a different dimension.
 */
TEST_F(NDShapeTest, CloneValid) {
    std::size_t v1 = shape3D->addVertex({1,2,3});
    std::size_t v2 = shape3D->addVertex({4,5,6});
    shape3D->addEdge(v1, v2);

    // Clone into a 2D shape
    NDShape shape2D = shape3D->clone(2);

    // shape2D has the same vertex IDs, edges, but new 2D coordinates that are all zero
    // Also the same vertexCounter_ means adding a new vertex picks up where shape3D left off
    EXPECT_EQ(shape2D.getDimension(), 2U);

    // Check cloned vertices
    auto clonedVertices = shape2D.getAllVertices();
    // Should have 2 vertices with same IDs: v1 and v2
    EXPECT_EQ(clonedVertices.size(), 2U);

    // Check that the coordinates in the clone have size=2
    for (auto& pair : clonedVertices) {
        std::vector<double> coords = pair.second;
        EXPECT_EQ(coords.size(), 2U);
    }

    // Check cloned edges
    const auto& clonedEdges = shape2D.getEdges();
    EXPECT_EQ(clonedEdges.size(), 1U);
}

/**
 * @test Clone with dimension == 0 -> throws exception.
 */
TEST_F(NDShapeTest, CloneInvalidDimension) {
    EXPECT_THROW({
        shape3D->clone(0);
    }, std::invalid_argument);
}

/**
 * @test Update the shape from an adjacency matrix.
 */
TEST_F(NDShapeTest, UpdateFromAdjacencyMatrix) {
    // Prepare shape with 3 vertices
    std::size_t v0 = shape3D->addVertex({0,0,0});
    std::size_t v1 = shape3D->addVertex({1,1,1});
    std::size_t v2 = shape3D->addVertex({2,2,2});

    // Currently no edges
    auto edges = shape3D->getEdges();
    EXPECT_EQ(edges.size(), 0U);

    // We'll create a 3x3 adjacency matrix (no headers) to connect (v0, v1) and (v1, v2).
    // Vertex IDs after sort => [0,1,2].
    // We only set upper triangle [i,j], j>i. The matrix is symmetrical in concept but we only need i<j in code.
    std::vector<std::vector<int>> newMatrix = {
        {0, 1, 0}, // adjacency from v0 to (v0,v1,v2)
        {1, 0, 1}, // adjacency from v1 to (v0,v1,v2)
        {0, 1, 0}  // adjacency from v2 to (v0,v1,v2)
    };

    shape3D->updateFromAdjacencyMatrix(newMatrix);

    // Now the shape should have edges (0,1) and (1,2)
    edges = shape3D->getEdges();
    EXPECT_EQ(edges.size(), 2U);

    // Confirm the edges are correct
    bool found01 = false, found12 = false;
    for (auto& edge : edges) {
        if ((edge.first == v0 && edge.second == v1) ||
            (edge.first == v1 && edge.second == v0)) {
            found01 = true;
        }
        if ((edge.first == v1 && edge.second == v2) ||
            (edge.first == v2 && edge.second == v1)) {
            found12 = true;
        }
    }
    EXPECT_TRUE(found01);
    EXPECT_TRUE(found12);
}

/**
 * @test updateFromAdjacencyMatrix with invalid dimensions -> throws exception.
 */
TEST_F(NDShapeTest, UpdateFromAdjacencyMatrixInvalidDimensions) {
    // shape3D has no vertices initially, or let's have 2:
    shape3D->addVertex({0,0,0});
    shape3D->addVertex({1,1,1});

    // The shape expects a 2x2 matrix, let's give it 3x3
    std::vector<std::vector<int>> wrongMatrix(3, std::vector<int>(3, 0));
    EXPECT_THROW({
        shape3D->updateFromAdjacencyMatrix(wrongMatrix);
    }, std::invalid_argument);
}

/**
 * @test updateFromAdjacencyMatrix with a valid dimension but a cell not in {0,1} -> throws exception.
 */
TEST_F(NDShapeTest, UpdateFromAdjacencyMatrixInvalidCellValue) {
    // 2 vertices => needs a 2x2 matrix
    shape3D->addVertex({0,0,0});
    shape3D->addVertex({1,1,1});

    // We'll put an invalid value '2' in one cell
    std::vector<std::vector<int>> invalidMatrix = {
        {0, 2},
        {2, 0}
    };

    EXPECT_THROW({
        shape3D->updateFromAdjacencyMatrix(invalidMatrix);
    }, std::invalid_argument);
}
