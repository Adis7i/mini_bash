#include <string>

/*
Semua fungsi akan mereturn bool sebagai tanda diberlakukan operasi atau tidak (Kecuali getter)
*/

#define PATH_LIMIT 5

// Struct untuk argumen fungsi inti yang berinteraksi dengan file system
struct FSArg {
    private: // Praktik enkapsulasi
    // Argumen path untuk sebagian besar fungsi
    std::string path1;
    std::string path2;
    
    public:
    int flag;
    // Menambah path sesuai urutan. dari path1 baru path2 yang diisi
    bool addPath(std::string path);

    const std::string* gpath1() const;
    const std::string* gpath2() const;
};









