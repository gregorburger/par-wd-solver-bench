#ifndef SOLVER_H
#define SOLVER_H

struct graph;

struct solver {
    /**
     * @brief construct csr sparse matrix formt from graph g
     * @param g input graph
     */
    solver(graph &g);
    
    virtual ~solver();
    
    /**
     * @brief solve the system and take time
     * @return time taken to solve the system
     */
    float solve();
    
private:
    void init();
    
    graph &g;
    int n, nnz;
    double *values, *b, *x;
    int *columns, *row_idx;
    
    //pardiso vars
    void    *handle[64];    //handle for pardiso
    int      iparm[64];
    double   dparm[64];
    int      mtype;     //real positiv symmetric
    int      maxfct;
    int      mnum;
    int      nrhs;
    int      msglvl;
};

#endif // SOLVER_H
