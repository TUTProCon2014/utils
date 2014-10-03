
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>

#include "../../utils/include/image.hpp"
#include "../../utils/include/dwrite.hpp"
#include "../../utils/include/exception.hpp"


using namespace procon::utils;


struct SimulatedImage
{
    SimulatedImage(SwappedImage const & img)
    : swpImage(img), _sIdx({0, 0})
    {}


    void select(size_t i, size_t j)
    {
        _sIdx[0] = i;
        _sIdx[1] = j;
    }


    void evaluate(char c)
    {
        Index2D aIdx;       // 入れ替えるもう一つの断片のIdx
        
        switch(c)
        {
            case 'R':
                PROCON_ENFORCE(_sIdx[1] != swpImage.div_x() - 1, "Simulation Error");
                aIdx = _sIdx;
                aIdx[1] += 1;
                break;

            case 'L':
                PROCON_ENFORCE(_sIdx[1] != 0, "Simulation Error");
                aIdx = _sIdx;
                aIdx[1] -= 1;
                break;

            case 'U':
                PROCON_ENFORCE(_sIdx[0] != 0, "Simulation Error");
                aIdx = _sIdx;
                aIdx[0] -= 1;
                break;

            case 'D':
                PROCON_ENFORCE(_sIdx[0] != swpImage.div_y() - 1, "Simulation Error");
                aIdx = _sIdx;
                aIdx[0] += 1;
                break;

          default:
            PROCON_ENFORCE(0, format("switch error. %", c));
        }
        
        swpImage.swap_element(_sIdx, aIdx);
        _sIdx = aIdx;
    }


    cv::Mat cvMat() const
    {
        auto dup = swpImage.clone();

        DividedImage::foreach(swpImage, [&](size_t i, size_t j){
            if(_sIdx[0] == i && _sIdx[1] == j){
                dup.get_element(i, j).cvMat() *= 0.5;
                dup.get_element(i, j).cvMat() += cv::Scalar(0, 0, 255) * 0.5;
            }
        });

        return dup.cvMat();
    }


    SwappedImage swpImage;
    Index2D _sIdx;      // 現在選択中の座標
};


template <typename T, typename S>
T readFrom(S& stream)
{
    T t;
    stream >> t;
    return t;
}


int main()
{
    const std::string windowName = "ご注文はシミュレータですか";


    write("Path(problem image .ppm) ----- ");
    auto p_opt = Problem::get(readFrom<std::string>(std::cin));

    PROCON_ENFORCE(p_opt, "cannot open image.");


    auto& pb = *p_opt;

    std::vector<std::vector<ImageID>> idxs(pb.div_y());
    DividedImage::foreach(pb, [&](size_t i, size_t j){
        if(j == 0) idxs.emplace_back();
        idxs[i].emplace_back(i, j);
    });

    auto simImage = SimulatedImage(SwappedImage(pb.dividedImage(), idxs));
    cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);
    cv::imshow(windowName, pb.cvMat());

    writeln("----- Please put answer -----");

    size_t selectCNT;
    std::cin >> selectCNT;

    for(size_t i = 0; i < selectCNT; ++i){
        size_t idxNum;
        std::cin >> std::hex >> idxNum;
        simImage.select(idxNum & 0xF, (idxNum >> 4) & 0xF);
        cv::imshow(windowName, simImage.cvMat());
        cv::waitKey(pb.select_cost() * 1000 / 100);

        ((void)readFrom<int>(std::cin));

        std::string line;
        std::cin >> line;
        for(char c: line){
            simImage.evaluate(c);
            cv::imshow(windowName, simImage.cvMat());
            cv::waitKey(pb.change_cost() * 1000 / 100);
        }
    }

    // キー入力を（無限に）待つ
    cv::waitKey(0);

    return 0;
}
