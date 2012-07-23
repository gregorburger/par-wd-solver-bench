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
