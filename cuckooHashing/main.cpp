#include <iostream>
#include "Cache.h"
#include "CuckooHash.h"
#include "Catalogos.h"
#include "Consola.h"
#include "Operaciones.h"

using namespace std;

// ruta del archivo binario
const string RUTA_BIN = "../setDatosBIN/setDatos/dataset.bin";

int main() {
    try {
        Cache cache;
        vector<uint32_t> dnis = cache.cargar(RUTA_BIN); // dnis[i] = DNI de la persona i
        cout << "Personas en la cache: " << cache.size() << "\n";

        CuckooHash tabla(cache.size());
        for (uint32_t i = 0; i < cache.size(); i++) {
            if (dnis[i] != DNI_BORRADO) tabla.insertar(dnis[i], i); // las marcadas no se insertan
        }
        cout << "Personas insertadas en el cuckoo\n";

        Catalogos catalogos = leerCatalogos(RUTA_CATALOGOS);

        while (true) {
            string opcion = leerLinea("\n1) Buscar\n2) Registrar\n3) Eliminar\n0) Salir\nOpcion: ");
            if (opcion == "1") {
                buscar(tabla, cache, catalogos);
            } else if (opcion == "2") {
                registrar(cache, tabla, catalogos);
            } else if (opcion == "3") {
                eliminar(cache, tabla);
            } else if (opcion == "0") {
                break;
            } else {
                cout << "Opcion invalida\n";
            }
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
