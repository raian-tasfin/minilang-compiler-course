int prev = 0
int curr = 1
int cnt = 1
while (cnt < 30) {
    int tmp = curr
    curr = curr + prev
    prev = tmp
    print(curr)
    cnt = cnt + 1
}
