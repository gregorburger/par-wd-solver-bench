#include "graph.h"
#include <cstdio>
#include <memory>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/random.hpp>

#include <QPrinter>
#include <QPainter>
#include <QApplication>

#include "vp-tree.h"


boost::random::mt19937 rng(std::time(0));
//boost::random::normal_distribution<> gen(0.0, 0.7);
boost::random::uniform_01<> gen;

void graph::dump_matlab(const char *file) const {
    FILE *out = fopen(file, "w");
    for (int i = 0; i < nodes.size(); ++i) {
        for (int j = 0; j < connections[i].size(); ++j) {
            fprintf(out, "%d\t%d\t %f\n", i+1, connections[i][j]+1, gen(rng));
        }
    }
    fclose(out);
}

void graph::dump_dot(const char *file) const {
    FILE *out = fopen(file, "w");
    
    fprintf(out, "graph G {\n");
    
    for (int i = 0; i < nodes.size(); ++i) {
        for (int j = 0; j < connections[i].size(); j++) {
            fprintf(out, "%d -- %d;\n", i, connections[i][j]);
        }
    }
    
    fprintf(out, "}");
    fclose(out);
}

#define SCALE 700
#define MARGIN 15
    
/**
 * @brief plot graph g to a pdf
 * @param g the graph
 * @param file where to plot
 */
void graph::plot(const char *file) const {
    QPrinter printer;
    printer.setPageMargins(0, 0, 0, 0, QPrinter::DevicePixel);
    printer.setPaperSize(QSizeF(SCALE+2*MARGIN,SCALE+2*MARGIN), QPrinter::DevicePixel);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(file);
    
    QPainter p(&printer);
    QPen line_pen = p.pen();
    line_pen.setWidth(0.5);
    QPen arc_pen = p.pen();
    arc_pen.setWidth(1);
    p.setPen(line_pen);
    
    QTransform transform;
    transform.translate(MARGIN, MARGIN);
    p.setTransform(transform);
    
    p.drawLine(0, 0, SCALE, 0);
    p.drawLine(SCALE, 0, SCALE, SCALE);
    p.drawLine(SCALE, SCALE, 0, SCALE);
    p.drawLine(0, SCALE, 0, 0);
    
    for (int i = 0; i < nodes.size(); ++i) {
        coord from = nodes[i];
        p.setPen(arc_pen);
        p.drawArc(from.x*SCALE-2,from.y*SCALE-2, 4, 4, 0, 16*360);
        for (int j = 0; j < connections[i].size(); ++j) {
            coord to = nodes[connections[i][j]];
            p.setPen(line_pen);
            p.drawLine(from.x*SCALE, from.y*SCALE, to.x*SCALE, to.y*SCALE);
        }
    }
    p.end();
}

/**
 * @brief generate a rondom graph::cord on ([0,1], [0,1])
 * @return random graph::coord
 */
graph::coord random_coord() {
    return graph::coord{gen(rng), gen(rng)};
}

graph graph::random(int n, int k) {
    k++; //self always included
    graph g;
    g.nodes.resize(n);
    
#pragma omp paralle for
    for (int i = 0; i < n; ++i) {
        g.nodes[i] = random_coord();
    }

    g.connections.resize(n);
    
    auto dist_fun = [&g](const int &i, const int &j){
        return g.nodes[i].dist(g.nodes[j]);
    };

    VpTree<int, decltype(dist_fun)> tree(dist_fun);
    std::vector<int> items(n);
    int first = 0;
    std::generate(items.begin(), items.end(), [&first]{ return first++; });
    tree.create(items);
    
#pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        std::vector<double> dists;
        tree.search(i, k, &g.connections[i], &dists);
        g.connections[i].erase(g.connections[i].begin(), g.connections[i].begin()+1);
    }
    
    return g;
}
    
int graph::make_symmetric() {
    int added;
    for (int i = 0; i < nodes.size(); ++i) {
        for (int j = 0; j < connections[i].size(); ++j) {
            int row = connections[i][j];
            if (!std::count(connections[row].begin(), connections[row].end(), i)) {
                connections[row].push_back(i);
                added++;
            }
        }
    }
    return added;
}

struct edge {
    int i, j;
    edge(int i, int j) : i(i), j(j) {
        if (j < i) std::swap(i, j);
    }
    bool operator==(const edge &other) const {
        return i == other.i && j == other.j;
    }
};

