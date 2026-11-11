"""
Verifica un dataset.bin version 2 (agrupado por primer digito del DNI).

Uso:
    py verificar_bin.py [--bin setDatos/dataset.bin] [--semilla 42] [--referencia v1.bin]

Para comparar contra la version anterior:
  - con --referencia se lee un .bin version 1 generado por el genBIN.py anterior
  - sin --referencia se regeneran las personas con la misma semilla en el orden
    original (lo mismo que escribia el genBIN.py anterior)
Carga todo en memoria: pensado para pruebas chicas (miles de personas).
"""

import argparse # para pasar la ruta y la semilla
import sys # para el codigo de salida
from collections import Counter # para comparar conjuntos de personas con repetidos
from genBIN import CABECERA_V2, VERSION_BIN, SECCIONES, DIVISOR_SECCION
from catalogog import *

errores = [] # se acumulan todos los fallos

def comprobar(condicion, mensaje):
    print(f"  [{'OK' if condicion else 'FALLO'}] {mensaje}")
    if not condicion:
        errores.append(mensaje)
    return condicion

def leer_personas(datos, inicio, cantidad):
    # devuelve una lista de (dni, bytes completos de la persona) y el byte donde termina
    personas = []
    pos = inicio
    for _ in range(cantidad):
        campos = REGISTRO.unpack_from(datos, pos)
        fin = pos + REGISTRO.size + campos[-1] # el ultimo campo es la longitud del usuario
        if fin > len(datos):
            raise ValueError(f"persona en byte {pos} se sale del archivo")
        personas.append((campos[0], bytes(datos[pos:fin])))
        pos = fin
    return personas, pos

def personas_referencia(args, total):
    if args.referencia: # leer el .bin de la version anterior
        datos = Path(args.referencia).read_bytes()
        magic, version, n = CABECERA.unpack_from(datos, 0)
        if magic != MAGIC or version != 1:
            raise ValueError("la referencia no es un .bin version 1")
        personas, fin = leer_personas(datos, CABECERA.size, n)
        if fin != len(datos):
            raise ValueError("la referencia tiene bytes de mas")
        return [p for _, p in personas]

    # regenerar igual que el genBIN.py anterior (orden original)
    rng = random.Random(args.semilla)
    multiplicador = calcular_multiplicador(args.semilla)
    personas = []
    for i in range(total):
        campos, usuario_correo = generar_ciudadano(i, rng, multiplicador)
        personas.append(REGISTRO.pack(*campos) + usuario_correo)
    return personas

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--bin", default="setDatos/dataset.bin", help="Ruta del .bin version 2")
    parser.add_argument("--semilla", type=int, default=42, help="Semilla usada al generar")
    parser.add_argument("--referencia", default=None, help="Ruta de un .bin version 1 (opcional)")
    args = parser.parse_args()

    datos = Path(args.bin).read_bytes()
    print(f"Archivo: {args.bin} ({len(datos):,} bytes)")

    # 1. cabecera
    print("Cabecera:")
    if not comprobar(len(datos) >= CABECERA_V2.size, f"el archivo tiene al menos {CABECERA_V2.size} bytes"):
        return
    valores = CABECERA_V2.unpack_from(datos, 0)
    magic, version, total = valores[0], valores[1], valores[2]
    inicios = list(valores[3:3 + SECCIONES])
    cantidades = list(valores[3 + SECCIONES:3 + 2 * SECCIONES])

    comprobar(magic == MAGIC, f"magic = {magic!r}")
    comprobar(version == VERSION_BIN, f"version = {version}")
    comprobar(sum(cantidades) == total, f"suma de cantidades ({sum(cantidades):,}) = total ({total:,})")
    comprobar(inicios[0] == CABECERA_V2.size, f"la seccion 0 empieza justo despues de la cabecera ({inicios[0]})")

    # 2. secciones
    print("Secciones:")
    todas = [] # (dni, bytes) de todas las personas
    pos = CABECERA_V2.size # donde deberia empezar la siguiente seccion
    for d in range(SECCIONES):
        comprobar(inicios[d] == pos, f"seccion {d}: empieza en {inicios[d]:,} (esperado {pos:,}), {cantidades[d]:,} personas")
        try:
            personas, pos = leer_personas(datos, inicios[d], cantidades[d])
        except (ValueError, struct.error) as e:
            comprobar(False, f"seccion {d}: no se pudo leer ({e})")
            return
        malos = [dni for dni, _ in personas if dni // DIVISOR_SECCION != d]
        comprobar(not malos, f"seccion {d}: todos los DNIs empiezan con {d}" + (f" (malos: {malos[:5]})" if malos else ""))
        todas += personas

    # 3. fin del archivo
    print("Fin de archivo:")
    comprobar(pos == len(datos), f"el archivo termina justo despues del ultimo registro ({pos:,} = {len(datos):,})")

    # 4. DNIs unicos
    print("DNIs:")
    dnis = Counter(dni for dni, _ in todas)
    repetidos = [dni for dni, c in dnis.items() if c > 1]
    comprobar(not repetidos, f"sin DNIs repetidos ({len(dnis):,} distintos)" + (f" repetidos: {repetidos[:5]}" if repetidos else ""))

    # 5. mismas personas que la version anterior
    print("Comparacion con la version anterior:")
    origen = f"referencia {args.referencia}" if args.referencia else f"regeneradas con semilla {args.semilla}"
    referencia = personas_referencia(args, total)
    actuales = [p for _, p in todas]
    comprobar(len(referencia) == len(actuales), f"misma cantidad ({len(actuales):,} vs {len(referencia):,}, {origen})")
    comprobar(Counter(referencia) == Counter(actuales), "mismo conjunto de registros (byte a byte)")
    print(f"  (orden distinto al original: {referencia != actuales})")

    print()
    if errores:
        print(f"VERIFICACION FALLIDA: {len(errores)} error(es)")
        sys.exit(1)
    print("VERIFICACION CORRECTA")

if __name__ == "__main__":
    main()
