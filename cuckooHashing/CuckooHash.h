#pragma once
#include <iostream>
#include <vector>
#include <cstdint> // uint32_t, UINT32_MAX
#include <utility> // swap

using namespace std;

struct Entrada {
    uint32_t dni; // clave
    uint32_t idx; // posicion de la persona en la cache
};

const uint32_t VACIO = UINT32_MAX; // casilla vacia

class CuckooHash {
    uint32_t capacidad; // casillas de cada tabla
    vector<Entrada> tabla1;
    vector<Entrada> tabla2;

public:
    CuckooHash(size_t n) {
        capacidad = n / 0.9 + 1;
        tabla1.assign(capacidad, Entrada{0, VACIO});
        tabla2.assign(capacidad, Entrada{0, VACIO});
    }

    // metodo multiplicativo de Knuth
    uint32_t hash1(uint32_t dni) const {
        return (dni * 2654435769u) % capacidad;
    }

    // mezcla de Murmur
    uint32_t hash2(uint32_t dni) const {
        uint32_t x = dni;
        x ^= x >> 16;
        x *= 0x85ebca6bu;
        x ^= x >> 13;
        x *= 0xc2b2ae35u;
        x ^= x >> 16;
        return x % capacidad;
    }

    // bucle de desplazamientos; false si no se ubico (falta rehash)
    bool insertar(uint32_t dni, uint32_t idx) {
        Entrada e{dni, idx};
        for (int i = 0; i < 100; i++) {
            swap(e, tabla1[hash1(e.dni)]); // entra a la tabla 1 y sale la que estaba
            if (e.idx == VACIO) return true;
            swap(e, tabla2[hash2(e.dni)]); // la que salio va a la tabla 2
            if (e.idx == VACIO) return true;
        }
        return false;
    }

    // devuelve el idx de la persona o VACIO si no esta
    uint32_t buscar(uint32_t dni) const {
        Entrada e1 = tabla1[hash1(dni)]; // su casilla en la tabla 1
        if (e1.idx != VACIO && e1.dni == dni) return e1.idx;
        Entrada e2 = tabla2[hash2(dni)]; // su casilla en la tabla 2
        if (e2.idx != VACIO && e2.dni == dni) return e2.idx;
        return VACIO;
    }
};
