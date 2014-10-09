#include <algorithm>
#include <random>
#include <vector>
#include <iomanip>


#include "../../utils/include/dwrite.hpp"
#include "../../utils/include/image.hpp"
#include "../../calc_exchange/include/greedy_calc_exchange.hpp"

using namespace procon;

int main()
{
    std::random_device seed_gen;
    std::mt19937 engine(seed_gen());
    std::uniform_int_distribution<size_t> dist(2, 16);
    std::size_t cnt = 0;

    while(1)
    {
        ++cnt;
        utils::writefln("The% %th...", std::dec, cnt);
        const size_t rows = dist(engine);
        const size_t cols = dist(engine);
        std::vector<std::vector<utils::ImageID>> idxs;
        idxs.reserve(rows);

        for(size_t i = 0; i < rows; ++i){
            idxs.emplace_back();

            auto& last = idxs[idxs.size()-1];
            last.reserve(cols);
            for(size_t j = 0; j < cols; ++j)
                last.emplace_back(i, j);
        }


        std::shuffle(idxs.begin(), idxs.end(), engine);
        for(auto& e: idxs)
            std::shuffle(e.begin(), e.end(), engine);

        utils::writeln(idxs);
        greedy_calc_exchange::greedy_calc_exchange(idxs, 3, 3, 2);
    }
}
