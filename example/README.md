# Example

This example can be generated with (Fedora 36, clang 14.0.0):

```bash
solgen --input Vehicle.h Submarine.h Car.h SportCar.h --output-dir solgen --regenerate --namespace-filter example --type-filter example::.+ --conf solgen.conf --clang-args -I /usr/lib64/clang/14.0.0/include -std=c++17
```
