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
const uint32_t DNI_BORRADO = UINT32_MAX; // marca en el .bin de persona eliminada
const uint64_t POS_BORRADA = UINT64_MAX; // marca en la cache de persona eliminada

class Cache {
    vector<uint64_t> posiciones; // posiciones[i] = byte donde empieza la persona i
    fstream archivo;             // queda abierto para leer y agregar personas despues

public:
    vector<uint32_t> cargar(const string& ruta) {
        archivo.open(ruta, ios::in | ios::out | ios::binary); //in permite leer out permite escribir
        if (!archivo) throw runtime_error("No se pudo abrir " + ruta); // binary hace que se trabaje por bytes

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

            posiciones.push_back(dni == DNI_BORRADO ? POS_BORRADA : pos); // agrega a las posiciones si es q esta borrado
                                                                         // si no solo agrega la posicion
            pos += 26 + largoCorreo;
        }
        return dnis;
    }

    // va al byte de la persona idx y la lee completa
    Persona leer(uint32_t idx) {
        archivo.clear(); // limpia errores de lecturas anteriores
        archivo.seekg(posiciones[idx]); // pone el cursor de lectura en el indice que se le dio

        Persona p;
        uint8_t largoCorreo; // para guardar los espacios que ocupa correo
        archivo.read((char*)&p.dni, 4); // se lee los campos con los bytes
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

        // con el correo no se puede hacer eso porque es un string porque tiene mas cosas que solo texto
        p.usuarioCorreo.resize(largoCorreo); // prepara un string 
        archivo.read(&p.usuarioCorreo[0], largoCorreo); // usuario del correo llena los espacios del string
        return p;
    }

    // FUNCION PARA AGREGAR UNA PERSONA
    // escribe la persona al final del archivo y devuelve su idx
    // Ahora no solo lo tiene como modo lectura si no que ahora tambien se modifica fstream
    uint32_t agregar(const Persona& p)
    {
        archivo.clear(); // limpia errores de lecturas anteriores
        archivo.seekp(0, ios::end); // pone la escritura al final porque se añade al final
        uint64_t pos = archivo.tellp(); // tellp indica donde esta el cursor para lectura o escritura
                                        // pos es un numero, el byte donde empieza la nueva persona

        uint8_t largoCorreo = p.usuarioCorreo.size(); // desde aqui se escriben todos
        archivo.write((char*)&p.dni, 4); // se usan los tamaños que son exactos los mismos que en Python
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

        uint64_t total = posiciones.size() + 1; // nuevo total de la cabecera se le añade 1 por el nuevo
        archivo.seekp(8); // El total de las personas esta en el byte 8 a 15 entonces se va al byte 8 
        archivo.write((char*)&total, 8); // se encarga de cambiar el total a total + 1
                                         // esto es para que cuando corra de nuevo la persona no se pierda 
        archivo.flush(); // obliga a escribir en el disco en ese momento.

        posiciones.push_back(pos); // aqui agrega a la cache el byte de donde se quedo la nueva persona
        return posiciones.size() - 1; // devuelve su indice - 1 porque empieza en 0
        // el ultimo esta en el 33 000 000 pero hay 33 000 001 - 1
    }

    // marca a la persona idx como borrada en el .bin y en la cache
    void marcarBorrado(uint32_t idx) {
        archivo.clear();
        archivo.seekp(posiciones[idx]);       // byte donde empieza la persona
        uint32_t marca = DNI_BORRADO;         // copia local para tomar su direccion
        archivo.write((char*)&marca, 4);      // sobrescribe solo su DNI
        archivo.flush();                      // se guarda en el disco ya inmediato
        posiciones[idx] = POS_BORRADA;        // la marca tambien en la cache
    }

    size_t size() const
    { 
        return posiciones.size();
    }
};
