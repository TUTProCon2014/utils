#pragma once

#include <utility>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <type_traits>
#include <cstdint>
#include <memory>
#include <fstream>
#include <boost/optional.hpp>
#include <algorithm>
#include "constants.hpp"
#include "template.hpp"
#include "types.hpp"
#include "range.hpp"
#include "dwrite.hpp"

namespace procon { namespace utils {


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


#ifndef NOT_SUPPORT_CONSTEXPR
PROCON_DEF_TYPE_TRAIT(is_image, true,
(
    identity<size_t>(p->height()),
    identity<size_t>(p->width()),
    identity<Pixel>(p->get_pixel(0u, 0u)),
    identity<typename std::remove_const<T>::type>(p->clone())
));
#endif


#ifndef NOT_SUPPORT_CONSTEXPR
PROCON_DEF_TYPE_TRAIT(has_cv_image, true,
(
    identity(p->cvMat())
));
#endif


#ifndef NOT_SUPPORT_CONSTEXPR
PROCON_DEF_TYPE_TRAIT(is_divided_image, is_image,
(
    identity<size_t>(p->div_x()),
    identity<size_t>(p->div_y()),
    identity(p->get_element(0u, 0u))
));
#endif



template <typename T> constexpr bool isCVMat(){ return std::is_same<cv::Mat, T>::value; }
template <typename T> constexpr bool isConstCVMat(){ return std::is_same<const cv::Mat, T>::value; }


class Image
{
  public:
    template <typename T>
    static constexpr bool is_same()
    { return std::is_same<T, Image>::value; }


	template <typename T>
	static constexpr bool isConstructibleCVMat()
	{ return std::is_constructible<cv::Mat, T>::value; }

  public:
    
#ifdef SUPPORT_TEMPLATE_CONSTRAINTS
    template <typename U
    , PROCON_TEMPLATE_CONSTRAINTS(isConstructibleCVMat<U>())
    >
    explicit Image(U && img)
    : _img(std::forward<U>(img)) {}
#else
    // disable forwarding to move-ctor
    explicit Image(cv::Mat img)
    : _img(img) {}
#endif

    std::size_t height() const { return _img.rows; }
    std::size_t width() const { return _img.cols; }
    Pixel get_pixel(size_t y, size_t x) const { return Pixel(_img.at<cv::Vec3b>(y, x)); }

    cv::Mat & cvMat() { return _img; }
    cv::Mat const & cvMat() const { return _img; }

    Image clone() const
    {
        Image dst(_img.clone());
        return dst;
    }


  private:
    cv::Mat _img;
};


template <typename CVMat
#ifdef SUPPORT_TEMPLATE_CONSTRAINTS
    , PROCON_TEMPLATE_CONSTRAINTS(isCVMat<std::remove_reference<CVMat>::type>())
#endif
>
Image makeImage(CVMat && img)
{
    return Image(std::forward<CVMat>(img));
}



template <typename CVMat
#ifdef SUPPORT_TEMPLATE_CONSTRAINTS
    , PROCON_TEMPLATE_CONSTRAINTS(isCVMat<std::remove_reference<CVMat>::type>())
#endif
>
const Image makeImage(const CVMat && img)
{
    const Image dst(std::forward<CVMat>(img));
    return dst;
}


class DividedImage
{
  public:
    template <typename T>
    static constexpr bool is_same()
    { return std::is_same<DividedImage, T>::value; }

    template <typename T>
    static constexpr bool isConstructibleImage()
    { return std::is_constructible<Image, T>::value; }


  public:
    template <typename T
#ifdef SUPPORT_TEMPLATE_CONSTRAINTS
        , PROCON_TEMPLATE_CONSTRAINTS(isConstructibleImage<T>())
#endif
    >
    DividedImage(T && m, std::size_t div_x, std::size_t div_y)
    : _master(std::forward<T>(m)), _div_x(div_x), _div_y(div_y)
    {}


    /// 問題画像の高さを返します
    std::size_t height() const { return _master.height(); }


    /// 問題画像の幅を返します
    std::size_t width() const { return _master.width(); }


