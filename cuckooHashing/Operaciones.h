#pragma once
#include <iostream>
#include "Cache.h"
#include "CuckooHash.h"
#include "Catalogos.h"
#include "Consola.h"

using namespace std;

// busca un DNI y muestra a la persona
inline void buscar(CuckooHash& tabla, Cache& cache, const Catalogos& c) {
    uint32_t dni = leerNumero("DNI a buscar: ", 1, 99999999);
    uint32_t idx = tabla.buscar(dni); // posicion en la cache o VACIO
    if (idx == VACIO) {
        cout << "No existe registro con ese DNI\n";
    } else {
        mostrarPersona(cache.leer(idx), c); // lee la persona del .bin
    }
}

// pide los datos de una persona nueva, la guarda en el .bin y en el cuckoo
inline void registrar(Cache& cache, CuckooHash& tabla, Catalogos& c) {
    uint32_t dni = leerNumero("DNI: ", 1, 99999999);
    if (tabla.buscar(dni) != VACIO) {
        cout << "Ya existe una persona con ese DNI\n";
        return;
    }

    Persona p;
    p.dni = dni;
    p.nombre1   = leerDeCatalogo("Primer nombre: ", c.nombres, 65535, c);
    p.nombre2   = leerDeCatalogo("Segundo nombre: ", c.nombres, 65535, c);
    p.apellido1 = leerDeCatalogo("Apellido paterno: ", c.apellidos, 65535, c);
    p.apellido2 = leerDeCatalogo("Apellido materno: ", c.apellidos, 65535, c);
    p.nacionalidad = 0; // Peruana

    p.lugarNacimiento = elegirDeLista("Lugar de nacimiento (nuevo: Ciudad, Departamento):", c.lugaresNacimiento, true, 255, c, false);
    p.ubicacion = elegirDeLista("Ubicacion:", c.ubicaciones, true, 255, c, true);
    p.tipoVia   = elegirDeLista("Tipo de via:", c.tiposVia, false, 255, c, false);
    p.nombreVia = elegirDeLista("Nombre de via:", c.nombresVia, true, 255, c, false);

    p.numeroVia = leerNumero("Numero de via (1-9999): ", 1, 9999);
    p.telefono  = leerNumero("Telefono (9 digitos, empieza en 9): ", 900000000, 999999999);
    p.estadoCivil = elegirDeLista("Estado civil:", c.estadosCiviles, false, 255, c, false);
    p.dominioCorreo = 0; // example.com

    // correo: nombres sin tildes ni espacios + dni de 8 digitos
    string d = to_string(dni);
    d = string(8 - d.size(), '0') + d; // 8 digitos
    string nombres = sinTildes(c.nombres[p.nombre1] + c.nombres[p.nombre2]);
    string sinEspacios;
    for (char ch : nombres) {
        if (ch != ' ') sinEspacios += ch;
    }
    p.usuarioCorreo = sinEspacios + d;

    uint32_t idx = cache.agregar(p); // escribe al final del .bin
    tabla.insertar(p.dni, idx);      // {dni, idx} en el cuckoo
    cout << "Persona registrada\n";
    mostrarPersona(p, c);
}

// borra a la persona del .bin y del cuckoo
inline void eliminar(Cache& cache, CuckooHash& tabla) {
    uint32_t dni = leerNumero("DNI a eliminar: ", 1, 99999999); // lee el numero
    uint32_t idx = tabla.buscar(dni); // lo busca
    if (idx == VACIO) { // si no hay
        cout << "No existe registro con ese DNI\n";
        return; // retorna
    }
    cache.marcarBorrado(idx); // primero en el disco
    tabla.eliminar(dni);      // despues en el cuckoo
    cout << "Persona eliminada\n";
}
