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
def generar_dni(indice):
    numero = (MULTIPLICADOR*indice) % TOTAL_DNI #se genera un numero dni
    return f"{numero:08d}" 
