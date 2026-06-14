int prev = 0
int curr = 1
while (curr < 10) {
      curr = prev + curr
      prev = curr - prev
      print(curr)
}
