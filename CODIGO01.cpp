#include <iostream>
#include <fstream>
#include <string>
#include <limits>
using namespace std;

struct Proceso {
    int id;
    string nombre;
    int prioridad;
    int memoriaRequerida;
    string estado;
    Proceso* sig;
};
struct NodoCola {
    int id;
    int prioridad;
    NodoCola* sig;
};
struct Bloque {
    int idProceso;
    int cantidadMB;
    Bloque* sig;
};

Proceso* lista = NULL;
NodoCola* frente = NULL;
Bloque* cima = NULL;
const int MEMORIA_TOTAL = 1024;

int leerEntero(const string& mensaje) {
    int n;
    while (true) {
        cout << mensaje;
        if (cin >> n) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return n;
        }
        if (cin.eof()) return 0;
        cout << "Entrada invalida. Ingrese un numero entero.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

Proceso* buscarId(int id) {
    for (Proceso* p = lista; p != NULL; p = p->sig)
        if (p->id == id) return p;
    return NULL;
}

void mostrarProceso(Proceso* p) {
    if (p == NULL) { cout << "Proceso no encontrado.\n"; return; }
    cout << "ID: " << p->id << " | Nombre: " << p->nombre
         << " | Prioridad: " << p->prioridad
         << " | Memoria requerida: " << p->memoriaRequerida << " MB"
         << " | Estado: " << p->estado << "\n";
}

void registrarProceso() {
    int id = leerEntero("ID del proceso: ");
    if (id <= 0 || buscarId(id) != NULL) {
        cout << "ID invalido o repetido.\n"; return;
    }
    string nombre;
    cout << "Nombre: "; getline(cin, nombre);
    if (nombre.empty()) { cout << "El nombre no puede estar vacio.\n"; return; }
    int prioridad = leerEntero("Prioridad (mayor numero = mayor prioridad): ");
    if (prioridad < 1) { cout << "La prioridad debe ser positiva.\n"; return; }
    int memoria = leerEntero("Memoria requerida (MB): ");
    if (memoria <= 0 || memoria > MEMORIA_TOTAL) {
        cout << "La memoria requerida debe estar entre 1 y 1024 MB.\n"; return;
    }
    Proceso* nuevo = new Proceso{id, nombre, prioridad, memoria, "REGISTRADO", lista};
    lista = nuevo;
    cout << "Proceso registrado correctamente.\n";
}

bool tieneBloques(int id) {
    for (Bloque* b = cima; b != NULL; b = b->sig)
        if (b->idProceso == id) return true;
    return false;
}

bool retirarDeCola(int id) {
    NodoCola* ant = NULL;
    NodoCola* act = frente;
    while (act != NULL && act->id != id) { ant = act; act = act->sig; }
    if (act == NULL) return false;
    if (ant == NULL) frente = act->sig;
    else ant->sig = act->sig;
    delete act;
    return true;
}

void insertarCola(int id, int prioridad) {
    NodoCola* n = new NodoCola{id, prioridad, NULL};
    if (frente == NULL || prioridad > frente->prioridad) {
        n->sig = frente; frente = n; return;
    }
    NodoCola* p = frente;
    while (p->sig != NULL && p->sig->prioridad >= prioridad) p = p->sig;
    n->sig = p->sig;
    p->sig = n;
}

void eliminarProceso() {
    int id = leerEntero("ID a eliminar: ");
    Proceso* ant = NULL;
    Proceso* p = lista;
    while (p != NULL && p->id != id) { ant = p; p = p->sig; }
    if (p == NULL) { cout << "Proceso no encontrado.\n"; return; }
    if (tieneBloques(id)) {
        cout << "Libere primero los bloques de memoria asociados.\n"; return;
    }
    retirarDeCola(id);
    if (ant == NULL) lista = p->sig;
    else ant->sig = p->sig;
    delete p;
    cout << "Proceso eliminado correctamente.\n";
}

void buscarNombre() {
    string nombre;
    cout << "Nombre a buscar: "; getline(cin, nombre);
    bool encontrado = false;
    for (Proceso* p = lista; p != NULL; p = p->sig)
        if (p->nombre == nombre) { mostrarProceso(p); encontrado = true; }
    if (!encontrado) cout << "No se encontraron coincidencias.\n";
}

void modificarPrioridad() {
    int id = leerEntero("ID del proceso: ");
    Proceso* p = buscarId(id);
    if (p == NULL) { cout << "Proceso no encontrado.\n"; return; }
    int nueva = leerEntero("Nueva prioridad: ");
    if (nueva < 1) { cout << "La prioridad debe ser positiva.\n"; return; }
    p->prioridad = nueva;
    if (p->estado == "EN_COLA") {
        retirarDeCola(id);
        insertarCola(id, nueva);
    }
    cout << "Prioridad actualizada.\n";
}

void encolarProceso() {
    int id = leerEntero("ID para encolar: ");
    Proceso* p = buscarId(id);
    if (p == NULL) { cout << "Proceso no encontrado.\n"; return; }
    if (p->estado != "REGISTRADO") {
        cout << "Solo se pueden encolar procesos registrados y pendientes.\n"; return;
    }
    insertarCola(p->id, p->prioridad);
    p->estado = "EN_COLA";
    cout << "Proceso incorporado a la cola.\n";
}

// Permite modificar el estado desde el menu sin dejar una cola inconsistente.
void cambiarEstado() {
    int id = leerEntero("ID del proceso: ");
    Proceso* p = buscarId(id);
    if (p == NULL) { cout << "Proceso no encontrado.\n"; return; }
    cout << "Estado actual: " << p->estado << "\n";
    cout << "1. REGISTRADO (pendiente)\n";
    cout << "2. EN_COLA (en espera)\n";
    cout << "3. EJECUTADO (finalizado)\n";
    int nuevo = leerEntero("Nuevo estado: ");
    if (nuevo < 1 || nuevo > 3) {
        cout << "Estado invalido.\n"; return;
    }
    if (nuevo == 1) {
        retirarDeCola(id);
        p->estado = "REGISTRADO";
    } else if (nuevo == 2) {
        if (p->estado != "EN_COLA") insertarCola(p->id, p->prioridad);
        p->estado = "EN_COLA";
    } else {
        retirarDeCola(id);
        p->estado = "EJECUTADO";
    }
    cout << "Estado actualizado a " << p->estado << ".\n";
}

void ejecutarProceso() {
    if (frente == NULL) { cout << "La cola esta vacia.\n"; return; }
    NodoCola* n = frente;
    frente = frente->sig;
    Proceso* p = buscarId(n->id);
    if (p != NULL) {
        p->estado = "EJECUTADO";
        cout << "Se ejecuto el proceso " << p->nombre << " (ID " << p->id << ").\n";
    }
    delete n;
}

void listarProcesos() {
    if (lista == NULL) { cout << "No hay procesos registrados.\n"; return; }
    for (Proceso* p = lista; p != NULL; p = p->sig) mostrarProceso(p);
}
void mostrarCola() {
    if (frente == NULL) { cout << "La cola esta vacia.\n"; return; }
    for (NodoCola* n = frente; n != NULL; n = n->sig)
        cout << "ID " << n->id << " | Prioridad " << n->prioridad << "\n";
}


// Busca un proceso que este esperando en la cola, sin cambiar su orden.
void buscarEnCola() {
    int id = leerEntero("ID a buscar en la cola: ");
    for (NodoCola* n = frente; n != NULL; n = n->sig) {
        if (n->id == id) {
            cout << "En cola: ID " << n->id << " | Prioridad " << n->prioridad << "\n";
            return;
        }
    }
    cout << "El proceso no esta en la cola.\n";
}

// Busca y suma los bloques asignados a un proceso dentro de la pila.
void buscarEnPila() {
    int id = leerEntero("ID de proceso a consultar en la pila: ");
    int bloques = 0, totalMB = 0;
    for (Bloque* b = cima; b != NULL; b = b->sig) {
        if (b->idProceso == id) {
            bloques++;
            totalMB += b->cantidadMB;
        }
    }
    if (bloques == 0) cout << "No hay bloques asignados a ese ID.\n";
    else cout << "Bloques encontrados: " << bloques << " | Memoria: " << totalMB << " MB\n";
}

void asignarMemoria() {
    int id = leerEntero("ID del proceso: ");
    Proceso* p = buscarId(id);
    if (p == NULL) { cout << "Proceso no encontrado.\n"; return; }
    int mb = leerEntero("Cantidad de memoria (MB): ");
    if (mb <= 0) { cout << "La memoria debe ser positiva.\n"; return; }
    int ocupada = 0, usadaProceso = 0;
    for (Bloque* b = cima; b != NULL; b = b->sig) {
        ocupada += b->cantidadMB;
        if (b->idProceso == id) usadaProceso += b->cantidadMB;
    }
    if (mb > p->memoriaRequerida - usadaProceso) {
        cout << "Supera la memoria requerida por este proceso. Disponible para el proceso: "
             << p->memoriaRequerida - usadaProceso << " MB.\n";
        return;
    }
    if (mb > MEMORIA_TOTAL - ocupada) {
        cout << "Memoria insuficiente. Disponible: " << MEMORIA_TOTAL - ocupada << " MB.\n";
        return;
    }
    cima = new Bloque{id, mb, cima};
    cout << "Bloque asignado en la pila.\n";
}
void liberarMemoria() {
    if (cima == NULL) { cout << "La pila esta vacia.\n"; return; }
    Bloque* b = cima;
    cima = cima->sig;
    cout << "Bloque liberado: ID " << b->idProceso
         << " | " << b->cantidadMB << " MB.\n";
    delete b;
}
void mostrarMemoria() {
    if (cima == NULL) { cout << "La pila esta vacia.\n"; return; }
    cout << "De la cima hacia el fondo:\n";
    for (Bloque* b = cima; b != NULL; b = b->sig)
        cout << "Proceso " << b->idProceso << " | " << b->cantidadMB << " MB\n";
}

void limpiarDatos() {
    while (lista != NULL) {
        Proceso* p = lista; lista = lista->sig; delete p;
    }
    while (frente != NULL) {
        NodoCola* n = frente; frente = frente->sig; delete n;
    }
    while (cima != NULL) {
        Bloque* b = cima; cima = cima->sig; delete b;
    }
}

void guardarDatos() {
    ofstream out("procesos_datos.txt");
    if (!out) { cout << "No se pudo abrir el archivo.\n"; return; }
    out << "SGP2\n";
    int n = 0;
    for (Proceso* p = lista; p != NULL; p = p->sig) n++;
    out << n << '\n';
    for (Proceso* p = lista; p != NULL; p = p->sig)
        out << p->id << '\n' << p->nombre << '\n' << p->prioridad
            << '\n' << p->memoriaRequerida << '\n' << p->estado << '\n';
    n = 0;
    for (NodoCola* q = frente; q != NULL; q = q->sig) n++;
    out << n << '\n';
    for (NodoCola* q = frente; q != NULL; q = q->sig)
        out << q->id << '\n' << q->prioridad << '\n';
    n = 0;
    for (Bloque* b = cima; b != NULL; b = b->sig) n++;
    out << n << '\n';
    for (Bloque* b = cima; b != NULL; b = b->sig)
        out << b->idProceso << '\n' << b->cantidadMB << '\n';
    cout << "Datos guardados en procesos_datos.txt.\n";
}

void cargarDatos() {
    ifstream in("procesos_datos.txt");
    if (!in) { cout << "No existe el archivo para cargar.\n"; return; }
    string marca;
    getline(in, marca);
    if (marca != "SGP2") { cout << "Archivo antiguo o formato no reconocido; use datos guardados con esta version (SGP2).\n"; return; }
    // Lee en estructuras auxiliares y cambia los datos solo si la lectura termina.
    Proceso* nuevaLista = NULL;
    Proceso* finLista = NULL;
    NodoCola* nuevaCola = NULL;
    NodoCola* finCola = NULL;
    Bloque* nuevaPila = NULL;
    Bloque* fondoPila = NULL;
    int cantidad;
    bool valido = bool(in >> cantidad) && cantidad >= 0 && cantidad <= 100000;
    if (valido) in.ignore(numeric_limits<streamsize>::max(), '\n');
    for (int i = 0; valido && i < cantidad; ++i) {
        int id, prioridad, memoria;
        string nombre, estado;
        valido = bool(in >> id);
        in.ignore(numeric_limits<streamsize>::max(), '\n');
        valido = valido && bool(getline(in, nombre)) && bool(in >> prioridad >> memoria);
        in.ignore(numeric_limits<streamsize>::max(), '\n');
        valido = valido && bool(getline(in, estado));
        valido = valido && id > 0 && prioridad > 0 && memoria > 0 && memoria <= MEMORIA_TOTAL && !nombre.empty()
            && (estado == "REGISTRADO" || estado == "EN_COLA" || estado == "EJECUTADO");
        if (valido) {
            Proceso* p = new Proceso{id, nombre, prioridad, memoria, estado, NULL};
            if (!nuevaLista) nuevaLista = p;
            else finLista->sig = p;
            finLista = p;
        }
    }
    int cantidadCola = 0;
    if (valido) valido = bool(in >> cantidadCola) && cantidadCola >= 0 && cantidadCola <= 100000;
    for (int i = 0; valido && i < cantidadCola; ++i) {
        int id, pr;
        valido = bool(in >> id >> pr) && id > 0 && pr > 0;
        if (valido) {
            NodoCola* q = new NodoCola{id, pr, NULL};
            if (!nuevaCola) nuevaCola = q;
            else finCola->sig = q;
            finCola = q;
        }
    }
    int cantidadBloques = 0;
    if (valido) valido = bool(in >> cantidadBloques) && cantidadBloques >= 0 && cantidadBloques <= 100000;
    for (int i = 0; valido && i < cantidadBloques; ++i) {
        int id, mb;
        valido = bool(in >> id >> mb) && id > 0 && mb > 0;
        if (valido) {
            Bloque* b = new Bloque{id, mb, NULL};
            if (!nuevaPila) nuevaPila = b;
            else fondoPila->sig = b;
            fondoPila = b;
        }
    }
    // Validaciones de consistencia entre lista, cola y pila antes de sustituir datos.
    if (valido) {
        for (Proceso* p = nuevaLista; p != NULL; p = p->sig) {
            for (Proceso* otro = p->sig; otro != NULL; otro = otro->sig)
                if (p->id == otro->id) valido = false;
            int vecesCola = 0;
            for (NodoCola* q = nuevaCola; q != NULL; q = q->sig)
                if (q->id == p->id) vecesCola++;
            if ((p->estado == "EN_COLA" && vecesCola != 1) ||
                (p->estado != "EN_COLA" && vecesCola != 0)) valido = false;
        }
        for (NodoCola* q = nuevaCola; q != NULL; q = q->sig) {
            Proceso* relacionado = nuevaLista;
            while (relacionado && relacionado->id != q->id) relacionado = relacionado->sig;
            if (!relacionado || relacionado->prioridad != q->prioridad ||
                (q->sig && q->prioridad < q->sig->prioridad)) valido = false;
        }
        int ocupada = 0;
        for (Bloque* b = nuevaPila; b != NULL; b = b->sig) {
            ocupada += b->cantidadMB;
            bool existe = false;
            for (Proceso* p = nuevaLista; p != NULL; p = p->sig)
                if (p->id == b->idProceso) existe = true;
            if (!existe || ocupada > MEMORIA_TOTAL) valido = false;
        }
        for (Proceso* p = nuevaLista; p != NULL; p = p->sig) {
            int usada = 0;
            for (Bloque* b = nuevaPila; b != NULL; b = b->sig)
                if (b->idProceso == p->id) usada += b->cantidadMB;
            if (usada > p->memoriaRequerida) valido = false;
        }
    }
    if (!valido) {
        while (nuevaLista) { Proceso* p = nuevaLista; nuevaLista = p->sig; delete p; }
        while (nuevaCola) { NodoCola* q = nuevaCola; nuevaCola = q->sig; delete q; }
        while (nuevaPila) { Bloque* b = nuevaPila; nuevaPila = b->sig; delete b; }
        cout << "Archivo incompleto o con datos invalidos.\n";
        return;
    }
    limpiarDatos();
    lista = nuevaLista;
    frente = nuevaCola;
    cima = nuevaPila;
    cout << "Datos recuperados correctamente.\n";
}

void mostrarMenu() {
    cout << "\n===== SISTEMA DE GESTION DE PROCESOS =====\n"
         << " 1. Registrar proceso\n 2. Eliminar proceso\n"
         << " 3. Buscar por ID\n 4. Buscar por nombre\n"
         << " 5. Modificar prioridad\n 6. Encolar proceso\n"
         << " 7. Ejecutar siguiente proceso\n 8. Mostrar cola\n"
         << " 9. Asignar memoria\n10. Liberar memoria\n"
         << "11. Mostrar memoria\n12. Listar procesos\n"
         << "13. Guardar datos\n14. Cargar datos\n"
         << "15. Buscar proceso en cola\n16. Buscar bloques por proceso\n"
         << "17. Cambiar estado del proceso\n 0. Salir\n";
}

int main() {
    int opcion;
    do {
        mostrarMenu();
        opcion = leerEntero("Seleccione una opcion: ");
        switch (opcion) {
            case 1: registrarProceso(); break;
            case 2: eliminarProceso(); break;
            case 3: mostrarProceso(buscarId(leerEntero("ID a buscar: "))); break;
            case 4: buscarNombre(); break;
            case 5: modificarPrioridad(); break;
            case 6: encolarProceso(); break;
            case 7: ejecutarProceso(); break;
            case 8: mostrarCola(); break;
            case 9: asignarMemoria(); break;
            case 10: liberarMemoria(); break;
            case 11: mostrarMemoria(); break;
            case 12: listarProcesos(); break;
            case 13: guardarDatos(); break;
            case 14: cargarDatos(); break;
            case 15: buscarEnCola(); break;
            case 16: buscarEnPila(); break;
            case 17: cambiarEstado(); break;
            case 0: cout << "Saliendo del programa.\n"; break;
            default: cout << "Opcion no reconocida.\n";
        }
    } while (opcion != 0);
    limpiarDatos();
    return 0;
}
