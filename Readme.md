## Introduction
Imagine, you have several ranges and you want to iterate over each element defined by the rangers just one time.
The ranges may be not disjunct.

For this problem, the class 'range_union' is provided.
You can add non disjunct ranges to the union and request iterators which will iterate over all elements.

## Complexity
Inserting and deleting new ranges into range_union is very cost efficient.
Inserting costs at max one upper and one lower bound which means, only 'O(log2(n))' where n is the number of ranges inside the union.
Delete operations performs one 'equal_range' operation which is again only 'O(log2(n))' where n is the number of ranges inside the union.

Both operations performs also a 'erase' operation which complexity depends on the container which is used.
For this reason, the 'range_union' template provides a parameter for the container type.
Suggested containers are 'std::vector' and 'std::list'.

When you have more insert/delete operations, 'std::list' is your choice, but the iteration over all the elements costs more.
The container type 'std::vector' provides faster iteration, but the performance of the insert/delete operation is slower than 'std::list' container provides.

## Dependencies
- C++98


## Example
More examples can be found in the test files.

```C++
#include <numeric>
#include <vector>
#include <iostream>
#include <range_union.hpp>

using namespace tyti;

int main()
{
    using range_t = pair_range<std::vector<int>::iterator>;
    range_t range;
    range_union< range_t > rUnion;
    std::vector<int> numbers(25);
    std::iota(numbers.begin(), numbers.end(), 0);

    range.first = numbers.begin() + 5;
    range.second = numbers.begin() + 10;

    rUnion += range;
    for (const auto& obj : rUnion)
        std::cout << *obj << " ";
        //output: 5 6 7 8 9
    std::cout << std::endl;

    range.first = numbers.begin() + 7;
    range.second = numbers.begin() + 15;
    
    rUnion += range;
    for (const auto& obj : rUnion)
        std::cout << *obj << " ";
        //output: 5 6 7 8 9 10 11 12 13 14 
    std::cout << std::endl;

    range.first = numbers.begin() + 6;
    range.second = numbers.begin() + 12;

    rUnion -= range;
    for (const auto& obj : rUnion)
        std::cout << *obj << " ";
        //output: 5 12 13 14
    std::cout << std::endl;

    range.first = numbers.begin() + 3;
    range.second = numbers.begin() + 18;

    rUnion += range;
    for (const auto& obj : rUnion)
        std::cout << *obj << " ";
        //output: 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17
    std::cout << std::endl;
}

```

## License

[MIT License](./LICENSE) © Matthias Möller. Made with ♥ in Germany.
