from configG import *
import unicodedata # para quitar tildes y ñ del correo

# Catalogos: cada texto se guarda en el .bin como su indice

NOMBRES = NOMBRES_MASCULINOS + NOMBRES_FEMENINOS
INICIO_FEMENINOS = len(NOMBRES_MASCULINOS) # desde este indice empieza los nombres de mujer

# Estado vicil: indices 0 - 3 pa hombre, 4 - 7 mujer
ESTADOS_CIVILES = ESTADOS_CIVILES_MASCULINO + ESTADOS_CIVILES_FEMENINO

def validar_catalogos():
    # se revisa que no haya repetidos 
    for nombre, lista, maximo in[
        ("NOMBRES", NOMBRES, 2**16),
        ("APELLIDOS", APELLIDOS_PERUANOS, 2**16),
        ("NACIONALIDADES", NACIONALIDADES, 2**8),
        ("LUGARES_NACIMIENTO", LUGARES_NACIMIENTO, 2**8),
        ("UBICACIONES", UBICACIONES, 2**8),
        ("TIPOS_VIA", TIPOS_VIA, 2**8),
        ("NOMBRES_VIA", NOMBRES_VIA, 2**8),
        ("ESTADOS_CIVILES", ESTADOS_CIVILES, 2**8),
        ("DOMINIOS_CORREO", DOMINIOS_CORREO, 2**8),
    ]:
        repetidos = {v for v in lista if lista.count(v) > 1}
        if repetidos:
            raise ValueError(f"Valores repetidos en {nombre}: {sorted(repetidos)}")
        if len(lista) > maximo:
            raise ValueError(f"{nombre} tiiene {len(lista)} valores; el maximo es {maximo}")

# para quitar tildes y ñ

def a_ascii(texto):
    return unicodedata.normalize("NFKD", texto).encode("ascii", "ignore").decode("ascii")
    #NFKD: separa la letra de su tilde o el sombrero de la ñ

# version sin tildes

NOMBRES_ASCII = [a_ascii(n) for n in NOMBRES]


# DNI
def calcular_multiplicador(semilla):
    rng = random.Random(f"dni-{semilla}") # generador propio para no alterar los demas datos
    while True:
        multiplicador = rng.randint(1, TOTAL_DNI - 1)
        if math.gcd(multiplicador, TOTAL_DNI) == 1: # si el mcd es 1 tonces es coprimo
            return multiplicador

# funcion para generar DNI
def generar_dni(indice, multiplicador):
    # la formula es: dni = (a * i + b) mod 10^8 asi da un DNI distinto
    return (multiplicador * indice + DESPLAZAMIENTO) % TOTAL_DNI

# para generar la persona

# para los nombres segun el sexo
def generar_nombres(rng):
    es_mujer = rng.random() < 0.5 # 50% de probabilidad de ser mujer
    if es_mujer:
        inicio, cantidad = INICIO_FEMENINOS, len(NOMBRES_FEMENINOS)
    else:
        inicio, cantidad = 0, len(NOMBRES_MASCULINOS)

    i1, i2 = rng.sample(range(cantidad), 2) # dos nombres 
    return es_mujer, inicio + i1, inicio + i2

# para los apellidos
def generar_apellidos(rng):
    cantidad = len(APELLIDOS_PERUANOS)
    if rng.random() < PROB_APELLIDO_IGUAL:
        a = rng.randrange(cantidad)
        return a, a
    a1, a2 = rng.sample(range(cantidad), 2)
    return a1, a2

# para la direccion
def generar_direccion(rng):
    ubicacion = rng.randrange(len(UBICACIONES))
    tipo_via = rng.randrange(len(TIPOS_VIA))
    nombre_via = rng.randrange(len(NOMBRES_VIA))
    numero = rng.randint(1, 9999)
    # direccion ficticia
    return ubicacion, tipo_via, nombre_via, numero

def generar_ciudadano(indice, rng, multiplicador):
    """
    Devuelve todo
    """
    dni = generar_dni(indice, multiplicador)
    es_mujer, nombre1, nombre2 = generar_nombres(rng)
    apellido1, apellido2 = generar_apellidos(rng)

    nacionalidad = 0 # "peruana"
    lugar_nacimiento = rng.randrange(len(LUGARES_NACIMIENTO))
    ubicacion, tipo_via, nombre_via, numero_via = generar_direccion(rng)

    # telefono
    telefono = 900_000_000 + rng.randint(0, 99_999_999)

    #Estado civil
    estado_civil = rng.randrange(4) + (4 if es_mujer else 0)

    # correo
    usuario_correo = f"{NOMBRES_ASCII[nombre1]}{NOMBRES_ASCII[nombre2]}{dni:08d}".encode("ascii")
    dominio = 0

    campos = (
        dni,
        nombre1, nombre2,
        apellido1, apellido2,
        nacionalidad,
        lugar_nacimiento,
        ubicacion,
        tipo_via,
        nombre_via,
        numero_via,
        telefono,
        estado_civil,
        dominio,
        len(usuario_correo),
    )
    return campos, usuario_correo