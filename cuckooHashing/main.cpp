#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept> // exception
#include "Cache.h"
#include "CuckooHash.h"
#include "Catalogos.h"

using namespace std;

int main() {
    try {
        Cache cache;
        vector<uint32_t> dnis = cache.cargar("../setDatosBIN/setDatos/dataset.bin"); // dnis[i] = DNI de la persona i
        cout << "Personas en la cache: " << cache.size() << "\n";

        CuckooHash tabla(cache.size());
        for (uint32_t i = 0; i < cache.size(); i++) {
            tabla.insertar(dnis[i], i); // pareja (dni, idx)
        }
        cout << "Personas insertadas en el cuckoo\n";

        Catalogos catalogos = leerCatalogos("../setDatosBIN/setDatos/catalogos.txt");

        uint32_t dni;
        while (true) {
            cout << "\nDNI a buscar (0 para salir): ";
            cin >> dni;
            if (dni == 0) break;

            uint32_t idx = tabla.buscar(dni); // posicion en la cache o VACIO
            if (idx == VACIO) {
                cout << "No existe registro con ese DNI\n";
            } else {
                mostrarPersona(cache.leer(idx), catalogos); // lee la persona del .bin
            }
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
