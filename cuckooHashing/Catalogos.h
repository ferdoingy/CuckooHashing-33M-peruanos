#pragma once
#include <iostream>
#include <iomanip>   // setw, setfill
#include <fstream>
#include <sstream>   // stringstream para separar por TAB
#include <string>
#include <vector>
#include <stdexcept> // runtime_error
#include "Persona.h"

using namespace std;

// textos de cada indice del .bin: catalogos.nombres[6] es el texto del nombre 6
struct Catalogos {
    vector<string> nombres;
    vector<string> apellidos;
    vector<string> nacionalidades;
    vector<string> lugaresNacimiento;
    vector<string> ubicaciones; // "departamento\tprovincia\tciudad\tdistrito"
    vector<string> tiposVia;
    vector<string> nombresVia;
    vector<string> estadosCiviles;
    vector<string> dominiosCorreo;
};

// lee "[NOMBRE] cantidad" y luego esa cantidad de lineas
inline vector<string> leerSeccion(istream& archivo, const string& nombre) {
    string linea;
    getline(archivo, linea);
    size_t cantidad = stoul(linea.substr(nombre.size() + 3)); // salta "[" + nombre + "] "

    vector<string> valores(cantidad);
    for (size_t i = 0; i < cantidad; i++) {
        getline(archivo, valores[i]);
    }
    return valores;
}

// lee las 9 secciones en el orden en que las escribe genBIN.py
inline Catalogos leerCatalogos(const string& ruta) {
    ifstream archivo(ruta);
    if (!archivo) throw runtime_error("No se pudo abrir " + ruta);

    Catalogos c;
    c.nombres           = leerSeccion(archivo, "NOMBRES");
    c.apellidos         = leerSeccion(archivo, "APELLIDOS");
    c.nacionalidades    = leerSeccion(archivo, "NACIONALIDADES");
    c.lugaresNacimiento = leerSeccion(archivo, "LUGARES_NACIMIENTO");
    c.ubicaciones       = leerSeccion(archivo, "UBICACIONES");
    c.tiposVia          = leerSeccion(archivo, "TIPOS_VIA");
    c.nombresVia        = leerSeccion(archivo, "NOMBRES_VIA");
    c.estadosCiviles    = leerSeccion(archivo, "ESTADOS_CIVILES");
    c.dominiosCorreo    = leerSeccion(archivo, "DOMINIOS_CORREO");
    return c;
}

// separa por TAB: departamento, provincia, ciudad, distrito
inline vector<string> separarUbicacion(const string& linea) {
    vector<string> campos(4);
    stringstream ss(linea);
    for (int i = 0; i < 4; i++) {
        getline(ss, campos[i], '\t');
    }
    return campos;
}

// imprime a la persona con sus indices traducidos a texto
inline void mostrarPersona(const Persona& p, const Catalogos& c) {
    vector<string> ubicacion = separarUbicacion(c.ubicaciones[p.ubicacion]);

    cout << "DNI:                 " << setfill('0') << setw(8) << p.dni << setfill(' ') << '\n'; // 8 digitos
    cout << "Nombres:             " << c.nombres[p.nombre1] << ' ' << c.nombres[p.nombre2] << '\n';
    cout << "Apellidos:           " << c.apellidos[p.apellido1] << ' ' << c.apellidos[p.apellido2] << '\n';
    cout << "Nacionalidad:        " << c.nacionalidades[p.nacionalidad] << '\n';
    cout << "Lugar de nacimiento: " << c.lugaresNacimiento[p.lugarNacimiento] << '\n';
    cout << "Departamento:        " << ubicacion[0] << '\n';
    cout << "Provincia:           " << ubicacion[1] << '\n';
    cout << "Ciudad:              " << ubicacion[2] << '\n';
    cout << "Distrito:            " << ubicacion[3] << '\n';
    cout << "Direccion:           " << c.tiposVia[p.tipoVia] << ' ' << c.nombresVia[p.nombreVia] << ' ' << p.numeroVia << '\n';
    cout << "Telefono:            " << p.telefono << '\n';
    cout << "Correo:              " << p.usuarioCorreo << '@' << c.dominiosCorreo[p.dominioCorreo] << '\n';
    cout << "Estado civil:        " << c.estadosCiviles[p.estadoCivil] << '\n';
}
