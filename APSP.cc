#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <omp.h>

#define MaxDistance 1073741823

class Timer {
  std::chrono::steady_clock::time_point t0;
public:
  Timer(): t0(std::chrono::steady_clock::now()) {}
  ~Timer() {
      std::chrono::duration<double> dt = std::chrono::steady_clock::now() - t0;
      printf("validation took %gs\n", dt.count());
  }
};

int main(int argc, char** argv) {
  Timer* t_total = new Timer;
  cpu_set_t cpu_set;
  sched_getaffinity(0, sizeof(cpu_set), &cpu_set);
  int thread_num = CPU_COUNT(&cpu_set);

  std::ifstream f(argv[1]);
  int i, j, k, h_start;
  int V, E;

  f.read((char*)&V, sizeof V);
  f.read((char*)&E, sizeof E);
  printf("V: %d, E: %d\n", V, E);

  int VV = V * V;
  int* Distance = (int *)malloc(sizeof(int) * VV);

  #pragma omp parallel for schedule(static, 10) num_threads(thread_num)
  for (i = 0; i < VV; i+=V) {
    for (j = i; j < i + V; j++) Distance[j] = MaxDistance;
    Distance[i+i/V] = 0;
  }
  
  int e[3];
  Timer* t_read = new Timer;
  for (i = 0; i < E; i++) {
    f.read((char*)e, sizeof e);
    Distance[e[0] * V + e[1]] = e[2];
  }
  printf("read: ");
  delete t_read;
  for (k = 0; k < VV; k+=V) {
    #pragma omp parallel for num_threads(thread_num)
    for (i = 0; i < VV; i+=V) {
      h_start = Distance[i + k/V];
      for (j = 0; j < V; j++)
        if (Distance[i + j] > h_start + Distance[k + j])
          Distance[i + j] = h_start + Distance[k + j];
    }
  }

  Timer* t_write = new Timer;
  std::ofstream f2(argv[2]);
  for (i = 0; i < VV; i++) f2.write((char*)&Distance[i], sizeof(int));
  printf("write: ");
  delete t_write;

  printf("total: ");
  delete t_total;
  return 0;
}