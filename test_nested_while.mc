int main() {
    int x = 0;
    int i = 0;
    int j = 0;

    while (i < 3) {
        j = 0;

        while (j < 2) {
            x = x + i + j;
            j = j + 1;
        }

        i = i + 1;
    }

    return x;
}
