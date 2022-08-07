#include <list>
#include <gtest/gtest.h>
#include "myframe/MyFlags.h"
#include "myframe/MyApp.h"

TEST(MyApp, list) {
    std::list<int> l1 = {1, 3, 5, 7};
    std::list<int> l2 = {2, 4, 6, 8};

    l1.insert(l1.end(), l2.begin(), l2.end());
    for (const auto& it : l1) {
        std::cout << it << std::endl;
    }
    for (const auto& it : l2) {
        std::cout << it << std::endl;
    }
}