#include "graph.h"
#include "solver.h"
#include <cassert>

#include <QApplication>
#include <QTemporaryFile>
#include <QProcess>

#include <stdio.h>

#define BENCH_FILE_MASK   "/tmp/benchfile_%1-%2n_%3-%4k_%5cores.txt"
#define EN_OUT_FILE  "/tmp/bench_en_out.txt"
#define EN_BINARY_PATH "./parpenet/src/epanet2"


void run(int n, int k, QString bench_file_path) {
   
   //create random graph and dunmp to a tmp epanet file
   graph g = graph::random(n, k);
   if (!g.is_connected())
       g.make_connected();

   g.make_symmetric();
   
   char tmp[L_tmpnam];
   tmpnam(tmp);
   g.dump_epanet(tmp);
   
   FILE *bench_file = fopen(bench_file_path.toLocal8Bit().constData(), "a");
   fprintf(bench_file, "%d,%d,", n, k);
   fclose(bench_file);
   
   //prepare for epanet run
   QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();
   penv.insert("EN_BENCH_FILE", bench_file_path);
   QProcess p;
   p.setProcessEnvironment(penv);
   
   //run epanet
   QStringList args = {tmp, EN_OUT_FILE};
   p.setStandardOutputFile("/dev/stdout", QIODevice::Append);
   p.setStandardErrorFile("/dev/stderr", QIODevice::Append);
   p.start(EN_BINARY_PATH, args);
   if (!p.waitForStarted()) {
      std::cerr << "could not start epanet process" << std::endl;
      exit(-1);
   }
   std::cout << "started process with pid " << p.pid() << std::endl;
   p.waitForFinished(-1);
   //std::cout << p.readAllStandardError().constData() << std::endl;
   //std::cout << p.readAllStandardOutput().constData() << std::endl;
   
   //cleanup
   QFile::remove(EN_OUT_FILE);
   QFile::remove(tmp);
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    if (!QFile::exists(EN_BINARY_PATH)) {
        std::cout << "epanet binary not found" << std::endl;
        exit(-1);
    }
    
    std::cout << "using " << EN_BINARY_PATH << " as epanet exe" << std::endl;
    
    int n_start = 100;
    int n_stop = 1800;
    int k_start = 2;
    int k_stop = 20;
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
          run(n, k, bench_file_path);
       }
    }
    
    return 0;
}