struct edge_hash {
    std::size_t operator()(const edge &e) const {
        return e.i*73856093 ^ e.j*19349663;
    }
};

graph graph::circle(int n) {
    graph g;
    g.nodes.resize(n);
    g.connections.resize(n);
    
    for (int i = 0; i < n; i++) {
        g.connections[i].push_back((i+1)%n);
    }
    
    return g;
}

graph graph::ring(int n) {
    graph g;
    g.connections.resize(2*n);
    g.nodes.resize(2*n); //not used but stores the size ;-)
    
    for (int i = 0; i < n; i++) {
        g.connections[i].push_back((i+1)%n);
        g.connections[n+i].push_back(n+(i+1)%n);
        g.connections[i].push_back(n+i);
    }
    assert(g.connections[n-1][0] == 0);
    assert(g.connections[2*n-1][0] == n);
    return g;
}

graph graph::torus(int n, int k) {
    graph g;
    g.nodes.resize(n*k);
    g.connections.resize(n*k);
    
    for (int j = 0; j < k; j++) {
        for (int i = 0; i < n; i++) {
            g.connections[n*j+i].push_back(n*j+(i+1)%n);
            g.connections[n*j+i].push_back(n*((j+1)%k)+i);
        }
    }
    
    return g;
}

boost::random::uniform_int_distribution<> elev_rnd(3, 12);
boost::random::uniform_int_distribution<> demand_rnd(1, 5);

void graph::dump_epanet(const char *file) const {
    FILE *out = fopen(file, "w");
    
    fprintf(out, "[TITLE]\n");
    
    fprintf(out, "\n[JUNCTIONS]\n");
    
    for (int i = 0; i < nodes.size(); ++i) {
        fprintf(out, " %d               	%d           	%d           	                	;\n", i+2, elev_rnd(rng)*10, demand_rnd(rng)*10); //id 1 is for the tank
    }
    
    
    fprintf(out, "\n[RESERVOIRS]\n");
    fprintf(out, " 1               	0           	                	;\n");
    
    
    fprintf(out, "\n[PIPES]\n");
    int pipe_id = 2;
    for (int i = 0; i < nodes.size(); ++i) {
        for (int j = 0; j < connections[i].size(); ++j) {
            int node1 = i;
            int node2 = connections[node1][j];
            if (node1 == node2) {
                continue;
            }
            fprintf(out, " %d               	%d               	%d               	1000        	12          	100         	0           	Open  	;\n", pipe_id++, node1+2, node2+2);
        }
    }
    fprintf(out, " %d               	%d               	%d               	1000        	12          	100         	0           	Open  	;\n", pipe_id++, 2, 1);
    
    fprintf(out, "\n[TIMES]\n");
    fprintf(out, "DURATION    250 HOURS");
    fprintf(out, "\n[END]");
    
    fclose(out);
}


bool graph::is_connected() const {
   boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> g;
   for (int i = 0; i < connections.size(); i++) {
       for (int j = 0; j < connections[i].size(); j++) {
           boost::add_edge(i, connections[i][j], g);
       }
   }
   std::vector<int> component(boost::num_vertices(g));
   int nc = boost::connected_components(g, &component[0]);
   
   return nc == 1;
}

int graph::make_connected() {
    boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> g;
    for (int i = 0; i < connections.size(); i++) {
        for (int j = 0; j < connections[i].size(); j++) {
            boost::add_edge(i, connections[i][j], g);
        }
    }
    std::vector<int> component(boost::num_vertices(g));
    int nc = boost::strong_components(g, &component[0]);
    int n_graphs = nc; //store for return
    for (int c = 0; c < nc-1; c++) {
        std::pair<int, int> new_vertex(-1, -1);
        double min_dist = 10.0; //max dist is max sqrt(2)
        for (int i = 0; i < component.size(); i++) {
            if (component[i] != c) continue;
            for (int j = 0; j < component.size(); j++) {
                if (component[j] != c+1) continue;
                double dist = nodes[i].dist(nodes[j]);
                if (dist < min_dist) {
                    dist = min_dist;
                    new_vertex.first = i;
                    new_vertex.second = j;
                }
            }
        }
        connections[new_vertex.first].push_back(new_vertex.second);
        assert(new_vertex.first >= 0);
    }
    assert(is_connected());
    return n_graphs;
}
