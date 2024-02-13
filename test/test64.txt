int ptr(int *ptr){ return *ptr;} int main(){int x; int *y; x=5; y=&x; return ptr(y);}
