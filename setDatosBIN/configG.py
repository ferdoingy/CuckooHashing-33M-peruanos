import math # para las operaciones
import random # para seleccionar aleatoriamente de las listas
import struct # para definir el formato del archivo binario
from pathlib import Path # para crear la carpeta donde guardare

# Pal DNI
TOTAL_DNI = 100_000_000 # 100 millones de combinaciones posibles para 8 digitos
DESPLAZAMIENTO = 18_472_613 # se suma al resultado para que los DNIs no sean 00000000

# La probabilidad para los apellidos
PROB_APELLIDO_IGUAL = 1 / 500 

# Nombres comunes en Peru
NOMBRES_MASCULINOS = [
    "José", "Luis", "Juan", "Carlos", "Jorge", "Miguel", "Víctor", "Pedro",
    "César", "Manuel", "Jesús", "Julio", "Mario", "Raúl", "Alberto", "Fernando",
    "Ricardo", "Daniel", "Javier", "Eduardo", "Óscar", "Alejandro", "Antonio",
    "Francisco", "Roberto", "Wilmer", "Edwin", "Walter", "Segundo", "Félix",
    "Hugo", "Rubén", "Marco", "Enrique", "Ángel", "Rafael", "Diego", "Andrés",
    "Sebastián", "Mateo", "Santiago", "Gabriel", "Sergio", "Martín", "Alonso",
    "Joel", "Jhon", "Jhonatan", "Elmer", "Wilson", "Nilton", "Percy", "Henry",
    "Freddy", "Hernán", "Gonzalo", "Gustavo", "Héctor", "Iván", "Ronald",
    "Renzo", "Piero", "Bruno", "Rodrigo", "Gerardo", "Abel", "Alfredo",
    "Armando", "Arturo", "Augusto", "Benjamín", "Christian", "David", "Edgar",
    "Edson", "Elías", "Emilio", "Ernesto", "Esteban", "Fabián", "Fidel",
    "Franco", "Germán", "Gilberto", "Guillermo", "Hilario", "Isaac", "Jaime",
    "Joaquín", "Josué", "Julián", "Leonardo", "Lorenzo", "Lucas", "Marcelino",
    "Mariano", "Máximo", "Moisés", "Nicolás", "Orlando", "Pablo", "Ramiro",
    "Reynaldo", "Rolando", "Samuel", "Saúl", "Simón", "Teodoro", "Tomás",
    "Ulises", "Valentín", "Vicente", "Williams", "Yuri", "Teófilo", "Eusebio",
    "Gregorio", "Celestino", "Leoncio", "Anderson", "Kevin", "Brayan", "Jean",
    "Alexander", "Adrián", "Thiago", "Aldo", "Américo", "Ciro", "Dante",
    "Erick", "Frank", "Gianfranco", "Harold", "Junior", "Luciano", "Milton",
]

NOMBRES_FEMENINOS = [
    "María", "Rosa", "Ana", "Carmen", "Juana", "Luz", "Elizabeth", "Gladys",
    "Isabel", "Teresa", "Julia", "Martha", "Silvia", "Patricia", "Lucía",
    "Milagros", "Flor", "Margarita", "Victoria", "Sonia", "Norma", "Nelly",
    "Ruth", "Gloria", "Carla", "Andrea", "Valeria", "Camila", "Daniela",
    "Fiorella", "Gabriela", "Paola", "Mariela", "Maribel", "Yolanda", "Yesenia",
    "Jessica", "Katherine", "Karina", "Karen", "Diana", "Sofía", "Valentina",
    "Ximena", "Luciana", "Alessandra", "Antonella", "Kiara", "Nicole",
    "Stephany", "Vanessa", "Verónica", "Mónica", "Claudia", "Cecilia", "Sandra",
    "Susana", "Pilar", "Esther", "Elena", "Liliana", "Lourdes", "Marleny",
    "Marisol", "Maritza", "Miriam", "Nancy", "Olga", "Raquel", "Rocío",
    "Roxana", "Sara", "Soledad", "Viviana", "Zulema", "Evelyn", "Fátima",
    "Fanny", "Haydee", "Irma", "Janet", "Jhoana", "Lidia", "Magaly", "Mercedes",
    "Delia", "Dora", "Edith", "Elsa", "Eva", "Felícita", "Graciela", "Hilda",
    "Inés", "Irene", "Josefina", "Leonor", "Lucila", "Marina", "Natalia",
    "Noemí", "Paula", "Pamela", "Rebeca", "Rosario", "Sabina", "Celia", "Clara",
    "Beatriz", "Betty", "Blanca", "Angélica", "Alicia", "Adriana", "Ángela",
    "Antonia", "Aurora", "Bertha", "Carolina", "Catalina", "Cinthia",
    "Consuelo", "Doris", "Erika", "Estela", "Francisca", "Gisela", "Guadalupe",
    "Ingrid", "Judith", "Julissa", "Kelly", "Lisbeth", "Lorena", "Melissa",
    "Nadia", "Nataly", "Rosmery", "Sheyla", "Tania", "Wendy", "Yanet", "Zoila",
]

