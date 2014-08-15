#pragma once

#include "template.hpp"

namespace procon { namespace utils{


PROCON_DEF_TYPE_TRAIT(is_image, true,
(
    identity<size_t>(p->height()),
    identity<size_t>(p->width()),
    identity(p->get_pixel(0u, 0u))
));


/** pixelを表す型
*/
struct Pixel
{
  public:
    Pixel(cv::Vec3b const & v) : _v(v) {}

    uint8_t b() const { return _v[0]; }
    uint8_t g() const { return _v[1]; }
    uint8_t r() const { return _v[2]; }

    cv::Vec3b vec() const { return _v; }

  private:
    cv::Vec3b _v;
};


// 
struct ProblemImpl
{
  public:
    ProblemImpl(cv::Mat img) : _img(img) {}

    std::size_t height() const { return _img.rows; }
    std::size_t width() const { return _img.cols; }
    Pixel get_pixel(size_t y, size_t x) const { return Pixel(_img.at<cv::Vec3b>(y, x)); }

    cv::Mat & cvMat() { return _img; }
    cv::Mat const & cvMat() const { return _img; }

  private:
    cv::Mat _img;
};


/**
全体画像の分割画像を表すクラスです。
*/
class ElementImage
{
  public:
    ElementImage(std::shared_ptr<ProblemImpl> const & m, std::size_t r, std::size_t c, std::size_t div_x, std::size_t div_y)
    : _master(m), _pos_x(c), _pos_y(r), _div_x(div_x), _div_y(div_y)
    {}


    /// 分割画像の高さを返します
    std::size_t height() const
    {
        return _master->height() / _div_y;
    }


    /// 分割画像の幅を返します
    std::size_t width() const
    {
        return _master->width() / _div_x;
    }


    /// 分割画像の(i, j)に位置するピクセル値を返します
    Pixel get_pixel(std::size_t y, std::size_t x) const
    {
        return _master->get_pixel(y + _pos_y * height(), x + _pos_x * width());
    }


  private:
    const std::shared_ptr<ProblemImpl> _master;
    std::size_t _pos_x; // (N, M)に画像が分割されていたとき、(i, j)位置の画像を示すなら i
    std::size_t _pos_y; // (N, M)に画像が分割されていたとき、(i, j)位置の画像を示すなら j]
    std::size_t _div_x;
    std::size_t _div_y;
};


/** 問題の各種定数と画像を管理する型です。
*/
class Problem
{
  public:
    /** ローカルに保存してあるppmファイルを読み込みます。
    * arguments:
        * ppm_file_path     = ローカルに保存したppmファイルへのパス
    
    * return:
        読み込みに成功した場合はオブジェクトが返りますが、失敗した場合にはnull_opt()が返ります。
    */
    static
    boost::optional<Problem> get(std::string const & ppm_file_path)
    {
        auto null_opt = [&](){ return boost::optional<Problem>(boost::none); };

        cv::Mat img = cv::imread(ppm_file_path);
        if(img.empty()){
            std::cout << "can't open " << ppm_file_path << std::endl;
            return null_opt();
        }

        Problem pb;
        pb._master = std::make_shared<ProblemImpl>(img);

        std::ifstream file(ppm_file_path);
        if(file.fail()){
            std::cout << "can't open " << ppm_file_path << std::endl;
            return null_opt();
        }

        std::string line;
        getline(file, line);    // P6
        
        char c;

        getline(file, line);    // 分割数
        std::stringstream ss1(line);
        ss1 >> c >> pb._div_x >> pb._div_y;
        
        getline(file, line);    // 最大選択可能回数
        std::stringstream ss2(line);
        ss2 >> c >> pb._max_select_times;
        
        getline(file, line);    // コスト変換レート
        std::stringstream ss3(line);
        ss3 >> c >> pb._select_cost >> pb._change_cost;

        return boost::optional<Problem>(pb);
    }


    /// 問題画像の高さを返します
    std::size_t height() const { return _master->height(); }


    /// 問題画像の幅を返します
    std::size_t width() const { return _master->width(); }


    /// (w, h) = (i, j) ピクセル目のピクセル値を返します
    Pixel get_pixel(std::size_t y, std::size_t x) const { return _master->get_pixel(y, x); }


    /// 横方向の画像分割数を返します
    std::size_t div_x() const { return _div_x; }
    

    /// 縦方向の画像分割数を返します
    std::size_t div_y() const { return _div_y; }


    /// インデックス配列におけるi行j列の断片を表すオブジェクトを返します
    ElementImage get_element(std::size_t r, std::size_t c) const
    {
        ElementImage dst(_master, r, c, _div_x, _div_y);
        return dst;
    }


    /// 交換レートを返します
    int change_cost() const { return _change_cost; }
    

    /// 選択レートを返します
    int select_cost() const { return _select_cost; }


    /// 最大選択可能回数
    std::size_t max_select_times() const { return _max_select_times; }

    cv::Mat & cvMat() { return _master->cvMat(); }
    cv::Mat const & cvMat() const { return _master->cvMat(); }


  private:
    std::shared_ptr<ProblemImpl> _master;
    std::size_t _div_x;
    std::size_t _div_y;
    int _change_cost;
    int _select_cost;
    std::size_t _max_select_times;
};

}} // namespace procon::utils