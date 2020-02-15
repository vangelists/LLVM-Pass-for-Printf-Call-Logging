// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2020 Vangelis Tsiatsianas

#include <functional>
#include <vector>

#include <cstdio>
#include <cstdlib>

//----------------------------------------------------------------------------------------------------------------------

using RandomNumberGenerator = std::function<std::size_t(void)>;

constexpr auto randomMod2048 = []() {
    return std::rand() % 2048;
};

//----------------------------------------------------------------------------------------------------------------------

struct TypeException : public std::exception {
    TypeException() : std::exception() {
        printf("Type exception thrown!\n");
    }
};

//----------------------------------------------------------------------------------------------------------------------

int foo() {
    printf("Hello, world!\n");
    return 5;
}

std::size_t bar(RandomNumberGenerator randomNumberGenerator) {
    static std::size_t depth = 0;
    const auto randomNumber = randomNumberGenerator();
    printf("Random number #%zu: %zu\n", depth, randomNumber);
    if (depth == 16) {
        return 0;
    } else {
        ++depth;
        return bar(randomNumberGenerator);
    }
}

//----------------------------------------------------------------------------------------------------------------------

int main() {
    int number = 5;
    float numberFollowing = 0.0;

    [&](int x) {
        const auto stackVariableDistance = (uintptr_t)&number - (uintptr_t)&numberFollowing;
        printf("%d, %d, 0x%lx\n", foo(), number, stackVariableDistance);
    }(2);

    {
        printf("%d\n", foo());
        try {
            if (bar(randomMod2048) != 0xDEADBEEF)
                throw TypeException();
        } catch (...) {}
    }

    std::vector<std::size_t> numVector(bar(randomMod2048), bar(randomMod2048));
    for (const auto num : numVector) {
        if (num > 50 && num < 99) {
            printf("Between 50 and 99: %zu\n", num);
            return 100;
        } else if (num > 1024) {
            printf("Over 1024: %zu\n", num);
            break;
        }
    }

    [](float y) {
        printf("%f\n", y);
    }(3.14);

    return number;
}
