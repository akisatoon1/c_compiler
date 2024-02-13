int main(){int x; int y; x=0; y=0; int *px; int *py; int **ppx; int **ppy; px=&x; ppx=&px; py=&y; ppy=&py; return ppx-ppy;}
