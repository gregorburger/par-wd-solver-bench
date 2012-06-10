#include "graph.h"
#include "solver.h"
#include <cassert>

#include <QApplication>
#include <QTemporaryFile>
#include <QProcess>

#include <stdio.h>

#define BENCH_FILE   "/tmp/benchfile.txt"
#define EN_OUT_FILE  "/tmp/bench_en_out.txt"

void run(int n, int k, QString epanet_exe) {
   
   //create random graph and dunmp to a tmp epanet file
   graph g;
   if (!g.is_connected())
       g.make_connected();

   g.make_symmetric();
   
   char tmp[L_tmpnam];
   tmpnam(tmp);
   g.dump_epanet(tmp);
   
   FILE *bench_file = fopen(BENCH_FILE, "a");
   fprintf(bench_file, "%d\t%d\t", n, k);
   fclose(bench_file);
   
   //prepare for epanet run
   QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();
   penv.insert("EN_BENCH_FILE", BENCH_FILE);
   QProcess p;
   p.setProcessEnvironment(penv);
   
   //run epanet
   QStringList args = {tmp, EN_OUT_FILE};
   p.setStandardOutputFile("/dev/stdout", QIODevice::Append);
   p.setStandardErrorFile("/dev/stderr", QIODevice::Append);
   p.start(epanet_exe, args);
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

    if (app.arguments().size() < 2 || !QFile::exists(app.arguments()[1])) {
        std::cout << "provide epanet binary file" << std::endl;
        exit(-1);
    }
    
    QString epanet_exe = app.arguments()[1];
    std::cout << "using " << epanet_exe.toStdString() << " as epanet exe" << std::endl;
    
    for (int n = 200; n <= 1800; n+= 200) {
       for (int k = 5; k <= 20; k+= 1) {
          run(n, k, epanet_exe);
       }
    }
    
    return 0;
}
