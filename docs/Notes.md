# Notes


## CTAD

Class Template Argument Deducation - allows you to write a class template without spelling out its type arguments.

```
std::vector<int> v = {1, 2, 3};   // old: explicit <int>
std::vector        w = {1, 2, 3};  // CTAD (C++17): compiler deduces vector<int>
std::pair          p{1, 2.0};      // deduces pair<int, double>
```


## String Argument Handling
std::string_view for 0 copy reads of an already allocated string.
std::string for reads of a newly allocated string.


## ^^ Reflection Operator

Lifts program entity into a value you can manipulate at compile time (i.e., result of std::meta::info)

## typename

Tell the compiler we are using the assignment as a template parameter.
