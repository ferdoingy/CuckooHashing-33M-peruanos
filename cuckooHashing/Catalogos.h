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

// indice de texto en lista, o -1 si no esta
inline int buscarEnCatalogo(const vector<string> &lista, const string &texto) // recorre la lista desde 0 - final
{
    for (size_t i = 0; i < lista.size(); i++) // En cuanto encuentra el elemento igual al texto
    { 
        if (lista[i] == texto) return i; // devuelve la pos
    }
    return -1; // no esta
}

// escribe "[NOMBRE] cantidad" y luego un valor por linea
// lleva & para usar el mismo archivo abierto
inline void escribirSeccion(ofstream& archivo, const string& nombre, const vector<string>& valores)
{ // es la cabecera de la seccion [NOMBRES] 280 la cantidad sale de valores.size()
    archivo << '[' << nombre << "] " << valores.size() << '\n';
    for (const string& v : valores)
    {
        archivo << v << '\n'; // escribe cada valor en su propia linea
    }
}

// escribe las 9 secciones en el mismo orden que genBIN.py
inline void guardarCatalogos(const Catalogos& c, const string& ruta)
{
    ofstream archivo(ruta, ios::binary); // binary: saltos de linea '\n' como Python
    // abre el archivo para escribir se borra y se escribe todo de nuevo
    escribirSeccion(archivo, "NOMBRES", c.nombres);
    escribirSeccion(archivo, "APELLIDOS", c.apellidos);
    escribirSeccion(archivo, "NACIONALIDADES", c.nacionalidades);
    escribirSeccion(archivo, "LUGARES_NACIMIENTO", c.lugaresNacimiento);
    escribirSeccion(archivo, "UBICACIONES", c.ubicaciones);
    escribirSeccion(archivo, "TIPOS_VIA", c.tiposVia);
    escribirSeccion(archivo, "NOMBRES_VIA", c.nombresVia);
    escribirSeccion(archivo, "ESTADOS_CIVILES", c.estadosCiviles);
    escribirSeccion(archivo, "DOMINIOS_CORREO", c.dominiosCorreo);
}

// cambia las letras con tilde (UTF-8) por su version sin tilde
inline string sinTildes(string s) {
    const string con[] = {"á", "é", "í", "ó", "ú", "ñ", "ü", "Á", "É", "Í", "Ó", "Ú", "Ñ", "Ü"};
    const string sin[] = {"a", "e", "i", "o", "u", "n", "u", "A", "E", "I", "O", "U", "N", "U"};
    for (int i = 0; i < 14; i++)
    {
        size_t pos = s.find(con[i]); // busca donde aparece en el texto, si no aparece devuevle no encontrado
        while (pos != string::npos)
        {
            s.replace(pos, con[i].size(), sin[i]);
            pos = s.find(con[i], pos + 1);
        }
    }
    return s;
}
