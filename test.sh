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

#assert 5 'foo(){ return 5;} main(){return foo();}'

assert 11 'main(){ans1=1; ans2=0; for (i=0;i<3;i=i+1) {ans1=ans1*2; ans2=ans2+1; } return ans1+ans2;}'
assert 0 'main(){return 0;}'

assert 60 'main(){sum=0; for (i=10; i<15; i=i+1) sum = sum + i; return sum;}'
assert 15 'main(){ans=0;for(i=0;i<15;i=i+1)ans=ans+1;return ans;}'
assert 5 'main(){ans=0;for(i=0;i<5;i=i+1)ans=ans+1;return ans;}'

assert 6 'main(){i=0; while(i<5) i=i+2; return i;}'
assert 5 'main(){i=0;while(i<5)i=i+1;return i;}'

assert 2 'main(){if (1) return 2; else return 3;}'
assert 3 'main(){if (0) return 2; else return 3;}'
assert 1 'main(){foo=0; if(0==0) foo=1; return foo;}'
assert 0 'main(){foo=0; if(0!=0) foo=1; return foo;}'

assert 0 'main(){return 0;}'
assert 42 'main(){return 42;}'
assert 21 'main(){return 5+20-4;}'
assert 41 'main(){return  12 + 34 - 5 ;}'
assert 47 'main(){return 5+6*7;}'
assert 15 'main(){return 5*(9-6);}'
assert 4 'main(){return (3+5)/2;}'
assert 10 'main(){return -10+20;}'
assert 10 'main(){return - (-10);}'
assert 10 'main(){return - (- (+10));}'

assert 0 'main(){return 0==1;}'
assert 1 'main(){return 42==42;}'
assert 1 'main(){return 0!=1;}'
assert 0 'main(){return 42!=42;}'

assert 1 'main(){return 0<1;}'
assert 0 'main(){return 1<1;}'
assert 0 'main(){return 2<1;}'
assert 1 'main(){return 0<=1;}'
assert 1 'main(){return 1<=1;}'
assert 0 'main(){return 2<=1;}'

assert 1 'main(){return 1>0;}'
assert 0 'main(){return 1>1;}'
assert 0 'main(){return 1>2;}'
assert 1 'main(){return 1>=0;}'
assert 1 'main(){return 1>=1;}'
assert 0 'main(){return 1>=2;}'

assert 3 'main(){a=3; return a;}'
assert 8 'main(){a=3; z=5; return a+z;}'

assert 3 'main(){a=3; return a;}'
assert 8 'main(){a=3; z=5; return a+z;}'
assert 6 'main(){a=b=3; return a+b;}'
assert 3 'main(){foo=3; return foo;}'
assert 8 'main(){foo123=3; bar=5; return foo123+bar;}'

assert 1 'main(){return 1; 2; 3;}'
assert 2 'main(){1; return 2; 3;}'
assert 3 'main(){1; 2; return 3;}'
echo OK
