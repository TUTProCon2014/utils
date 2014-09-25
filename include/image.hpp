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
PROCON_DEF_TYPE_TRAIT(is_divided_image, is_image<T>(),
(
    identity<size_t>(p->div_x()),
    identity<size_t>(p->div_y()),
    identity(p->get_element(0u, 0u))
));
#endif


struct ImageID
{
    ImageID() : ImageID(0, 0) {}

    explicit ImageID(utils::Index2D idx)
    : ImageID(idx[0], idx[1]) {}


    ImageID(size_t r, size_t c)
    {
        _val[0] = r & 0xFF;
        _val[1] = c & 0xFF;
    }


    template <typename DivImgType>
    auto get_image(DivImgType & img) const
    -> std::enable_if_t<is_divided_image<DivImgType>(), decltype(img.get_element(0, 0))>
    {
        return img.get_element(_val[0] , _val[1]);
    }


    auto get_index() const
    {
        return utils::makeIndex2D(_val[0], _val[1]);
    }


    bool operator==(ImageID const & other) const
    {
        return this->_val == other._val;
    }


    bool operator!=(ImageID const & other) const
    {
        return !(*this == other);
    }


    int opCmp(ImageID const & other) const
    {
        const auto v1 = *reinterpret_cast<uint16_t const *>(&(_val[0]));
        const auto v2 = *reinterpret_cast<uint16_t const *>(&(other._val[0]));

        if(v1 < v2)
            return -1;
        else if(v1 == v2)
            return 0;
        else
            return 1;
    }


    bool operator>=(ImageID const & other) const
    {
        return this->opCmp(other) >= 0;
    }


    bool operator>(ImageID const & other) const
    {
        return this->opCmp(other) > 0;
    }


    bool operator<(ImageID const & other) const
    {
        return this->opCmp(other) < 0;
    }


    bool operator<=(ImageID const & other) const
    {
        return this->opCmp(other) <= 0;
    }


    size_t get_hash() const
    {
        return std::hash<uint16_t>()((static_cast<uint16_t>(_val[0]) << 8) | _val[1]);
    }


    void to_string(std::ostream& s)
    {
        s << "(" << _val[0] << ", " << _val[1] << ")";
    }


  private:
    uint8_t _val[2];     // 8bit*2
};


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


template <typename CVMat>
std::enable_if_t<isCVMat<std::remove_reference<CVMat>::type>(),
Image> makeImage(CVMat && img)
{
    return Image(std::forward<CVMat>(img));
}



template <typename CVMat>
std::enable_if_t<isCVMat<std::remove_reference<CVMat>::type>(),
const Image> makeImage(const CVMat && img)
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


    Image get_element(ImageID id) { return id.get_image(*this); }


    const Image get_element(std::size_t r, std::size_t c) const
    {
        const auto ww = width() / div_x(),
                   hh = height() / div_y();

        auto mat = cvMat()(cv::Rect(c * ww, r * hh, ww, hh));
        const Image dst(mat);
        return dst;
    }

    
    const Image get_element(ImageID id) const { return id.get_image(*this); }


    cv::Mat & cvMat() { return _master.cvMat(); }
    cv::Mat const & cvMat() const { return _master.cvMat(); }


    DividedImage clone() const
    {
        auto dst = _master.clone();
        return DividedImage(dst, _div_x, _div_y);
    }


    template <typename T, typename F>
    static std::enable_if_t<is_divided_image<T>(),
    void> foreach(T const & pb, F f)
    {
        for(auto i: iota(0, pb.div_y()))
            for(auto j: iota(0, pb.div_x()))
                f(i, j);
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
    Image get_element(ImageID id) { return id.get_image(*this); }
    const Image get_element(std::size_t r, std::size_t c) const { return _master.get_element(r, c); }
    const Image get_element(ImageID id) const { return id.get_image(*this); }


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
  public:
    template <typename T>
    static constexpr bool isConstructibleDividedImage()
    { return std::is_constructible<DividedImage, T>::value; }

    template <typename T
#ifdef SUPPORT_TEMPLATE_CONSTRAINTS
        , PROCON_TEMPLATE_CONSTRAINTS(isConstructibleDividedImage<T>())
#endif
    >
    SwappedImage(T && master, std::vector<std::vector<ImageID>> const & idx)
    : _master(std::forward<T>(master)), _idx(idx)
    {}


    void swap_element(utils::Index2D a, utils::Index2D b)
    {
        std::swap(_idx[a[0]][a[1]], _idx[b[0]][b[1]]);
    }


    std::vector<std::vector<ImageID>> const & get_index() const
    {
        return _idx;
    }


    cv::Mat cvMat() const
    {
        auto cln = _master.clone();

        DividedImage::foreach(_master, [&](std::size_t i, std::size_t j){
            get_element(i, j).cvMat().copyTo(cln.get_element(i, j).cvMat());
        });

        return cln.cvMat();
    }


    size_t height() const { return _master.height(); }
    size_t width() const { return _master.width(); }
    size_t div_x() const { return _master.div_x(); }
    size_t div_y() const { return _master.div_y(); }
    Pixel get_pixel(std::size_t r, std::size_t c) const
    {
        const auto i = r / _master.div_y(),
                   j = c / _master.div_x(),
                   remI = r % _master.div_y(),
                   remJ = c % _master.div_x();
        
        return this->get_element(i, j).get_pixel(remI, remJ);
    }

    Image get_element(std::size_t r, std::size_t c)
    {
        return _master.get_element(_idx[r][c]);
    }


    const Image get_element(std::size_t r, std::size_t c) const
    {
        return _master.get_element(_idx[r][c]);
    }


    SwappedImage clone() const
    {
        SwappedImage dst(_master.clone(), _idx);
        return dst;
    }


    DividedImage dividedImage() { return _master; }
    const DividedImage dividedImage() const { return _master; }


  private:
    DividedImage _master;
    std::vector<std::vector<ImageID>> _idx;
};


}} // namespace procon::utils


namespace std {
template<>
class hash<procon::utils::ImageID> {
  public:
    size_t operator()(procon::utils::ImageID const & id) const
    {
        return id.get_hash();
    }
};
}