#include "graph.h"
#include <iostream>

int main(int argc, char **argv) {
   std::cout << "hallo" << std::endl;
   char path[1024];
   
   int n = 10000;
   
   for (int k = 2; k <= 12; k += 2) {
      graph g = graph::random(n, k);
      //g.make_symmetric();
      if (!g.is_connected())
         g.make_connected();
      snprintf(path, 1024, "matrix_%d-%d.dat", n, k);
      g.dump_matlab(path);
      snprintf(path, 1024, "matrix_%d-%d.dat", n, k);
      
      double avg_k = 0.0;
      
      for (int i = 0; i < g.connections.size(); i++) {
         avg_k += g.connections[i].size() - 1;
      }
      std::cout << "average k: " << (avg_k/g.connections.size()) << " " << k << std::endl;
   }
   return 0;
}
