#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <cmath>
#include <iostream>

/**
 * @brief 2d Graph abstraction of a water distribution network
 */
class graph {
public:
    struct coord {
        double x, y;
        coord operator-(const coord &other) const {
            return coord{x-other.x, y-other.y};
        }
        double length() {
            return (x*x + y*y);
        }
        double dist(const graph::coord &other) const {
            return std::sqrt((*this - other).length());
        }
    };

    /**
     * @brief generates a random graph based on k nearest neighbors of randomly generated nodes
     * @param n number of nodes
     * @param k number of connections per node
     * @return the random graph
     */
    static graph random(int n, int k);
    
    /**
     * @brief return a circle graph with n nodes. each node has two neighbors
     * @param n
     * @return 
     */
    static graph circle(int n);
    
    /**
     * @brief generate a ring with 2*n nodes and n*3 edges => k==3
     * @param n
     * @return 
     */
    static graph ring(int n);
    
    /**
     * @brief generate a torus graph with n nodes in length and k around the torous
     * it hast n*k nodes with 2*n*k connections which means in a undirected graph 4
     * neighbors per node
     * @param n length of torous
     * @param k height of torous
     * @return a graph representing a torous
     */
    static graph torus(int n, int k);

    /**
     * @brief dump graph for loading int matlab with load and spconvert
     * @param file
     */
    void dump_matlab(const char *file) const;
    
    /**
     * @brief dump into a runnable epanet2 file
     * @param to where
     */
    void dump_epanet(const char *file) const;
    
    /**
     * @brief dump into dot file 
     * @param to where
     */
    void dump_dot(const char *file) const;
    
    /**
     * @brief plot to a pdf file
     * @param file
     */
    void plot(const char *file) const;
    
    /**
     * @brief make_symmetric
     * @return 
     */
    int make_symmetric();
    
    /**
     * @brief checks if the graph is connected
     * @return true if connected otherwise false
     */
    bool is_connected() const;
    bool is_connected(bool *seen) const;
    
    int make_connected();
    
    
    std::vector<coord> nodes;
    std::vector<std::vector<int> > connections;
};
    
#endif // GRAPH_H