APELLIDOS_PERUANOS = [
    "Quispe", "Flores", "Sánchez", "Rodríguez", "García", "Rojas", "Huamán",
    "Mamani", "Ramos", "Chávez", "Vásquez", "Díaz", "Torres", "Mendoza",
    "Gonzales", "López", "Pérez", "Castillo", "Ramírez", "Fernández", "Cruz",
    "Espinoza", "Gutiérrez", "Condori", "Vargas", "Ruiz", "Salazar", "Romero",
    "Silva", "Medina", "Herrera", "Castro", "Morales", "Choque", "Huanca",
    "Apaza", "Ccama", "Ticona", "Calla", "Cusi", "Huillca", "Ccahuana",
    "Yupanqui", "Paucar", "Pari", "Puma", "Coaquira", "Cutipa", "Machaca",
    "Laura", "Limachi", "Nina", "Ccori", "Inca", "Taype", "Cahuana", "Chambi",
    "Chura", "Coila", "Colque", "Cueva", "Guevara", "Aguilar", "Alvarado",
    "Arias", "Ayala", "Barrios", "Benites", "Bravo", "Bustamante", "Cabrera",
    "Cáceres", "Calderón", "Campos", "Cárdenas", "Carrasco", "Carrillo",
    "Contreras", "Córdova", "Cornejo", "Correa", "Cortez", "Cuba", "Delgado",
    "Domínguez", "Escobar", "Estrada", "Farfán", "Figueroa", "Fuentes",
    "Gallegos", "Gamarra", "Gómez", "Guerrero", "Hernández", "Huaranga",
    "Ibáñez", "Jiménez", "Lazo", "León", "Lozano", "Luna", "Maldonado",
    "Manrique", "Martínez", "Meléndez", "Mejía", "Miranda", "Molina", "Montes",
    "Montoya", "Mora", "Moreno", "Muñoz", "Navarro", "Núñez", "Ortega", "Ortiz",
    "Pacheco", "Palacios", "Paredes", "Peña", "Pinto", "Ponce", "Prado",
    "Quiroz", "Ríos", "Rivera", "Robles", "Rosales", "Salas", "Salinas",
    "Sandoval", "Santos", "Saavedra", "Serrano", "Soto", "Suárez", "Tapia",
    "Távara", "Valdivia", "Valencia", "Vega", "Velásquez", "Vera",
    "Villanueva", "Zapata", "Zevallos", "Zúñiga", "Huamaní", "Huayta",
    "Huarcaya", "Llanos", "Lliuya", "Mallma", "Poma", "Sullca", "Tito", "Vilca",
    "Yauri", "Zamata", "Atoche", "Chunga", "Chumpitaz", "Huertas", "Inga",
    "Julca", "Lázaro", "Llontop", "Sialer", "Yovera", "Ñañez", "Ancco",
    "Anccasi", "Ccoa", "Ccopa", "Huallpa", "Mayta", "Pacco", "Quicaño",
    "Tacuri", "Tupa", "Uscamayta", "Ayme", "Callo", "Chipana", "Huancahuari",
    "Oré", "Rivas", "Salcedo", "Sotelo", "Valverde", "Villegas", "Zárate",
    "Acosta", "Aguirre", "Alarcón", "Alva", "Álvarez", "Arce", "Arévalo",
    "Bautista", "Becerra", "Blanco", "Cano", "Castañeda", "Chacón", "Chirinos",
    "Dávila", "Durand", "Enríquez", "Falcón", "Gálvez", "Hidalgo", "Hurtado",
    "Lara", "Llerena", "Loayza", "Marín", "Mori", "Olivera", "Osorio",
    "Pajuelo", "Paz", "Polo", "Reyes", "Rengifo", "Sáenz", "Solís", "Tello",
    "Tineo", "Trujillo", "Ugarte", "Urbina", "Vílchez", "Yáñez", "Zegarra",
    "Panduro", "Pezo", "Shapiama", "Tuesta", "Pinedo", "Ahuanari", "Pacaya",
    "Sangama", "Tananta", "Cachique", "Amasifuén",
]

