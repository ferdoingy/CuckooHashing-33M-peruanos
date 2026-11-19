#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept> // runtime_error
#include <cstring>   // memcpy
#include "Persona.h"

using namespace std;

// cabecera (176 bytes): "DNIP" | version | total | inicio[10] | cantidad[10]
// persona: 26 bytes fijos (el ultimo es el largo del correo) + usuario del correo
class Cache {
    vector<uint64_t> posiciones; // posiciones[i] = byte donde empieza la persona i
    fstream archivo;             // queda abierto para leer y agregar personas despues

public:
    vector<uint32_t> cargar(const string& ruta) {
        archivo.open(ruta, ios::in | ios::out | ios::binary);
        if (!archivo) throw runtime_error("No se pudo abrir " + ruta);

        archivo.ignore(8);                 // salta la firma "DNIP" y la version
        uint64_t total;
        archivo.read((char*)&total, 8);    // total de personas
        archivo.ignore(160);               // inicio[10] y cantidad[10], no se usan

        posiciones.reserve(total);
        vector<uint32_t> dnis; // dnis[i] = DNI de la persona i
        dnis.reserve(total);
        uint64_t pos = 176; // la primera persona va despues de la cabecera
        char registro[26];
        for (uint64_t i = 0; i < total; i++) {
            archivo.read(registro, 26);
            uint32_t dni;
            memcpy(&dni, registro, 4); // el DNI son los primeros 4 bytes
            dnis.push_back(dni);
            uint8_t largoCorreo = registro[25];
            archivo.ignore(largoCorreo); // se salta el correo

            posiciones.push_back(pos);
            pos += 26 + largoCorreo;
        }
        return dnis;
    }

    // va al byte de la persona idx y la lee completa
    Persona leer(uint32_t idx) {
        archivo.clear(); // limpia errores de lecturas anteriores
        archivo.seekg(posiciones[idx]);

        Persona p;
        uint8_t largoCorreo;
        archivo.read((char*)&p.dni, 4);
        archivo.read((char*)&p.nombre1, 2);
        archivo.read((char*)&p.nombre2, 2);
        archivo.read((char*)&p.apellido1, 2);
        archivo.read((char*)&p.apellido2, 2);
        archivo.read((char*)&p.nacionalidad, 1);
        archivo.read((char*)&p.lugarNacimiento, 1);
        archivo.read((char*)&p.ubicacion, 1);
        archivo.read((char*)&p.tipoVia, 1);
        archivo.read((char*)&p.nombreVia, 1);
        archivo.read((char*)&p.numeroVia, 2);
        archivo.read((char*)&p.telefono, 4);
        archivo.read((char*)&p.estadoCivil, 1);
        archivo.read((char*)&p.dominioCorreo, 1);
        archivo.read((char*)&largoCorreo, 1);

        p.usuarioCorreo.resize(largoCorreo);
        archivo.read(&p.usuarioCorreo[0], largoCorreo); // usuario del correo
        return p;
    }

    // escribe la persona al final del archivo y devuelve su idx
    uint32_t agregar(const Persona& p)
    {
        archivo.clear(); // limpia errores de lecturas anteriores
        archivo.seekp(0, ios::end);
        uint64_t pos = archivo.tellp(); // byte donde empieza

        uint8_t largoCorreo = p.usuarioCorreo.size();
        archivo.write((char*)&p.dni, 4);
        archivo.write((char*)&p.nombre1, 2);
        archivo.write((char*)&p.nombre2, 2);
        archivo.write((char*)&p.apellido1, 2);
        archivo.write((char*)&p.apellido2, 2);
        archivo.write((char*)&p.nacionalidad, 1);
        archivo.write((char*)&p.lugarNacimiento, 1);
        archivo.write((char*)&p.ubicacion, 1);
        archivo.write((char*)&p.tipoVia, 1);
        archivo.write((char*)&p.nombreVia, 1);
        archivo.write((char*)&p.numeroVia, 2);
        archivo.write((char*)&p.telefono, 4);
        archivo.write((char*)&p.estadoCivil, 1);
        archivo.write((char*)&p.dominioCorreo, 1);
        archivo.write((char*)&largoCorreo, 1);
        archivo.write(p.usuarioCorreo.data(), largoCorreo); // usuario del correo

        uint64_t total = posiciones.size() + 1; // nuevo total de la cabecera
        archivo.seekp(8);
        archivo.write((char*)&total, 8);
        archivo.flush();

        posiciones.push_back(pos);
        return posiciones.size() - 1;
    }

    size_t size() const { return posiciones.size(); }
};
