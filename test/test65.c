int ptr(int **ptr){ return **ptr;} int main(){int x; int *y; int **z; x=7; y=&x; z=&y; return ptr(z);}
