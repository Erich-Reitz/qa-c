// EXPECTED_RETURN: 5

int max(int a, int b) {
    if (a > b) {
        return a;
    }

    return b;
}

int main() {
    return max(3, 5);
}