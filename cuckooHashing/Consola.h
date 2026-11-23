#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept> // runtime_error
#include "Catalogos.h"

using namespace std;

// ruta del archivo de catalogos
const string RUTA_CATALOGOS = "../setDatosBIN/setDatos/catalogos.txt";

// muestra el mensaje y devuelve la linea escrita
inline string leerLinea(const string& mensaje) {
    cout << mensaje;
    string s; // la entrada se guarda en s
    if (!getline(cin, s)) throw runtime_error("Entrada terminada"); // si esta vacia
    return s; // retorna el texto
}

// pide hasta recibir un numero entre minimo y maximo
inline uint32_t leerNumero(const string& mensaje, uint32_t minimo, uint32_t maximo) { // el min y max lo define el caracter o sea dni tel tipo via
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
inline uint32_t leerDeCatalogo(const string& mensaje, vector<string>& lista, size_t maximo, Catalogos& c) { // recibe la lista para modificar el catalogo y c el catalogo completo
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
inline uint32_t elegirDeLista(const string& titulo, vector<string>& lista, bool permiteAgregar, size_t maximo, Catalogos& c, bool esUbicacion) {
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

    uint32_t opcion = leerNumero("Opcion: ", (permiteAgregar && lista.size() < maximo) ? 0 : 1, lista.size()); // 0 solo si se puede agregar
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
