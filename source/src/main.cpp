

#include <glm/glm.hpp>
#include <iostream>

#include "camera.hpp"
#include "film.hpp"
#include "glm/geometric.hpp"
#include "sphere.hpp"
#include "threadpool.hpp"

using namespace std;

class SimpleTask : public Task {
public:
  virtual void run() override { cout << "Hello World!" << endl; }
};

int main() {
  ThreadPool pool{};
  Film film(800, 450);

  pool.parallelFor(
      200, 100, [&](auto x, auto y) { film.setPixel(x, y, {0.6, 0.7, 0.2}); });

  pool.wait();

  film.save("test.ppm");

  return 0;
}