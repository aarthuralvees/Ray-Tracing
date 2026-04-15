#ifndef VETORHEADER
#define VETORHEADER

#include <iostream>
#include <cmath>

class Vetor {
public:
    Vetor(double x=0, double y=0, double z=0): x(x), y(y), z(z) {}

    Vetor operator+(const Vetor& v) const { 
        return Vetor(x + v.x, y + v.y, z + v.z); 
    }
    Vetor operator-(const Vetor& v) const {
        return Vetor(x - v.x, y - v.y, z - v.z);
    }
    Vetor operator-() const { 
        return Vetor(-x, -y, -z); 
    }
    Vetor operator*(double t) const {
        return Vetor(x * t, y * t, z * t);
    }
    Vetor operator/(double t) const {
        return Vetor(x / t, y / t, z / t);
    }
    
    Vetor& operator*=(double t) {
        x *= t; y *= t; z *= t;
        return *this;
    }
    Vetor& operator/=(double t) {
        return *this *= 1/t;
    }

    double length() const {
        return std::sqrt(length_squared());
    }
    double length_squared() const {
        return x*x + y*y + z*z;
    }

    friend std::ostream& operator<<(std::ostream& os, const Vetor &v){ 
        return os << "(" << v.x << ", " << v.y << ", " << v.z << ")T"; 
    }
    
    double getX() const { return x; }
    double getY() const { return y; }
    double getZ() const { return z; }

private:
    double x, y, z;
};

inline Vetor operator*(double t, const Vetor& v) {
    return v * t;
}

inline double dot(const Vetor& u, const Vetor& v) {
    return u.getX() * v.getX()
         + u.getY() * v.getY()
         + u.getZ() * v.getZ();
}

inline Vetor cross(const Vetor& u, const Vetor& v) {
    return Vetor(u.getY() * v.getZ() - u.getZ() * v.getY(),
                 u.getZ() * v.getX() - u.getX() * v.getZ(),
                 u.getX() * v.getY() - u.getY() * v.getX());
}

inline Vetor unit_vector(const Vetor& v) {
    return v / v.length();
}

using color = Vetor;

inline void write_color(std::ostream& out, const color& pixel_color) {
    auto r = pixel_color.getX();
    auto g = pixel_color.getY();
    auto b = pixel_color.getZ();

    int ir = int(255.999 * r);
    int ig = int(255.999 * g);
    int ib = int(255.999 * b);

    out << ir << ' ' << ig << ' ' << ib << '\n';
}

#endif