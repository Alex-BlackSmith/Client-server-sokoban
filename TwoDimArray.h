#ifndef SOKOBAN_PROJ_TWODIMARRAY_H
#define SOKOBAN_PROJ_TWODIMARRAY_H

#include <iostream>
#include <libtcod.h>
#include <ostream>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::ostream;
using std::ifstream;
using std::istream;
template <typename T> class TwoDimArray {
public:
    TwoDimArray() = default;

    TwoDimArray(const unsigned argLen,const unsigned argWth) : length(argLen), width(argWth)
    {
        size = width*length;
        value = new T [size];
    }

    TwoDimArray(const TwoDimArray& tempArr){
        *this = tempArr;
        length = tempArr.length;
        width = tempArr.width;
        size = tempArr.size;
        value = new T [size];
        for (auto i = 0; i < size; i++) {
            value[i] = tempArr.value[i];
        }
    }

    TwoDimArray& operator= (const TwoDimArray& tempArr) {
        if (this != &tempArr) {
            delete[] value;
            length = tempArr.length;
            width = tempArr.width;
            size = tempArr.size;
            value = new T [size];
            for (int i = 0; i < size; i++) {
                value[i] = tempArr.value[i];
            }
        }
        else{
            return *this;
        }
    }

    ~TwoDimArray() {
        delete[] value;
    }

    void setObjPos(const unsigned posX, const unsigned posY, T obj) {
        value[posX * width + posY] = obj;
    }
    char getObjPos (const unsigned posX, const unsigned posY) const{
        return value[posX * width + posY];
    }

    unsigned  getDimX() const {
        return width;
    }

    unsigned  getDimY() const {
        return length;
    }
    T *getArray(){
        return value;
    }
    friend ostream& operator<< (ostream& stream, TwoDimArray<char>& TwoDArray);
    friend istream& operator>> (istream &file, TwoDimArray<char>& TwoDArray);
    //T *value;
private:
    unsigned length;
    unsigned width;
    unsigned size;
    T * value;
};

ostream& operator<<(ostream& stream, TwoDimArray<char>& TwoDArray) {
    for (auto i = 0; i < TwoDArray.length; i++){
        for (auto j = 0; j < TwoDArray.width; j++){
            stream << TwoDArray.getObjPos(i,j);
        }
        cout << endl;
    }
    return stream;
}

//overload >> operator
istream& operator>> (istream &file, TwoDimArray<char> &TwoDArray) {
    unsigned len, wth;
    file >> len >> wth;
    string buf;
    getline(file, buf);

    if ((TwoDArray.getDimY() == len) && (TwoDArray.getDimX() == wth)){
        for (auto i = 0; i < len; i++) {
            for (auto j = 0; j <= wth; j++) {
                TwoDArray.setObjPos(i, j, file.get());
            }
        }
    }
    else {
        TwoDimArray<char> mapLocal(len, wth);
        for (auto i = 0; i < len; i++) {
            for (auto j = 0; j <= wth; j++) {
                mapLocal.setObjPos(i, j, file.get());
            }
        }
        TwoDArray = mapLocal;
    }
}

#endif //SOKOBAN_PROJ_TWODIMARRAY_H