    /// (w, h) = (i, j) ピクセル目のピクセル値を返します
    Pixel get_pixel(std::size_t y, std::size_t x) const { return _master.get_pixel(y, x); }


    /// 横方向の画像分割数を返します
    std::size_t div_x() const { return _div_x; }
    

    /// 縦方向の画像分割数を返します
    std::size_t div_y() const { return _div_y; }


    /// インデックス配列におけるi行j列の断片を表すオブジェクトを返します
    Image get_element(std::size_t r, std::size_t c)
    {
        const auto ww = width() / div_x(),
                   hh = height() / div_y();

        auto mat = cvMat()(cv::Rect(c * ww, r * hh, ww, hh));
        Image dst(mat);
        return dst;
    }


    const Image get_element(std::size_t r, std::size_t c) const
    {
        const auto ww = width() / div_x(),
                   hh = height() / div_y();

        auto mat = cvMat()(cv::Rect(c * ww, r * hh, ww, hh));
        const Image dst(mat);
        return dst;
    }


    cv::Mat & cvMat() { return _master.cvMat(); }
    cv::Mat const & cvMat() const { return _master.cvMat(); }


    DividedImage clone() const
    {
        auto dst = _master.clone();
        return DividedImage(dst, _div_x, _div_y);
    }


  private:
    Image _master;
    std::size_t _div_x;
    std::size_t _div_y;
};


template <typename T
#ifdef SUPPORT_TEMPLATE_CONSTRAINTS
    , PROCON_TEMPLATE_CONSTRAINTS(Image::is_same<T>())
#endif
>
DividedImage makeDividedImage(T && img, std::size_t div_x, std::size_t div_y)
{
    return DividedImage(std::forward<T>(img), div_x, div_y);
}


template <typename T
#ifdef SUPPORT_TEMPLATE_CONSTRAINTS
    , PROCON_TEMPLATE_CONSTRAINTS(Image::is_same<T>())
#endif
>
const DividedImage makeDividedImage(const T && img, std::size_t div_x, std::size_t div_y)
{
    const DividedImage dst(std::forward<T>(img), div_x, div_y);
    return dst;
}



/** 問題の各種定数と画像を管理する型です。
*/
class Problem
{
    template <typename T>
    static constexpr bool isConstructibleImage()
    { return std::is_constructible<Image, T>::value; }

    template <typename T>
    static constexpr bool isConstructibleDividedImage()
    { return std::is_constructible<DividedImage, T>::value; }


  public:
#ifdef SUPPORT_TEMPLATE_CONSTRAINTS
    template <typename T,
        PROCON_TEMPLATE_CONSTRAINTS(isConstructibleImage<T>())
    >
    Problem(T && m, std::size_t div_x, std::size_t div_y, int change_cost, int select_cost, std::size_t max_select_times)
    : _master(Image(std::forward<T>(m)), div_x, div_y), _change_cost(change_cost), _select_cost(select_cost), _max_select_times(max_select_times)
    {}

    template <typename T,
        PROCON_TEMPLATE_CONSTRAINTS(isConstructibleDividedImage<T>())
    >
    Problem(T && m, int change_cost, int select_cost, std::size_t max_select_times)
    : _master(std::forward<T>(m)), _change_cost(change_cost), _select_cost(select_cost), _max_select_times(max_select_times)
    {}

#else
    // disable forwarding to move ctor
    Problem(Image m, std::size_t div_x, std::size_t div_y, int change_cost, int select_cost, std::size_t max_select_times)
    : _master(m, div_x, div_y), _change_cost(change_cost), _select_cost(select_cost), _max_select_times(max_select_times)
    {}

    Problem(DividedImage m, int change_cost, int select_cost, std::size_t max_select_times)
    : _master(m), _change_cost(change_cost), _select_cost(select_cost), _max_select_times(max_select_times)
    {}

#endif

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

        Image image(img);

        std::ifstream file(ppm_file_path);
        if(file.fail()){
            std::cout << "can't open " << ppm_file_path << std::endl;
            return null_opt();
        }

        std::string line;
        getline(file, line);    // P6
        
        char c;

