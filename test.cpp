#include <iostream>
#include <cstring>

using namespace std;

int main(int argc, char const *argv[])
{
    char *a = new char[1024];
    int b = -5;
    int c;
    memcpy(a, &b, sizeof(int));
    memcpy(&c, a, sizeof(int));
    cout << c << endl;
    return 0;
}
