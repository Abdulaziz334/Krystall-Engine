#include <iostream>
#include <iomanip> // Matritsa chiqishi uchun
using namespace std;

struct Vektor2 {
    float x, y;

    Vektor2() : x(0), y(0) {}
    Vektor2(float _x, float _y) : x(_x), y(_y) {}

    Vektor2 operator+(const Vektor2 &other) const {
        return Vektor2(x + other.x, y + other.y);
    }

    Vektor2 operator-(const Vektor2 &other) const {
        return Vektor2(x - other.x, y - other.y);
    }
};

struct Matritsa4 {
    float qiymatlar[16];
    Matritsa4() {
        for(int i = 0; i < 16; ++i) {
            qiymatlar[i] = 0.0f;
        }
        qiymatlar[0] = qiymatlar[5] = qiymatlar[10] = qiymatlar[15] = 1.0f;
    }
};

int main() {
    // ⚡️ Vektor2 test
    Vektor2 a(2.0f, 3.0f);
    Vektor2 b(1.0f, 1.0f);

    Vektor2 natija_qosh = a + b;
    Vektor2 natija_ayir = a - b;

    cout << "Vektor2 Qoshish natijasi: (" << natija_qosh.x << ", " << natija_qosh.y << ")\n";
    cout << "Vektor2 Ayirish natijasi: (" << natija_ayir.x << ", " << natija_ayir.y << ")\n\n";

    // ⚡️ Matritsa4 test
    Matritsa4 mat;

    cout << "Matritsa4 (4x4):\n";
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            cout << fixed << setprecision(1) << mat.qiymatlar[i * 4 + j] << " ";
        }
        cout << '\n';
    }

    return 0;
}