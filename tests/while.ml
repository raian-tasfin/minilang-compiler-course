int prev = 0
int curr = 1
while (curr < 10) {
    int tmp = curr
    curr = curr + prev
    prev = tmp
    print(curr)
}