NACIONALIDADES = ["Peruana"]

# Estado civil
ESTADOS_CIVILES_MASCULINO = ["Soltero", "Casado", "Divorciado", "Viudo"]
ESTADOS_CIVILES_FEMENINO = ["Soltera", "Casada", "Divorciada", "Viuda"]

# Correo
DOMINIOS_CORREO = ["example.com"] # dominio reservado para datos ficticios

# Direcciones
UBICACIONES = [
    ("Lima", "Lima", "Lima", "Miraflores"),
    ("Lima", "Lima", "Lima", "San Isidro"),
    ("Lima", "Lima", "Lima", "Santiago de Surco"),
    ("Lima", "Lima", "Lima", "San Miguel"),
    ("Arequipa", "Arequipa", "Arequipa", "Cayma"),
    ("Arequipa", "Arequipa", "Arequipa", "Cerro Colorado"),
    ("Cusco", "Cusco", "Cusco", "Wanchaq"),
    ("Cusco", "Cusco", "Cusco", "San Sebastián"),
    ("Piura", "Piura", "Piura", "Castilla"),
    ("Piura", "Piura", "Piura", "Veintiséis de Octubre"),
    ("La Libertad", "Trujillo", "Trujillo", "Trujillo"),
    ("Lambayeque", "Chiclayo", "Chiclayo", "La Victoria"),
    ("Junín", "Huancayo", "Huancayo", "El Tambo"),
    ("Puno", "Puno", "Puno", "Puno"),
    ("Loreto", "Maynas", "Iquitos", "Iquitos")
]

# Lugar de nacimiento
LUGARES_NACIMIENTO = list(dict.fromkeys(f"{u[2]}, {u[0]}" for u in UBICACIONES))

TIPOS_VIA = [
    "Av.", "Jr.", "Calle", "Pasaje"
]

NOMBRES_VIA = [
    "Los Olivos", "Los Pinos", "Los Eucaliptos", "Los Cedros",
    "Los Álamos", "Los Sauces", "Los Robles", "Los Nogales",
    "Los Cipreses", "Los Abetos", "Los Fresnos", "Los Tilos",
    "Los Laureles", "Los Jacarandás", "Los Magnolios",
    "Los Girasoles", "Los Tulipanes", "Los Claveles",
    "Los Jazmines", "Los Lirios", "28 de Julio", "9 de Diciembre",
    "2 de Mayo", "7 de Junio",
    "8 de Octubre", "29 de Junio",
    "1 de Mayo", "30 de Agosto",
    "Combate de Angamos", "Batalla de Ayacucho",
    "Independencia", "La República",
    "Los Libertadores", "Los Próceres",
    "Los Héroes", "La Unión", "José de San Martín", "Simón Bolívar",
    "Miguel Grau", "Francisco Bolognesi",
    "Andrés Avelino Cáceres", "José Olaya",
    "Túpac Amaru II", "Micaela Bastidas",
    "María Parado de Bellido", "Francisco Pizarro",
    "Ricardo Palma", "César Vallejo",
    "José Carlos Mariátegui", "Abraham Valdelomar",
    "José María Arguedas", "Jorge Basadre",
    "Manuel González Prada", "Daniel Alcides Carrión",
    "Hipólito Unanue", "Antonio Raimondi"
]

# Formato del archivo binario
MAGIC = b"DNIP"
VERSION = 1

# < = little-endian y sin relleno | 4s = 4 bytes, I = uint32, Q = uint64, H = uint16, B = uint8
CABECERA = struct.Struct("<4sIQ") # magic, version, n_registros
REGISTRO = struct.Struct("<IHHHHBBBBBHIBBB") # parte fija de cada persona (26 bytes)

assert CABECERA.size == 16
assert REGISTRO.size == 26