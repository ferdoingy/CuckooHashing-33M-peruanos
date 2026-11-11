# Formato de dataset.bin (version 2, little-endian, sin relleno)
#
# CABECERA (176 bytes):
#   magic      4s      b"DNIP"
#   version    I       2
#   total      Q       cantidad total de personas
#   inicio     10 x Q  inicio[d] = byte donde empieza la seccion d dentro del .bin
#   cantidad   10 x Q  cantidad[d] = personas en la seccion d
#
# SECCIONES: despues de la cabecera van las personas agrupadas por el primer
# digito del DNI (dni // 10_000_000): primero la seccion 0, luego la 1, ... hasta la 9.
# Dentro de cada seccion quedan en el orden en que se generaron.
# Una seccion vacia tiene cantidad 0 e inicio igual al de la siguiente.
#
# PERSONA (tamaño variable):
#   REGISTRO (26 bytes, ver configG.py) + usuario del correo (ascii, sin terminador)
#   el ultimo campo del REGISTRO es la longitud del usuario del correo

import shutil # para copiar los temporales al .bin por bloques
from catalogog import *

TAM_BUFFER = 8 * 1024 * 1024 # se escribe al disco cada 8 mb aprox

SECCIONES = 10 # una seccion por cada primer digito del DNI (0..9)
DIVISOR_SECCION = TOTAL_DNI // SECCIONES # dni // 10_000_000 = primer digito

VERSION_BIN = 2 # version del formato con secciones
# magic, version, total, inicio[0..9], cantidad[0..9]
CABECERA_V2 = struct.Struct(f"<4sIQ{SECCIONES}Q{SECCIONES}Q")
assert CABECERA_V2.size == 176

def generar_dataset_bin(cantidad, ruta_bin, ruta_catalogos, semilla):
    if not 1 <= cantidad <= TOTAL_DNI:
        raise ValueError(f"cantidad debe estar entre 1 y {TOTAL_DNI}")

    validar_catalogos()

    rng = random.Random(semilla) # misma semilla mismo datos
    multiplicador = calcular_multiplicador(semilla)

    ruta_bin, ruta_catalogos = Path(ruta_bin), Path(ruta_catalogos)
    ruta_bin.parent.mkdir(parents=True, exist_ok=True)
    ruta_catalogos.parent.mkdir(parents=True, exist_ok=True)

    print(f"Nombres disponibles: {len(NOMBRES_MASCULINOS)} masculinos, {len(NOMBRES_FEMENINOS)} femeninos")
    print(f"Apellidos disponibles: {len(APELLIDOS_PERUANOS)}")

    # un temporal por seccion, al lado del .bin pa que la union sea en el mismo disco
    rutas_tmp = [ruta_bin.with_name(f"{ruta_bin.name}.seccion{d}.tmp") for d in range(SECCIONES)]
    temporales = [] # archivos temporales abiertos
    buffers = [bytearray() for _ in range(SECCIONES)] # un buffer por seccion
    cantidades = [0] * SECCIONES # personas en cada seccion
    bytes_seccion = [0] * SECCIONES # bytes que ocupa cada seccion

    try:
        temporales = [ruta.open("wb") for ruta in rutas_tmp]

        for i in range(cantidad):
            campos, usuario_correo = generar_ciudadano(i, rng, multiplicador)
            d = campos[0] // DIVISOR_SECCION # primer digito del DNI (campos[0] es el dni)

            buffer = buffers[d]
            tam_antes = len(buffer)
            buffer += REGISTRO.pack(*campos)
            buffer += usuario_correo
            bytes_seccion[d] += len(buffer) - tam_antes # bytes de esta persona
            cantidades[d] += 1

            if len(buffer) >= TAM_BUFFER: # se vacia solo el buffer que se lleno
                temporales[d].write(buffer)
                buffer.clear()

            if (i + 1) % 1_000_000 == 0:
                print(f"Generados: {i+1:,} / {cantidad:,}")

        for d in range(SECCIONES):
            temporales[d].write(buffers[d]) # escribir lo que queda en cada buffer
            temporales[d].close()
        buffers.clear() # liberar memoria antes de unir

        # inicio de cada seccion: la 0 empieza justo despues de la cabecera
        inicios = [0] * SECCIONES
        posicion = CABECERA_V2.size
        for d in range(SECCIONES):
            inicios[d] = posicion # si la seccion esta vacia queda igual al de la siguiente
            posicion += bytes_seccion[d]

        with ruta_bin.open("wb") as archivo:
            archivo.write(CABECERA_V2.pack(MAGIC, VERSION_BIN, cantidad, *inicios, *cantidades))

            for d in range(SECCIONES): # unir en orden 0..9
                with rutas_tmp[d].open("rb") as tmp:
                    shutil.copyfileobj(tmp, archivo, TAM_BUFFER) # copia por bloques de 8 mb

    finally:
        for tmp in temporales:
            tmp.close() # cerrar por si hubo error (cerrar dos veces no pasa nada)
        for ruta in rutas_tmp:
            ruta.unlink(missing_ok=True) # borrar temporales siempre, haya error o no

    escribir_catalogos(ruta_catalogos)

    # el tamaño se mide despues de cerrar el archivo
    print("DataSet generado")
    print(f"Registros: {cantidad:,}")
    print(f"Archivo: {ruta_bin} ({ruta_bin.stat().st_size / 1024**2:,.2f} MB)")
    print(f"Catalogos: {ruta_catalogos}")
    print()
    print(f"{'Seccion':>7} | {'Inicio (byte)':>15} | {'Cantidad':>12}")
    print(f"{'-'*7}-+-{'-'*15}-+-{'-'*12}")
    for d in range(SECCIONES):
        print(f"{d:>7} | {inicios[d]:>15,} | {cantidades[d]:>12,}")

def escribir_catalogos(ruta):
    secciones = [
        ("NOMBRES", NOMBRES),
        ("APELLIDOS", APELLIDOS_PERUANOS),
        ("NACIONALIDADES", NACIONALIDADES),
        ("LUGARES_NACIMIENTO", LUGARES_NACIMIENTO),
        ("UBICACIONES", ["\t".join(u) for u in UBICACIONES]),
        ("TIPOS_VIA", TIPOS_VIA),
        ("NOMBRES_VIA", NOMBRES_VIA),
        ("ESTADOS_CIVILES", ESTADOS_CIVILES),
        ("DOMINIOS_CORREO", DOMINIOS_CORREO),
    ]

    with Path(ruta).open("w", encoding="utf-8", newline="\n") as f:
        for nombre, valores in secciones:
            f.write(f"[{nombre}] {len(valores)}\n")
            for valor in valores:
                f.write(valor + "\n")
