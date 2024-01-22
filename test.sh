#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 3 'int main(){ int x; x=3; int *y; y =&x; int **z; z=&y;  return **z; }'
assert 3 'int main(){ int x; int *y; y = &x; *y = 3; return x; }'

assert 3 'int main(){ int x; x=3; return *&x; }'
assert 5 'int main(){ int x; int y; x=3; y=5; return *(&x-8); }'
assert 3 'int main(){ int x; x=3; return *&x; }'
assert 3 'int main(){ int x; int y; int z; x=3; y=&x; z=&y; return **z; }'
assert 3 'int main(){ int x; int y; x=3; y=5; return *(&y+8); }'

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
