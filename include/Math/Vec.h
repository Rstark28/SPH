//
// Created by Robert Stark on 2/15/26.
//

#ifndef Vec3_H
#define Vec3_H
#include <algorithm>
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <stdexcept>

/*
 * A simple fixed-size Vec3tor class for basic mathematical operations.
 * Supports addition, subtraction, scalar multiplication/division, dot product, and normalization.
 * The dimension of the Vec3tor is specified as a template parameter.
 */
template <typename T> struct Vec3 {
    T _data[3];

    Vec3() = default;

    Vec3(std::initializer_list<T> list) noexcept
    {
        std::copy_n(list.begin(), std::min(list.size(), static_cast<size_t>(3)), _data);
    }

    Vec3(const Vec3& v) = default;
    Vec3(Vec3&& v) = default;
    Vec3& operator=(const Vec3& v)
    {
        if (this != &v) {
            std::memcpy(_data, v._data, sizeof(_data));
        }
        return *this;
    }

    template <typename U>
    explicit Vec3(const Vec3<U>& other)
        : Vec3 { static_cast<T>(other[0]), static_cast<T>(other[1]), static_cast<T>(other[2]) }
    {
    }

    Vec3& operator=(Vec3&& v) = default;

    // Access element by index
    T& operator[](size_t index)
    {
        return _data[index];
    }

    const T& operator[](size_t index) const
    {
        return _data[index];
    }

    // Vec3tor operations
    Vec3& operator+=(const Vec3& v) noexcept
    {
        _data[0] += v[0];
        _data[1] += v[1];
        _data[2] += v[2];
        return *this;
    }

    Vec3& operator+=(const T scalar) noexcept
    {
        _data[0] += scalar;
        _data[1] += scalar;
        _data[2] += scalar;
        return *this;
    }

    Vec3 operator+(const Vec3& v) const noexcept
    {
        return Vec3(*this) += v;
    }

    Vec3 operator+(const T scalar) const noexcept
    {
        return Vec3(*this) += scalar;
    }

    Vec3& operator-=(const Vec3& v) noexcept
    {
        _data[0] -= v[0];
        _data[1] -= v[1];
        _data[2] -= v[2];
        return *this;
    }

    Vec3& operator-=(const T scalar) noexcept
    {
        _data[0] -= scalar;
        _data[1] -= scalar;
        _data[2] -= scalar;
        return *this;
    }

    Vec3 operator-(const Vec3& v) const noexcept
    {
        return Vec3(*this) -= v;
    }

    Vec3 operator-(const T scalar) const noexcept
    {
        return Vec3(*this) -= scalar;
    }

    Vec3& operator*=(const T scalar) noexcept
    {
        _data[0] *= scalar;
        _data[1] *= scalar;
        _data[2] *= scalar;
        return *this;
    }

    Vec3 operator*(const T scalar) const noexcept
    {
        return Vec3(*this) *= scalar;
    }

    Vec3& operator/=(const T scalar) noexcept
    {
        _data[0] /= scalar;
        _data[1] /= scalar;
        _data[2] /= scalar;
        return *this;
    }

    Vec3 operator/(const T scalar) const noexcept
    {
        return Vec3(*this) /= scalar;
    }

    Vec3 operator^=(const Vec3& v)
    {
        _data[0] ^= v[0];
        _data[1] ^= v[1];
        _data[2] ^= v[2];
        return *this;
    }

    Vec3& operator^(const Vec3& v) const
    {
        return Vec3(*this) ^= v;
    }

    T operator*(const Vec3& v) const
    {
        return _data[0] * v[0] + _data[1] * v[1] + _data[2] * v[2];
    }

    [[nodiscard]] T norm() const
    {
        return std::sqrt(_data[0] * _data[0] + _data[1] * _data[1] + _data[2] * _data[2]);
    }

    Vec3 operator|(const Vec3& v)
    {
        return Vec3 { _data[1] * v[2] - _data[2] * v[1], _data[2] * v[0] - _data[0] * v[2],
            _data[0] * v[1] - _data[1] * v[0] };
    }

    Vec3 normalize()
    {
        T norm = this->norm();
        if (norm == 0.0f)
            throw std::runtime_error("Cannot normalize zero-length Vec3tor");
        return *this / norm;
    }

    T* begin()
    {
        return _data;
    }
    T* end()
    {
        return _data + 3;
    }
    [[nodiscard]] const T* begin() const
    {
        return _data;
    }
    [[nodiscard]] const T* end() const
    {
        return _data + 3;
    }

    friend Vec3<T> operator+(T scalar, const Vec3<T>& v);
    friend Vec3<T> operator*(T scalar, const Vec3<T>& v);
    friend std::ostream& operator<<(std::ostream& os, const Vec3<T>& v);
};

template <typename T> Vec3<T> operator+(const T scalar, const Vec3<T>& v)
{
    return Vec3(v) + scalar;
}

template <typename T> Vec3<T> operator*(const T scalar, const Vec3<T>& v)
{
    return v * scalar;
}

#endif // Vec3_H
