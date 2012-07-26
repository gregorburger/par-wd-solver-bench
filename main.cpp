#include "graph.h"
#include "solver.h"
#include <cassert>

#include <QApplication>
#include <QTemporaryFile>
#include <QProcess>

#include <stdio.h>
#include <iostream>

#define BENCH_FILE_MASK   "benchfile_%1-%2n_%3-%4k_%5cores.txt"
#define EN_OUT_FILE  "bench_en_out.txt"
#define EN_PAR_BINARY_PATH "./epanet-par/src/epanet-par"
#define EN_SEQ_BINARY_PATH "./epanet-seq/src/epanet-seq"


void run(const graph &g, QString bench_file_path) {
   //dump to a tmp epanet file
   char tmp[L_tmpnam];
   tmpnam(tmp);
   g.dump_epanet(tmp);
   
   //prepare for epanet run
   QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();
   penv.insert("EN_BENCH_FILE", bench_file_path);
   QProcess p;
   p.setProcessEnvironment(penv);
   
   //run epanet
   QStringList args = {tmp, EN_OUT_FILE};
   p.setStandardOutputFile("/dev/stdout", QIODevice::Append);
   p.setStandardErrorFile("/dev/stderr", QIODevice::Append);
   p.start(EN_SEQ_BINARY_PATH, args);
   if (!p.waitForStarted()) {
      std::cerr << "could not start epanet process" << std::endl;
      exit(-1);
   }
   p.waitForFinished(-1);
   
   FILE *bench_file = fopen(bench_file_path.toLocal8Bit().constData(), "a");
   fprintf(bench_file, ",");
   fclose(bench_file);
   
   p.start(EN_PAR_BINARY_PATH, args);
   if (!p.waitForStarted()) {
      std::cerr << "could not start epanet process" << std::endl;
      exit(-1);
   }
   p.waitForFinished(-1);
   
   bench_file = fopen(bench_file_path.toLocal8Bit().constData(), "a");
   fprintf(bench_file, "\n");
   fclose(bench_file);
   
   //cleanup
   QFile::remove(EN_OUT_FILE);
   QFile::remove(tmp);
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    if (!QFile::exists(EN_SEQ_BINARY_PATH) || !QFile::exists(EN_PAR_BINARY_PATH)) {
        std::cout << "epanet binaries not found" << std::endl;
        exit(-1);
    }
    
    std::cout << "using " << EN_PAR_BINARY_PATH << " as parallel epanet exe" << std::endl;
    std::cout << "using " << EN_SEQ_BINARY_PATH << " as sequential epanet exe" << std::endl;
    
    int n_start = 1000;
    int n_stop = 1800;
    int k_start = 4;
    int k_stop = 12;
    if (!QProcessEnvironment::systemEnvironment().contains("OMP_NUM_THREADS")) {
       std::cout << "set OMP_NUM_THREADS" << std::endl;
       exit(-1);
    }
    QString omp_num_threads = QProcessEnvironment::systemEnvironment().value("OMP_NUM_THREADS");
    QString bench_file_path = QString(BENCH_FILE_MASK)
          .arg(n_start).arg(n_stop)
          .arg(k_start).arg(k_stop)
          .arg(omp_num_threads);
    
    if (QFile::exists(bench_file_path)) {
       std::cout << "benchfile exists" << std::endl;
       exit(-1);
    }
    
    for (int n = n_start; n <= n_stop; n+= 200) {
       for (int k = k_start; k <= k_stop; k+= 1) {
            graph g;
            if (k == 1) {
                std::cerr << "k == 1 not possible" << std::endl;
                exit(-1);
            }
            if (k == 2) {
                g = graph::circle(n);
            }
            if (k == 3) {
                g = graph::ring(n/2);
            }
            if (k == 4) {
                g = graph::torus(n/4, 4);
            }
            if (k > 4) {
                g = graph::random(n, k);
                if (!g.is_connected())
                    g.make_connected();
            }
            
            FILE *bench_file = fopen(bench_file_path.toLocal8Bit().constData(), "a");
            fprintf(bench_file, "%d,%d,", n, k);
            fclose(bench_file);
            
            //g.make_symmetric();
            run(g, bench_file_path);
       }
    }
    
    return 0;
}
