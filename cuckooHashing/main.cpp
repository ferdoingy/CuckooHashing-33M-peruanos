#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept> // exception, runtime_error
#include "Cache.h"
#include "CuckooHash.h"
#include "Catalogos.h"

using namespace std;

// rutas para los archivos binario y catalogo
const string RUTA_BIN = "../setDatosBIN/setDatos/dataset.bin";
const string RUTA_CATALOGOS = "../setDatosBIN/setDatos/catalogos.txt";

// muestra el mensaje y devuelve la linea escrita
string leerLinea(const string& mensaje) {
    cout << mensaje;
    string s; // la entrada se guarda en s
    if (!getline(cin, s)) throw runtime_error("Entrada terminada"); // si esta vacia 
    return s; // retorna el texto
}

// pide hasta recibir un numero entre minimo y maximo
uint32_t leerNumero(const string& mensaje, uint32_t minimo, uint32_t maximo) { // el min y max lo define el caracter o sea dni tel tipo via
    while (true) {
        string texto = leerLinea(mensaje);
        bool valido = texto.size() >= 1 && texto.size() <= 9; // 9 digitos caben en uint32_t
        for (char ch : texto) {
            if (ch < '0' || ch > '9') valido = false; // si no estan del 0 - 9 falso
        }
        if (valido) {
            uint32_t n = stoul(texto); // si es que si es valido se convierte ese string en numero con stoul
            if (n >= minimo && n <= maximo) return n; //sirve para todo numero de DNI telefono y tipo via
        }
        cout << "Valor invalido\n"; // avisa y vuelve al while
    }
}

// pide un texto del catalogo; si no existe ofrece agregarlo
uint32_t leerDeCatalogo(const string& mensaje, vector<string>& lista, size_t maximo, Catalogos& c) { // recibe la lista para modificar el catalogo y c el catalogo completo
    while (true) { // c el catalogo completo que necsita para guardar y reescribir el catalogo
        string texto = leerLinea(mensaje); // lee el texto 
        int i = buscarEnCatalogo(lista, texto); // lo busca en el catalogo y guarda la posicion en i
        if (i >= 0) return i; // si es mayor igual a 0 es decir que lo encontro entonces devuelve ese elemento del catalogo

        string respuesta = leerLinea("No existe en el catalogo. Agregarlo? (s/n): "); // si no pregunta si quiere agregarlo
        if (respuesta == "s" && lista.size() < maximo) // si es si y el tamaño de la lista < que el limite del tipo de dato
        {
            lista.push_back(texto);
            guardarCatalogos(c, RUTA_CATALOGOS); // guarda el catalogo con el nuevo valor
            return lista.size() - 1;
        }
        if (lista.size() >= maximo) cout << "El catalogo esta lleno\n";
    }
}

// muestra la lista numerada y devuelve el indice elegido (0 agrega uno nuevo)
uint32_t elegirDeLista(const string& titulo, vector<string>& lista, bool permiteAgregar, size_t maximo, Catalogos& c, bool esUbicacion) {
    cout << titulo << "\n"; // el titulo es el encabezado
    for (size_t i = 0; i < lista.size(); i++) { // recorre y muestra
        string valor;
        for (char ch : lista[i]) {
            if (ch == '\t') valor += " / "; // TAB se muestra como " / " -> Lima/Lima/Lima/Miraflores
            else valor += ch;
        }
        cout << "  " << i + 1 << ") " << valor << "\n"; // muestra desde el 1
    }
    if (permiteAgregar && lista.size() < maximo) cout << "  0) Agregar nuevo\n"; // si la lista no esta llena entonces se permite agregar nuevo

    uint32_t opcion = leerNumero("Opcion: ", permiteAgregar ? 0 : 1, lista.size()); // si el bool es true return 0 
    if (opcion > 0) return opcion - 1;

    string nuevo; // guarda lo que lee en departamento, provincia, ciudad, distrito
    if (esUbicacion) { // departamento\tprovincia\tciudad\tdistrito los separa con TAB para tener el mismo formato que en el catalogo
        nuevo = leerLinea("Departamento: ");
        nuevo += '\t' + leerLinea("Provincia: ");
        nuevo += '\t' + leerLinea("Ciudad: ");
        nuevo += '\t' + leerLinea("Distrito: ");
    } else {
        nuevo = leerLinea("Nuevo valor: "); // si no pide una calle nueva por ejemplo
    }
    lista.push_back(nuevo);
    guardarCatalogos(c, RUTA_CATALOGOS); // guarda el catalogo con el nuevo valor
    return lista.size() - 1; // devuelve el ultimo indice
}

int main() {
    try {
        Cache cache;
        vector<uint32_t> dnis = cache.cargar(RUTA_BIN); // dnis[i] = DNI de la persona i
        cout << "Personas en la cache: " << cache.size() << "\n";

        CuckooHash tabla(cache.size());
        for (uint32_t i = 0; i < cache.size(); i++) {
            tabla.insertar(dnis[i], i); // pareja (dni, idx)
        }
        cout << "Personas insertadas en el cuckoo\n";

        Catalogos catalogos = leerCatalogos(RUTA_CATALOGOS);

        while (true) {
            string opcion = leerLinea("\n1) Buscar\n0) Salir\nOpcion: ");
            if (opcion == "1") {
                uint32_t dni = leerNumero("DNI a buscar: ", 1, 99999999);
                uint32_t idx = tabla.buscar(dni); // posicion en la cache o VACIO
                if (idx == VACIO) {
                    cout << "No existe registro con ese DNI\n";
                } else {
                    mostrarPersona(cache.leer(idx), catalogos); // lee la persona del .bin
                }
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
