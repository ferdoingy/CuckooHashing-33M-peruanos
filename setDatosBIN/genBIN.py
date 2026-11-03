from catalogog import *

TAM_BUFFER = 8 * 1024 * 1024 # se escribe al disco cada 8 mb aprox

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

    buffer = bytearray()

    with ruta_bin.open("wb") as archivo:
        archivo.write(CABECERA.pack(MAGIC, VERSION, cantidad))

        for i in range(cantidad):
            campos, usuario_correo = generar_ciudadano(i, rng, multiplicador)
            buffer += REGISTRO.pack(*campos)
            buffer += usuario_correo

            if len(buffer) >= TAM_BUFFER:
                archivo.write(buffer)
                buffer.clear()

            if (i + 1) % 1_000_000 == 0:
                print(f"Generados: {i+1:,} / {cantidad:,}")

        escribir_catalogos(ruta_catalogos)


        print("DataSet generado")
        print(f"Registros: {cantidad:,}")
        print(f"Archivo: {ruta_bin} ({ruta_bin.stat().st_size / 1024**2:,.2f} MB)")
        print(f"Catalogos: {ruta_catalogos}")

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