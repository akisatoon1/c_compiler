#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 1 'int main(){char *x; x = "helloworld"; return 0;}'

assert 10 'char x[5]; char main(){x[0]=0;x[1]=1;x[2]=2;x[3]=3;x[4]=4;return x[0]+x[1]+x[2]+x[3]+x[4];}'
assert 6 'char main(){char x[4]; x[0]=0;x[1]=1;x[2]=2;x[3]=3;return x[0]+x[1]+x[2]+x[3];}'

assert 0 'int x; int main() { return x; }'
assert 7 'int x; int y; int main() { x=3; y=4; return x+y; }'
assert 0 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[0]; }'
assert 1 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[1]; }'
assert 2 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[2]; }'
assert 3 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[3]; }'
assert 4 'int x; int main() { return sizeof(x); }'
assert 16 'int x[4]; int main() { return sizeof(x); }'
assert 23 'int x; int y; int main(){x=12; y=11; return x+y;}'
assert 46 'int x; int main(){x=34; int x; x=46; return x;}'
assert 30 'int x; int main(){x=13; return x+17;}'

assert 4 'int main(){int a[2]; a[0]=1; a[1]=2; return sizeof (a[0]+a[1]);}'
assert 3 'int main(){int a[2]; a[0]=1; a[1]=2; return a[0]+a[1];}'
assert 20 'int main(){int a[5]; return sizeof(a);}'
assert 3 'int main(){int a[2]; *a=1; *(a+1)=2; int *p; p=a; return *p+*(p+1);}'

assert 4 'int main(){return sizeof(sizeof(1));}'
assert 8 'int main(){int *x; return sizeof (x+1);}'
assert 4 'int main(){int x; return sizeof (x+1);}'
assert 8 'int main(){int *x; return sizeof x;}'
assert 4 'int main(){int x; return sizeof x;}'
assert 4 'int main(){return sizeof 1;}'

assert 1 'int main(){int x; int y; x=0; y=0; int *px; int *py; int **ppx; int **ppy; px=&x; ppx=&px; py=&y; ppy=&py; return ppx-ppy;}'
assert 1 'int main(){int x; int y; int *px; int *py; px=&x; py=&y; return px-py;}'

assert 73 'int *add(int *x, int y){return x+y;} int main(){int a[5]; a[4]=73; int *p; p=add(a,4); return *p;}'
assert 32 'int *addr(int x){return &x;} int main(){int x; x=32; int *p; p=addr(x); return *p;}'
assert 7 'int ptr(int **ptr){ return **ptr;} int main(){int x; int *y; int **z; x=7; y=&x; z=&y; return ptr(z);}'
assert 5 'int ptr(int *ptr){ return *ptr;} int main(){int x; int *y; x=5; y=&x; return ptr(y);}'
assert 3 'int main(){ int x; x=3; int *y; y =&x; int **z; z=&y;  return **z; }'
assert 3 'int main(){ int x; int *y; y = &x; *y = 3; return x; }'

assert 0 'int main(){int x; int y; x=0; y=0; int *px; int *py; int **ppx; int **ppy; px=&x; ppx=&px; py=&y; ppy=&py; return **(ppx-1);}'
assert 3 'int main(){ int x; x=3; return *&x; }'
assert 5 'int main(){ int x; int y; x=3; y=5; return *(&x-1); }'
assert 3 'int main(){ int x; x=3; return *&x; }'
assert 3 'int main(){ int x; int *y; int **z; x=3; y=&x; z=&y; return **z; }'
assert 3 'int main(){ int x; int y; x=3; y=5; return *(&y+1); }'

assert 34 'int fib(int a,int b){int c; c=a+b; if(c>21){ return c;} return fib(b,c);} int main(){ return fib(1,1);}'
assert 5 'int add(int a,int b){ return a+b;} int main(){return add(2,3);}'
assert 10 'int foo(){return 5; } int main(){ return 5+foo();}'
assert 5 'int foo(){ return 5;} int main(){return foo();}'

assert 11 'int main(){int ans1; int ans2; int i; ans1=1; ans2=0; for (i=0;i<3;i=i+1) {ans1=ans1*2; ans2=ans2+1; } return ans1+ans2;}'
assert 0 'int main(){return 0;}'

assert 60 'int main(){int sum; int i; sum=0; for (i=10; i<15; i=i+1) sum = sum + i; return sum;}'
assert 15 'int main(){int ans; int i; ans=0;for(i=0;i<15;i=i+1)ans=ans+1;return ans;}'
assert 5 'int main(){int ans; int i; ans=0;for(i=0;i<5;i=i+1)ans=ans+1;return ans;}'

assert 6 'int main(){int i; i=0; while(i<5) i=i+2; return i;}'
assert 5 'int main(){int i; i=0;while(i<5)i=i+1;return i;}'

assert 2 'int main(){if (1) return 2; else return 3;}'
assert 3 'int main(){if (0) return 2; else return 3;}'
assert 1 'int main(){int foo; foo=0; if(0==0) foo=1; return foo;}'
assert 0 'int main(){int foo; foo=0; if(0!=0) foo=1; return foo;}'

assert 0 'int main(){return 0;}'
assert 42 'int main(){return 42;}'
assert 21 'int main(){return 5+20-4;}'
assert 41 'int main(){return  12 + 34 - 5 ;}'
assert 47 'int main(){return 5+6*7;}'
assert 15 'int main(){return 5*(9-6);}'
assert 4 'int main(){return (3+5)/2;}'
assert 10 'int main(){return -10+20;}'
assert 10 'int main(){return - (-10);}'
assert 10 'int main(){return - (- (+10));}'
assert 10 'int main(){return 10-20+20;}'
assert 10 'int main() { return - -10; }'
assert 10 'int main() { return - - +10; }'
assert 10 'int main() { return -10+20; }'

assert 0 'int main(){return 0==1;}'
assert 1 'int main(){return 42==42;}'
assert 1 'int main(){return 0!=1;}'
assert 0 'int main(){return 42!=42;}'

assert 1 'int main(){return 0<1;}'
assert 0 'int main(){return 1<1;}'
assert 0 'int main(){return 2<1;}'
assert 1 'int main(){return 0<=1;}'
assert 1 'int main(){return 1<=1;}'
assert 0 'int main(){return 2<=1;}'

assert 1 'int main(){return 1>0;}'
assert 0 'int main(){return 1>1;}'
assert 0 'int main(){return 1>2;}'
assert 1 'int main(){return 1>=0;}'
assert 1 'int main(){return 1>=1;}'
assert 0 'int main(){return 1>=2;}'

assert 3 'int main(){int a; a=3; return a;}'
assert 8 'int main(){int a; int z; a=3; z=5; return a+z;}'

assert 3 'int main(){int a;a=3; return a;}'
assert 8 'int main(){int a;int z;a=3; z=5; return a+z;}'
assert 6 'int main(){int a;int b;a=b=3; return a+b;}'
assert 3 'int main(){int foo;foo=3; return foo;}'
assert 8 'int main(){int foo123; int bar; foo123=3; bar=5; return foo123+bar;}'

assert 1 'int main(){return 1; 2; 3;}'
assert 2 'int main(){1; return 2; 3;}'
assert 3 'int main(){1; 2; return 3;}'
echo OK
