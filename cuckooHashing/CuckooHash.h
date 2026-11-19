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
    uint32_t cantidad = 0; // parejas guardadas para calcular la carga

    // bucle de desplazamientos; si falla, e queda con la pareja que quedo fuera
    bool colocar(Entrada& e) {
        for (int i = 0; i < 100; i++) {
            swap(e, tabla1[hash1(e.dni)]); // entra a la tabla 1 y sale la que estaba
            if (e.idx == VACIO) return true;
            swap(e, tabla2[hash2(e.dni)]); // la que salio va a la tabla 2
            if (e.idx == VACIO) return true;
        }
        return false;
    }

    // duplica la capacidad y reinserta todas las parejas
    void rehash() {
        vector<Entrada> todas; // se crea un vector para llenar con todas las claves
        for (Entrada e : tabla1) if (e.idx != VACIO) todas.push_back(e); // Agregamos las claves
        for (Entrada e : tabla2) if (e.idx != VACIO) todas.push_back(e);

        bool ok = false; // bandera para 
        while (!ok) {
            capacidad *= 2;
            tabla1.assign(capacidad, Entrada{0, VACIO});
            tabla2.assign(capacidad, Entrada{0, VACIO});
            ok = true;
            for (Entrada e : todas) { // e es una copia
                if (!colocar(e)) { ok = false; break; } // falla: se vuelve a duplicar
            }
        }
    }

public:
    CuckooHash(size_t n) { // n es la cantidad de personas que se van a guardar -> cache.size()
                            // Sale del total que dice la cabecera del -bin
        capacidad = n / 0.9 + 1; // Sale del factor de carga (alpha) mide que tan llenas estan las tablas
        tabla1.assign(capacidad, Entrada{0, VACIO}); // alpha = parejas guardadas / casillas totales * 2 porque son 2 tablas
        tabla2.assign(capacidad, Entrada{0, VACIO}); // alpha = n / 2m; m -> capacidad de tabla
    }

    // hash entero de Thomas Wang
    uint32_t hash1(uint32_t dni) const {
        uint32_t x = dni;
        x = (x ^ 61) ^ (x >> 16);
        x *= 9;
        x ^= x >> 4;
        x *= 0x27d4eb2du;
        x ^= x >> 15;
        return x % capacidad;
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

    // inserta la pareja; hace rehash si la carga pasa del 45% o si colocar falla
    void insertar(uint32_t dni, uint32_t idx) {
        if (cantidad + 1 > 0.45 * 2 * capacidad) rehash();
        Entrada e{dni, idx};
        while (!colocar(e)) rehash(); // reintenta con la pareja que quedo fuera
        cantidad++;
    }

    // devuelve el idx de la persona o VACIO si no esta
    uint32_t buscar(uint32_t dni) const {
        Entrada e1 = tabla1[hash1(dni)]; // su casilla en la tabla 1
        if (e1.idx != VACIO && e1.dni == dni) return e1.idx;
        Entrada e2 = tabla2[hash2(dni)]; // su casilla en la tabla 2
        if (e2.idx != VACIO && e2.dni == dni) return e2.idx;
        return VACIO;
    }

    // vacia la casilla del dni; devuelve false si no estaba
    bool eliminar(uint32_t dni) {
        Entrada &e1 = tabla1[hash1(dni)]; // calcula en que casilla de la tabla 1 esta ese DNI
        if (e1.idx != VACIO && e1.dni == dni) // ¿Esta ahi el DNI?
        {
            e1 = Entrada{0, VACIO}; // lo marca como vacio
            cantidad--; // resta 1 a las parejas guardadas para que el factor siga siendo correcto
            return true; // si no se resta la funcion insertar creera que la tabla esta más llena
        } //  Y cambiaria el rehash porque se haria antes
        Entrada &e2 = tabla2[hash2(dni)];
        if (e2.idx != VACIO && e2.dni == dni) {
            e2 = Entrada{0, VACIO};
            cantidad--;
            return true;
        }
        return false; // no estaba
    }
};
