from configG import *

# Funcion para poder generar los catalogos 
def crear_catalogos(semilla): #Se toma como parametro una seed que sirve para repetir}

    fake = Faker("es_ES") #se genera una instacia de faker con el idioma y region de españa
    fake.seed_instance(semilla) #Se posiciona en la semilla que hemos elegido para asi generar los datos pseudoaletoriamente

    nombres = set(NOMBRES_PERUANOS) #se le asignan vairables al set de nombres
    apellidos = set(APELLIDOS_PERUANOS) #se le asigna una variable al set de apellidos

    for _ in range(5000): #sirve para ampliar más el catalogo de nombres

        #como se definio la seed con la que se va trabajar los nombres generados seran exclusivamente de la seed "semilla"
        nombres.add(fake.first_name()) #Se genera un nuevos nombres para ampliar el set predeterminado 
        apellidos.add(fake.last_name()) #Se generan nuevos apellidos para ampliar el set predeterminado

    return sorted(nombres), sorted(apellidos) #retorna los sets reordenados con los nombres nuevos generados

#funcion para generar dni con un multiplicador
def generar_dni(indice):
    numero = (MULTIPLICADOR*indice) % TOTAL_DNI #se genera un numero dni
    return f"{numero:08d}" 


#funcion para generar el nombre completo
def generar_nombre(rng, nombres, apellidos): #se toma un valor generador, un set de nombres y un set de apellidos

    nombre1, nombre2 = rng.sample(nombres,2) #se toma al azar 2 nombres y se le asigna a nombre1 y nombre2 
    apellido1, apellido2 = rng.sample(apellidos,2) #lo mismo que la anterior pero con los apellidos

    return f"{nombre1} {nombre2}",f"{apellido1} {apellido2}" #se devuelve en forma de tupla el nombre completo

#funcion para generar la direccion
def generar_ubicacion(rng): #se toma solo el generador
    departamento, provincia, ciudad, distrito = (
        rng.choice(UBICACIONES)
    )
    via = rng.choide(TIPOS_VIA)
    numero = rng.randint(1, 9999)

    #Direccion ficticia, no tomada de personas reales
    ubicacion = f"{via} Simulada {numero}"

    return( #retorna los datos
        departamento,
        provincia,
        ciudad,
        distrito,
        ubicacion
    )
def generar_ciudadano(indice, rng, nombres, apellidos):
    #se llama a la funcion para generar el dni del ciudadano
    dni = generar_dni(indice)
    #se llama a la funcion para generar su nombre
    nombres, apellidos = generar_nombre(
        rng,nombres,apellidos
    )
    nacionalidad = rng.choices( #se realiza una eleccion aleatoria dentro del set de nacionalidades 
        NACIONALIDADES,
        weights=[94, 2, 1, 1, 1, 0.5, 0.5], #se le define un peso para cada una de las nacionalidades asi sale
        k=1 #se selecciona solo 1 de las nacionalidades
    )[0]

      # Lugar de nacimiento simulado.
    nacimiento = rng.choice(UBICACIONES) #se selecciona de las ubicaciones para el lugar de naciomiento
    lugar_nacimiento = (
        f"{nacimiento[2]}, {nacimiento[0]}" #devuelve el distrito y el departamento en una tupla
    )

    (
        departamento,
        provincia,
        ciudad,
        distrito,
        ubicacion
    ) = generar_ubicacion(rng) #se genera la ubicacion y se le asignan los valoeres que se retornan

    # Genrerador de numero telefonicos
    telefono = f"000{indice % 1_000_000:06d}"
    # Generador de correo electronico
    correo = f"ciudadano{dni}@example.com"
    # Del set de ESTADOS_CIVILES se selecciona 1 al azar
    estado_civil = rng.choice(ESTADOS_CIVILES)

    return [
        dni,
        nombres,
        apellidos,
        nacionalidad,
        lugar_nacimiento,
        departamento,
        provincia,
        ciudad,
        distrito,
        ubicacion,
        telefono,
        correo,
        estado_civil
    ]