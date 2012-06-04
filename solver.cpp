#include "solver.h"
#include "graph.h"
#include <algorithm>
#include <cassert>

solver::solver(graph &g) : g(g), n(g.nodes.size()) {
    row_idx = new int[n+1];
    b = new double[n];
    x = new double[n];
    
    std::fill_n(b, n, 1.0);
    row_idx[0] = 1;
    
    nnz = 0;
    
    for (int i = 0; i < g.connections.size(); ++i) {
        int nz = 0;
        for (int j = 0; j < g.connections[i].size(); ++j) {
            if (g.connections[i][j] >= i) //only upper triangle part
                nz++;
        }
        nnz += nz;
        row_idx[i+1] = nnz + 1;
    }
    
    
    values = new double[nnz];
    std::fill(values, values + nnz, 1.0);
    columns = new int[nnz];
            
    int c = 0;
    
    for (int i = 0; i < g.connections.size(); ++i) {
        std::sort(g.connections[i].begin(), g.connections[i].end());
        for (int j = 0; j < g.connections[i].size(); ++j) {
            if (g.connections[i][j] < i) //only upper triangle part
                continue;
            columns[c++] = g.connections[i][j]+1;
        }
    }
    
    init();
}

/* PARDISO prototype. */
extern "C" {
void pardisoinit (void   *, int    *,   int *, int *, double *, int *);
void pardiso     (void   *, int    *,   int *, int *,    int *, int *,
                  double *, int    *,    int *, int *,   int *, int *,
                  int *, double *, double *, int *, double *);

void pardiso_chkmatrix  (int *, int *, double *, int *, int *, int *);
void pardiso_chkvec     (int *, int *, double *, int *);
void pardiso_printstats (int *, int *, double *, int *, int *, int *,
                         double *, int *);
}

solver::~solver() {
    int error;
    int phase = 0;
    
    pardiso (handle, &maxfct, &mnum, &mtype, &phase,
             &n, values, row_idx, columns, 0/*perm*/, &nrhs,
             iparm, &msglvl, 0, 0, &error,  dparm);
    if (error != 0) {
        printf("error releasing pardios memory\n");
    }
    
    delete[] row_idx;
    delete[] values;
    delete[] columns;
}

float solver::solve() {
    int error;
    int phase = 23;
    pardiso (handle, &maxfct, &mnum, &mtype, &phase,
             &n, values, row_idx, columns, 0/*perm*/, &nrhs,
             iparm, &msglvl, b, x, &error,  dparm);
    
    if (error != 0) {
        printf("\nERROR during solution: %d", error);
        exit(3);
    }
}

void solver::init() {
    int error = 0;
    int solver = 0; /* use sparse direct solver */
    mtype = 2;
    maxfct = 1;
    mnum = 1;
    nrhs = 1;
    msglvl = 0;
    
    int num_procs;
    
    char *var = getenv("OMP_NUM_THREADS");
    if (var != NULL) {
        sscanf(var , "%d" ,&num_procs);
    } else {
        printf("Set environment OMP_NUM_THREADS\n");
        exit(1);
    }
    
    
    iparm[2] = num_procs;
    iparm[7] = 0; //no iterative refinement
    
    
    pardisoinit (handle,  &mtype, &solver, iparm, dparm, &error);
    
    if (error != 0) {
        if (error == -10 )
            printf("No license file found \n");
        if (error == -11 )
            printf("License is expired \n");
        if (error == -12 )
            printf("Wrong username or hostname \n");
    } else {
        printf("[PARDISO]: License check was successful ... \n");
    }
    
    pardiso_chkmatrix (&mtype, &n, values, row_idx, columns, &error);
    
    if (error != 0) {
        printf("ERROR in consistency of matrix: %d\n", error);
        exit(1);
    }
    
    fflush(stdout);
    
    int phase = 11;
    
    pardiso (handle, &maxfct, &mnum, &mtype, &phase,
             &n, values, row_idx, columns, 0, &nrhs,
             iparm, &msglvl, 0, 0, &error, dparm);
    
    if (error != 0) {
        printf("ERROR during symbolic factorization: %d\n", error);
        exit(1);
    }
    printf("Reordering completed ... \n");
    printf("Number of nonzeros in factors  = %d\n", iparm[17]);
    printf("Number of factorization MFLOPS = %d\n", iparm[18]);
}

