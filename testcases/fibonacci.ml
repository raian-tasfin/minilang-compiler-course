int n := 10
if n <= 0 {
    print("error: {n} must be positive")
}
elseif n <= 2 {
    print("1")
}
else {
    int a := 1
    int b := 1
    int c := 2
    while c <= n {
        int tmp := b
        b := a + b
        a := tmp
        c := c + 1
    }
    print("{b}")
}
