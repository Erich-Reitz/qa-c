//10

int main() {
    int num = 10;
    int *ptr1 = &num;
    int **ptr2 = &ptr1;
    int ***ptr3 = &ptr2;
    int result = ***ptr3;
    return result;
}