        std::size_t div_x, div_y;
        getline(file, line);    // 分割数
        std::stringstream ss1(line);
        ss1 >> c >> div_x >> div_y;

        std::size_t max_select_times;
        getline(file, line);    // 最大選択可能回数
        std::stringstream ss2(line);
        ss2 >> c >> max_select_times;

        int select_cost, change_cost;
        getline(file, line);    // コスト変換レート
        std::stringstream ss3(line);
        // ss1 = std::stringstream(line);
        ss3 >> c >> select_cost >> change_cost;

        Problem dst(image, div_x, div_y, change_cost, select_cost, max_select_times);
        return boost::optional<Problem>(std::move(dst));
    }


    /// 問題画像の高さを返します
    std::size_t height() const { return _master.height(); }


    /// 問題画像の幅を返します
    std::size_t width() const { return _master.width(); }


    /// (w, h) = (i, j) ピクセル目のピクセル値を返します
    Pixel get_pixel(std::size_t y, std::size_t x) const { return _master.get_pixel(y, x); }


    /// 横方向の画像分割数を返します
    std::size_t div_x() const { return _master.div_x(); }
    

    /// 縦方向の画像分割数を返します
    std::size_t div_y() const { return _master.div_y(); }


    /// インデックス配列におけるi行j列の断片を表すオブジェクトを返します
    Image get_element(std::size_t r, std::size_t c) { return _master.get_element(r, c); }
    const Image get_element(std::size_t r, std::size_t c) const { return _master.get_element(r, c); }


    /// 交換レートを返します
    int change_cost() const { return _change_cost; }
    

    /// 選択レートを返します
    int select_cost() const { return _select_cost; }


    /// 最大選択可能回数
    std::size_t max_select_times() const { return _max_select_times; }

    cv::Mat & cvMat() { return _master.cvMat(); }
    cv::Mat const & cvMat() const { return _master.cvMat(); }


    Problem clone() const
    {
        auto dst = _master.clone();
        return Problem(dst, _change_cost, _select_cost, _max_select_times);
    }


    DividedImage dividedImage() { return _master; }
    const DividedImage dividedImage() const { return _master; }


  private:
    DividedImage _master;
    int _change_cost;
    int _select_cost;
    std::size_t _max_select_times;
};


/**

*/
class SwappedImage
{
    template <typename T>
    static constexpr bool isConstructibleDividedImage()
    { return std::is_constructible<DividedImage, T>::value; }

    template <typename T
#ifdef SUPPORT_TEMPLATE_CONSTRAINTS
        , PROCON_TEMPLATE_CONSTRAINTS(isConstructibleDividedImage<T>())
#endif
    >
    SwappedImage(T && master, std::vector<std::vector<Index2D>> const & idx)
    : _master(std::forward<T>(master)), _idx(idx)
    {}


    void swap_element(utils::Index2D a, utils::Index2D b)
    {
        std::swap(_idx[a[0]][a[1]], _idx[b[0]][b[1]]);
    }


    std::vector<std::vector<Index2D>> const & get_index() const
    {
        return _idx;
    }


    cv::Mat cvMat() const
    {
        auto cln = _master.clone();

        for(auto i: utils::iota(div_y()))
            for(auto j: utils::iota(div_x()))
                get_element(i, j).cvMat().copyTo(cln.get_element(i, j).cvMat());

        return cln.cvMat();
    }


    size_t height() const { return _master.height(); }
    size_t width() const { return _master.width(); }
    size_t div_x() const { return _master.div_x(); }
    size_t div_y() const { return _master.div_y(); }
    Pixel get_pixel(std::size_t r, std::size_t c) const { return _master.get_pixel(r, c); }

    Image get_element(std::size_t r, std::size_t c)
    {
        return _master.get_element(r, c);
    }


    const Image get_element(std::size_t r, std::size_t c) const
    {
        return _master.get_element(r, c);
    }


    SwappedImage clone() const
    {
        SwappedImage dst(_master.clone(), _idx);
        return dst;
    }


  private:
    DividedImage _master;
    std::vector<std::vector<Index2D>> _idx;
};

}} // namespace procon::utils