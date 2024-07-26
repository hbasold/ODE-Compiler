# ODE-FPAAConfig Compiler 
A compiler which is able to convert input systems of Ordinary Differential Equations into a general FPAA configuration. This configuration can then serve as an input to a hybrid analog-digital computer.

The compiler can also iteratively digitally simulate the system of differential equations.

## Running the compiler
After running `make` the program can be ran by `./compiler`

## Program options
`./compiler {-n|-s} {-k} {-i} {-o} {-d} [filename.ode]`
`-n` - no interval scaling is performed
`-s` - variable shifting and scaling is performed
`-k` - compare and clustering is performed on the expressions inside a system
`-i` - digitally simulate the read systems
`-o` - output the read system into an FPAA configuration
`-d` - print debug information to the terminal

## Input ODE format
The systems of ODEs are of the following general form
```
system {
    var x_1 = <constant>;
    var x_2 = integ(<expr>, <init>);
    emit x_1 as g_1;
    interval x_1 = [<lower>, <upper>];
    interval x_2 = [<lower>, <upper>];
    time <float>;
}
```

## Output FPAA Configuration format
The output format is generated using the following grammar
```
FPAASystem ::= {<FPAAInput>} 
            |  {<FPAAInput>} <FPAASystem>
FPAAInput  ::=  <var> <FPAAInput>
            |   <var> <CABConfig>
            |   <int> <FPAAInput>
            |   <int> <CABConfig>
CABConfig  ::=  <CABConfig> <CABConfig>  
            |   <CABConfig> <FPAAOutput>
            |   <op> <inp> <inp> <scale>
            |   <op> <inp> <scale>
FPAAOutput ::=  <var> <FPAAOutput>
            |   <var>
op    ::= <sum> | <min> | <mul> | <div> | <sin> | <cos> | <integ>
inp   ::= <FPAAInput>
scale ::= <int>
var   ::= <var> <lett> | <lett>
int   ::= <int> <num> | <int>.<int> | <num>
lett  &in; {'a', ... , 'z', '_'}
num   &in; {0, ... , 9}
```

## Digital simulator
The read systems of ODEs can be iteratively simulated by using the command line flag `-i`, the systems are then simulated using the boost library's ODEInt simulator.
