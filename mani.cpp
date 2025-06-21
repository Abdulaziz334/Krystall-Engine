#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

//
// ⚡️ Vektor2
//
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

//
// ⚡️ Vektor3
//
struct Vektor3 {
    float x, y, z;

    Vektor3() : x(0), y(0), z(0) {}
    Vektor3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    Vektor3 operator+(const Vektor3 &other) const {
        return Vektor3(x + other.x, y + other.y, z + other.z);
    }

    Vektor3 operator-(const Vektor3 &other) const {
        return Vektor3(x - other.x, y - other.y, z - other.z);
    }
};

//
// ⚡️ Matritsa4
//
struct Matritsa4 {
    float qiymatlar[16];
    Matritsa4() {
        for(int i = 0; i < 16; ++i) {
            qiymatlar[i] = 0.0f;
        }
        qiymatlar[0] = qiymatlar[5] = qiymatlar[10] = qiymatlar[15] = 1.0f;
    }
};

//
// ⚡️ Rang
//
struct Rang {
    float r, g, b, a;

    Rang() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Rang(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
};

//
// ⚡️ Hodisa
//
struct Hodisa {
    enum Tur {
        KlavishBosildi,
        KlavishQoyibYuborildi,
        SichqonchaBosildi,
        SichqonchaQoyibYuborildi
    };
    Tur turi;
    int kodi;

    Hodisa(Tur _turi, int _kodi) : turi(_turi), kodi(_kodi) {}

    void chop() {
        std::string nom;
        switch (turi) {
            case KlavishBosildi: nom = "Klavish bosildi"; break;
            case KlavishQoyibYuborildi: nom = "Klavish qoyib yuborildi"; break;
            case SichqonchaBosildi: nom = "Sichqoncha bosildi"; break;
            case SichqonchaQoyibYuborildi: nom = "Sichqoncha qoyib yuborildi"; break;
        }
        cout << "Hodisa: " << nom << ", Kodi: " << kodi << '\n';
    }
};

//
// ⚡️ Ilova
//
struct Ilova {
    bool ishlayaptimi;

    Ilova() : ishlayaptimi(true) {}

    void boshlash() {
        cout << "[Ilova] Boshlash\n";
    }

    void aylanish() {
        cout << "[Ilova] Aylanish boshlangan\n";
        for(int i = 0; i < 3; ++i) {
            cout << "Aylanish... " << i + 1 << '\n';
        }
    }

    void yopish() {
        cout << "[Ilova] Yopish\n";
        ishlayaptimi = false;
    }
};

//
// ⚡️ Resurs
//
struct Resurs {
    bool yuklandi;

    Resurs() : yuklandi(false) {}

    bool yuklash(const std::string &yo_l) {
        cout << "Resurs yuklanmoqda: " << yo_l << '\n';
        yuklandi = true;
        return yuklandi;
    }
};

//
// ⚡️ Render (Placeholder)
//
struct Render {
    void chizish(const std::string &nom) {
        cout << "[Render] '" << nom << "' chizilmoqda...\n";
    }
};

//
// ⚡️ Shablon (Template) Misoli
//
template<typename T>
struct Shablon {
    T qiymat;

    Shablon(T q) : qiymat(q) {}

    void chop() {
        cout << "[Shablon] Qiymat: " << qiymat << '\n';
    }
};

//
// ⚡️ MAIN
//
int main() {
    // ✅ Vektor2 / Vektor3
    Vektor2 v2a(2.0f, 3.0f), v2b(1.0f, 1.0f);
    auto v2_qosh = v2a + v2b;
    cout << "Vektor2 Qoshish: (" << v2_qosh.x << ", " << v2_qosh.y << ")\n";

    Vektor3 v3a(1.0f, 2.0f, 3.0f), v3b(4.0f, 5.0f, 6.0f);
    auto v3_qosh = v3a + v3b;
    cout << "Vektor3 Qoshish: (" << v3_qosh.x << ", " << v3_qosh.y << ", " << v3_qosh.z << ")\n";

    // ✅ Rang
    Rang rang1(1.0f, 0.5f, 0.25f);
    cout << "Rang: (" << rang1.r << ", " << rang1.g << ", " << rang1.b << ", " << rang1.a << ")\n";

    // ✅ Hodisa
    Hodisa h1(Hodisa::KlavishBosildi, 65);
    Hodisa h2(Hodisa::SichqonchaBosildi, 1);
    h1.chop();
    h2.chop();

    // ✅ Ilova
    Ilova app;
    app.boshlash();
    app.aylanish();
    app.yopish();

    // ✅ Resurs
    Resurs r;
    r.yuklash("resurs.png");

    // ✅ Matritsa4
    Matritsa4 mat;
    cout << "Matritsa4 (4x4):\n";
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            cout << fixed << setprecision(1) << mat.qiymatlar[i * 4 + j] << " ";
        }
        cout << '\n';
    }

    // ✅ Render
    Render rend;
    rend.chizish("Uchburchak");

    // ✅ Shablon
    Shablon<int> s1(42);
    s1.chop();

    Shablon<std::string> s2("KrystallEngine");
    s2.chop();

    return 0;
}