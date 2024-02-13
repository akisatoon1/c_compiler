int *addr(int x){return &x;} int main(){int x; x=32; int *p; p=addr(x); return *p;}
